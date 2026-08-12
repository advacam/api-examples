#include <windows.h>

#include <iostream>
#include <fstream>
#include <filesystem>  
using namespace std;

// (c) 2026 Pavel Hudecek, Advacam, https://advacam.cz, https://wiki.advacam.cz/wiki/Binary_core_API
//
// This example:
// 1. Load calibration to clustering engine
// 2. Set - up the callbacks
// 3. Start the measurement thru clustering engine
// 4. Process measured data and online show preview of biggest and energiest clusters occured



// See example-basic.cpp about compilation and API package location considerations.
#define PATH_TO_API ../x64/Debug
//#define CHDIRS_toAPI_off
//#define CHDIRS_back_off

#ifdef PATH_TO_API
#define STR2(x) #x
#define STR(x) STR2(x)
#define API_HEADER(file) STR(PATH_TO_API/file)
#include API_HEADER(common.h)
#include API_HEADER(pxcapi.h)
#include API_HEADER(clusteringapi.h)
#else
#include "common.h"
#include "pxcapi.h"
#include "clusteringapi.h"
#endif // PATH_TO_API

#define TEST_dir "test-files/"
#define TEST_fileT3pa "test.t3pa"
#define TEST_fileClog "test.clog"
#define TEST_fileClusters "test-outcl.txt"

double measTime = 400.0; // total measurement time [s]
int measToAcqDivider = 1;   // number of acquisitions per measurement (>1 can be for frame-based devices)
unsigned ignoreYunder = 10; // ignore pixels with y < ignoreYunder

ofstream clustersMaxsFout;
clhandle_t clHandle = nullptr;
void* iPixet = nullptr;

bool errHandler(int rc, const char* funcName, bool silent = false) { // ===========================
    if (silent && rc == 0) return true; // if silent and OK, do not print anything
    cout << funcName << ": " << rc << " ";
    if (rc == 0) {
        cout << "(0 is OK)\n";
    } else if (rc > 0) {
        cout << "(output val)\n";
    } else {
        char buff[500];
        pxpClGetLastError(clHandle, buff, 500);
        cerr << "\n  err msg: '" << buff << "'\n";
    }
    return rc >= 0;
}

unsigned devIdx = 0;
uint32_t devWidth, devHeight;
uint32_t devPixelsCount = 0;

// message callback in the Windows CLR app
void ClMessageCallbackFn(bool error, const char* message, void* userData) { // ====================
    cout << "clb Msg: err:" << (int)error << ", msg:'" << message << "'\n";
}

void ClProgressCallbackFn(bool finished, double progress, void* userData) { // ====================
    cout << "clb progress:" << progress << " %, " << "fin:" << finished << " (A to abort)\n";
    // note: Placing abort here looks more elegant, but aborting called from callbacks takes multiple longer.
}

float maxClEglob = 0.0, maxClHglob = 0.0;
unsigned short maxClsizGlob = 0;
void ClNewClustersCallbackFn(PXPCluster* clusters, size_t clusterCount, size_t acqIndex, void* userData) { // ====
    cout << "clb New Cl: clusterCount " << clusterCount << ", acqIndex " << acqIndex << "\n";

    float maxClE = 0.0, maxClH = 0.0;
    unsigned short maxClsiz = 0;
    for (unsigned ci = 0; ci < (unsigned)clusterCount; ci++) {
        if (clusters[ci].energy > maxClE) maxClE = clusters[ci].energy;
        if (clusters[ci].height > maxClH) maxClH = clusters[ci].height;
        if (clusters[ci].size > maxClsiz) maxClsiz = clusters[ci].size;
    }
    if (maxClE > maxClEglob) maxClEglob = maxClE;
    if (maxClH > maxClHglob) maxClHglob = maxClH;
    if (maxClsiz > maxClsizGlob) maxClsizGlob = maxClsiz;
    cout << "^ max single cluster size " << maxClsiz << " px, max energy " << maxClE / 1000.0 << " MeV, max height " << maxClH / 1000.0 << " MeV\n";
}

const size_t cMaxStoredPixels = 2500;
PXPClusterWithPixels    biggestCluster = { .energy = -1234, .size = 0, .height = -1234, .pixels = new PXPPixel[cMaxStoredPixels] };
PXPClusterWithPixels    energiestCluster = { .energy = -1234, .size = 0, .height = -1234, .pixels = new PXPPixel[cMaxStoredPixels] };
PXPClusterWithPixels    highestCluster = { .energy = -1234, .size = 0, .height = -1234, .pixels = new PXPPixel[cMaxStoredPixels] };

// simple logaritmic test view ====================================================================
string clusterPreview(PXPClusterWithPixels* cluster, string pref, string comment, unsigned frWid, unsigned frHei) {
    cout << "clusterPreview str - start\n";
    ostringstream out;
    unsigned xdiv = frWid / 64;
    unsigned ydiv = frHei / 32;

    double toaMin = 1e100, toaMax = -1e100;
    float* frame = new float[frWid * frHei];
    for (unsigned n = 0; n < frWid * frHei; n++) frame[n] = 0.0f;
    for (unsigned n = 0; n < cluster->size; n++) {
        frame[cluster->pixels[n].x + cluster->pixels[n].y * frWid] = cluster->pixels[n].energy;
        if (cluster->pixels[n].toa < toaMin) toaMin = cluster->pixels[n].toa;
        if (cluster->pixels[n].toa > toaMax) toaMax = cluster->pixels[n].toa;
    }

    out << pref << comment << ":\n" << pref;
    out.precision(3);
    out << "ToA:" << cluster->toa << " (dt:" << (toaMax - toaMin) << ") ns, size:" << cluster->size << " px, height:" << cluster->height;
    out << " keV, energy:" << cluster->energy / 1000.0 << " MeV\n";
    out << pref << "------------------------------------------------------------------\n";

    string desc[10] = {
        "E [keV]", "in block", "-------",
        ". >0", "+ >4", "* >16", "o >64", "O >256", "8 >1024", "# >4096"
    };
    for (unsigned y = 0; y < 32; y++) {
        out << pref << "|";
        for (unsigned x = 0; x < 64; x++) {
            unsigned val = 0;
            for (unsigned yy = 0; yy < ydiv; yy++) {
                for (unsigned xx = 0; xx < xdiv; xx++) {
                    unsigned pxIdx = (x * xdiv + xx) + (y * ydiv + yy) * frWid;
                    if (pxIdx < frWid * frHei) {
                        val += static_cast<unsigned>(frame[pxIdx]);
                    }
                }
            }
            if (val > 4096)     out << "#";
            else if (val > 1024) out << "8";
            else if (val > 256) out << "O";
            else if (val > 64)  out << "o";
            else if (val > 16)  out << "*";
            else if (val > 4)   out << "+";
            else if (val > 0)   out << ".";
            else                out << " ";
        }
        out << "| " << (y < 10 ? desc[y] : "") << "\n";
    }
    out << pref << "------------------------------------------------------------------\n";
	cout << "clusterPreview str - end\n";
    return out.str();
}
void clusterPreview(PXPClusterWithPixels* cluster, string pref, string comment) {
	cout << "clusterPreview - start\n";
    cout << clusterPreview(cluster, pref, comment, devWidth, devHeight);
    cout << pref << "pixels (position, time relative [ns], energy [keV]):\n" << pref;
    for (unsigned n = 0; n < cluster->size; n++) {
        cout << "xy:" << cluster->pixels[n].x << "," << cluster->pixels[n].y << " tr:" << cluster->pixels[n].toa - cluster->toa << " e:" << cluster->pixels[n].energy << ", ";
        if (n > 12) { cout << "..."; break; }
        if (n % 3 == 2 && n < cluster->size - 1) cout << endl << pref;
    }
    cout << "\n" << pref << "==================================================================\n";
	cout << "clusterPreview - end\n";
}
void clusterPreviewSave(PXPClusterWithPixels* cluster, string comment, ofstream* fout) { // 
	cout << "clusterPreviewSave - start\n";
    (*fout) << clusterPreview(cluster, (string)"", comment, devWidth, devHeight);
    (*fout) << "pixels (position, time relative [ns], energy [keV]):\n";
    for (unsigned n = 0; n < cluster->size; n++) {
        (*fout) << "xy:" << cluster->pixels[n].x << "," << cluster->pixels[n].y << " tr:" << cluster->pixels[n].toa - cluster->toa << " e:" << cluster->pixels[n].energy << ", ";
        if (n % 4 == 3 && n < cluster->size - 1) (*fout) << endl;
    }
    (*fout) << "\n" << "==================================================================\n";
	cout << "clusterPreviewSave - end\n";
}

unsigned testIgnoreClusterMaskCounter = 0;
bool testIgnoreCluster(PXPClusterWithPixels* cluster) { // ========================================
    if (cluster->y < ignoreYunder) { testIgnoreClusterMaskCounter++; return true; }
    if (cluster->y > ignoreYunder * 3) return false;
    for (unsigned n = 0; n < cluster->size; n++) {
        if (cluster->pixels[n].y < ignoreYunder) { testIgnoreClusterMaskCounter++; return true; }
    }
    return false;
}

double lastClusterToa = 0.0;
size_t lastClusterAcqIndex = 0;
void ClNewClustersWithPixelsCallbackFn(PXPClusterWithPixels* clusters, size_t clusterCount, size_t acqIndex, void* userData) { // ====
    cout << "clb New CWP: clusterCount " << clusterCount << ", acqIndex " << acqIndex << "\n";

    if (acqIndex > lastClusterAcqIndex) lastClusterAcqIndex = acqIndex;

    float maxClE = -123.0, maxClH = -123.0;
    unsigned short maxClsiz = 0;
    unsigned ignored = 0;
    for (unsigned ci = 0; ci < (unsigned)clusterCount; ci++) {
        if (testIgnoreCluster(&clusters[ci])) { ignored++; continue; }

        if (clusters[ci].toa > lastClusterToa) lastClusterToa = clusters[ci].toa;
        if (clusters[ci].energy > maxClE) maxClE = clusters[ci].energy;
        if (clusters[ci].height > maxClH) maxClH = clusters[ci].height;
        if (clusters[ci].size > maxClsiz) maxClsiz = clusters[ci].size;
        size_t pxCount = clusters[ci].size;
        if (pxCount > cMaxStoredPixels) {
            pxCount = cMaxStoredPixels;
            cout << "\tci:" << ci << " Cluster size " << clusters[ci].size << " px trimmed to cMaxStoredPixels (" << cMaxStoredPixels << ")\n";
        }
        string comment = "";
        auto addComment = [&comment](string c) { if (comment.length() > 0) comment += ", "; comment += c; };
        if (clusters[ci].energy > energiestCluster.energy) {
            memcpy(&energiestCluster, &clusters[ci], sizeof(PXPClusterWithPixels) - sizeof(PXPPixel*));
            memcpy(energiestCluster.pixels, clusters[ci].pixels, pxCount * sizeof(PXPPixel));
            addComment("New energiest");
        }
        if (clusters[ci].height > highestCluster.height) {
            memcpy(&highestCluster, &clusters[ci], sizeof(PXPClusterWithPixels) - sizeof(PXPPixel*));
            memcpy(highestCluster.pixels, clusters[ci].pixels, pxCount * sizeof(PXPPixel));
            addComment("New highest");
        }
        if (clusters[ci].size > biggestCluster.size) {
            memcpy(&biggestCluster, &clusters[ci], sizeof(PXPClusterWithPixels) - sizeof(PXPPixel*));
            memcpy(biggestCluster.pixels, clusters[ci].pixels, pxCount * sizeof(PXPPixel));
            addComment("New biggest");
        }
        if (clusters[ci].size > 1000) addComment("Size>1000");
        if (clusters[ci].energy > 10000.0) addComment("Energy>10M");

        if (comment.length() > 0) {
            clusterPreview(&clusters[ci], "\t", comment);
            clusterPreviewSave(&clusters[ci], comment, &clustersMaxsFout);
        }
    }
    if (ignored > 0) {
        cout << "\t(Ignored " << ignored << " clusters by testIgnoreCluster)\n";
    }
    cout << "\tLocal any single cluster maxs: size:" << maxClsiz << " px, height:" << maxClH << " keV, energy:" << maxClE / 1000.0 << " MeV\n";
    cout << "ClNewClustersWithPixelsCallbackFn - end\n";
}

int main() { // ===================================================================================
    int rc; // return code

    cout << "PXCAPI example - for details see https://wiki.advacam.cz/wiki/Binary_Clustering_API\n\n";

#ifdef PATH_TO_API
#ifndef CHDIRS_toAPI_off
    auto cwdOrig = filesystem::current_path();
    cout << "Original working directory:" << cwdOrig << "\n";
    auto changeDirToAPI = [cwdOrig]() {
        try {
            filesystem::current_path(STR(PATH_TO_API));
        }
        catch (const filesystem::filesystem_error& e) {
            cerr << "Error changing working directory to " << STR(PATH_TO_API) << ":\n" << e.what() << '\n';
            return -1;
        }
        cout << "Changed WD to PATH_TO_API: " << filesystem::current_path() << "\n";
        return 0;
        };
    if (auto chrc = changeDirToAPI() != 0) return chrc;
#endif // !CHDIRS_toAPI_off
#endif // PATH_TO_API

    cout << "pxcInitialize...\n";
    rc = pxcInitialize();
    if (!errHandler(rc, "pxcInitialize")) return rc;

    int dcnt = pxcGetDevicesCount();
    errHandler(dcnt, "pxcGetDevicesCount");
    if (dcnt == 0) {
        cout << "No devices found, exiting...\n";
        rc = pxcExit();
        cout << "pxcExit: " << rc << " (0 is OK)\n";
        return 0;
    }
    for (int n = 0; n < dcnt; n++) {
        char name[100];
        rc = pxcGetDeviceName(n, name, 100);
        errHandler(rc, "  pxcGetDeviceName", true);
        cout << "  Dev " << n << ": name:'" << name << "'\n";
    }
    devIdx = 0;
    cout << "Using dev:" << devIdx << "\n";

    rc = pxcGetDeviceDimensions(devIdx, &devWidth, &devHeight);
    errHandler(rc, "pxcGetDeviceDimensions");
    if (rc == 0) {
        devPixelsCount = devWidth * devHeight;
    } else {
        cout << "Cannot get device dimensions, trying using default 256x256.\n";
        devWidth = 256; devHeight = 256;
        devPixelsCount = devWidth * devHeight;
    }
    cout << "\t" << devWidth << "x" << devHeight << " pxCnt:" << devPixelsCount << "\n";


    //rc = pxpClLoadPixetCore("pxcore.dll");
    //errHandler(rc, "pxpClLoadPixetCore");

    rc = pxcLoadFactoryConfig(devIdx);
    errHandler(rc, "pxcLoadFactoryConfig");
    if (rc == -1027) {
        cout << "Create the 'factory' subdir and copy the factory config files there.\n";
        cout << "Or set the FactoryDir= in the [settings] section of the pixet.ini file\n";
    }

    cout << "Warning: Measuring immediately after init may cause the first data contains power-on artefacts.\n";

    cout << "Dummy acq...\n";
    rc = pxcMeasureMultipleFrames(devIdx, 5, 1.0);
    errHandler(rc, "pxcMeasureMultipleFrames");

    // Warning: Use the pxcGetIPixet from pxcapi.h, not pxpClGetIPixet from clusteringapi.h
    iPixet = pxcGetIPixet();
    if (iPixet == nullptr) cerr << "pxcGetIPixet returned nullptr\n";
    else cout << "pxcGetIPixet OK\n";

    pxpClSetIPixet(iPixet);

    clHandle = pxpClCreate(devIdx);
    if (clHandle == CL_INVALID_HANDLE) cout << "pxpClCreate INVALID\n";
    else cout << "pxpClCreate OK\n";

    //clHandle = pxpClCreate(0);
    //if (clHandle == CL_INVALID_HANDLE) cout << "pxpClCreate INVALID\n";
    //else cout << "pxpClCreate OK\n";

#ifdef PATH_TO_API
#ifndef CHDIRS_toAPI_off
#ifndef CHDIRS_back_off
    filesystem::current_path(cwdOrig);
    cout << "Returned WD to original:   " << filesystem::current_path() << "\n";
#endif // !CHDIRS_back_off
#endif // !CHDIRS_toAPI_off
#endif // PATH_TO_API

    rc = pxpClLoadCalibrationFromDevice(clHandle);
    errHandler(rc, "pxpClLoadCalibrationFromDevice");

    // offline alternative
    // pxpClLoadCalibrationFromFiles(clhandle_t handle, const char* filePaths);
    //rc = pxpClLoadCalibrationFromFiles(clHandle, "configs/MiniPIX-D06-W0065.xml");
    //errHandler(rc, "pxpClLoadCalibrationFromFiles");

    //pxpClSetMessageCallback(clhandle_t handle, ClMessageCallback callback, void* userData)
    rc = pxpClSetMessageCallback(clHandle, ClMessageCallbackFn, NULL);
    errHandler(rc, "pxpClSetMessageCallback");

    //pxpClSetProgressCallback(clhandle_t handle, ClProgressCallback callback, void* userData);
    rc = pxpClSetProgressCallback(clHandle, ClProgressCallbackFn, NULL);
    errHandler(rc, "pxpClSetProgressCallback");

    // arriving data containing properties of the clusters with all their pixels
    rc = pxpClSetNewClustersWithPixelsCallback(clHandle, ClNewClustersWithPixelsCallbackFn, NULL);
    errHandler(rc, "pxpClSetNewClustersWithPixelsCallback");

    // arriving data containing properties of the clusters with parameters only, no pixel data
    //rc = pxpClSetNewClustersCallback(clHandle, ClNewClustersCallbackFn, NULL);
    //errHandler(rc, "pxpClSetNewClustersCallback");

    // note: If registered both clusters callbacks, only second one will be called.
    
    string fName = (string)TEST_dir TEST_fileClusters;
    clustersMaxsFout.open(fName, ios::out);
    cout << "Opened file '" << fName << "' for cluster preview output.\n";
	cout << "Stream state: good:" << clustersMaxsFout.good() << ", fail:" << clustersMaxsFout.fail() << ", bad:" << clustersMaxsFout.bad() << "\n";
    clustersMaxsFout.precision(3);
    
    cout.precision(3);

    /*cout << "Meas data for masking noisy pixels...\n";
    rc = pxcMeasureTpx3DataDrivenMode(devIdx, 13.0, "");
    errHandler(rc, "pxcMeasureTpx3DataDrivenMode");
    //pxpClMaskNoisyPixels(clhandle_t handle);
    rc = pxpClMaskNoisyPixels(clHandle);
    errHandler(rc, "pxpClMaskNoisyPixels");*/

    //pxpClEnableFilteringOfNoisyPixels(clhandle_t handle, bool enable);
    rc = pxpClEnableFilteringOfNoisyPixels(clHandle, true);
    errHandler(rc, "pxpClEnableFilteringOfNoisyPixels");

    cout << "Start measuring with online clustering for " << measTime << " seconds...\n";
    //pxpClStartMeasurement(clhandle_t handle, double acqTime, double measTime, const char* outputFilePath);
    //rc = pxpClStartMeasurement(clHandle, 5.0, measTime, (TEST_dir TEST_fileClog));
    rc = pxpClStartMeasurement(clHandle, measTime / measToAcqDivider, measTime, (TEST_dir TEST_fileClog));
    errHandler(rc, "pxpClStartMeasurement");


    cout << "===============================================================================\n";
    while (pxpClIsRunning(clHandle) > 0) {
        Sleep(100);
        if (GetAsyncKeyState('A') & 0x8000) {
            cout << "while (pxpClIsRunning: aborting...\n";
            rc = pxpClAbort(clHandle);
            errHandler(rc, "pxpClAbort");
            while (pxpClIsRunning(clHandle) > 0) Sleep(100);
        } // note: Placing it in a progress callback looks more elegant, but aborting called from callbacks takes multiple longer.
        
    }
    cout << "===============================================================================\n";
    clustersMaxsFout.close();

    if (biggestCluster.energy > 0.0) {
        clusterPreview(&biggestCluster, "", (string)"Biggest cluster");
    }
    else cout << "(No biggest cluster detected)\n";
    if (energiestCluster.energy > 0.0) {
        clusterPreview(&energiestCluster, "", (string)"Energiest cluster");
    }
    else cout << "(No energiest cluster detected)\n";
    if (highestCluster.energy > 0.0) {
        clusterPreview(&highestCluster, "", (string)"Highest cluster");
    }
    else cout << "(No highest cluster detected)\n";
    cout << "testIgnoreClusterMaskCounter:" << testIgnoreClusterMaskCounter << endl;
    cout << "lastClusterToa:" << lastClusterToa << " ns\nlastClusterAcqIndex:" << lastClusterAcqIndex << endl;

#ifdef PATH_TO_API
#ifndef CHDIRS_toAPI_off
#ifndef CHDIRS_back_off
    if (auto chrc = changeDirToAPI() != 0) return chrc;
#endif // !CHDIRS_back_off
#endif // !CHDIRS_toAPI_off
#endif // PATH_TO_API

    //cout << "pxpClUnloadPixetCore...\n";
    //pxpClUnloadPixetCore();
    //cout << "pxpClUnloadPixetCore: done\n";

    cout << "pxcExit...\n";
    rc = pxcExit();
    cout << "pxcExit: " << rc << " (0 is OK)\n";
}