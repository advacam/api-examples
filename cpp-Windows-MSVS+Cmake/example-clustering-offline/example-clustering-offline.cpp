#include <windows.h>

#include <iostream>
#include <fstream>
#include <filesystem>  
#include <format>
using namespace std;

// (c) 2026 Pavel Hudecek, Advacam, https://advacam.cz, https://wiki.advacam.cz/wiki/Binary_core_API
//                                  https://wiki.advacam.cz/wiki/Binary_Clustering_API
//
// This example:
// 1. Load Pixet core without try starting devices
// 2. Init clustering engine
// 3. Set-up the callbacks
// 4. Process measured data from file to clusters
// 5. Filter clusters in callback and show preview of biggest and energiest clusters occured

// See example-basic.cpp about compilation and API package location considerations.
#define PATH_TO_API ../x64/Debug
//#define CHDIRS_toAPI_off
#define CHDIRS_back_off

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

#define TEST_dir "test-files/"                          // test files directory
#define TEST_fileT3pa "test.t3pa"                       // clustering input file T3PA, T3P, CLOG, PMF, H5, TXT
#define TEST_fileClog "test.clog"                       // clustering output file (optional) CLOG only
#define TEST_fileClusters "test-outcl.txt"              // test preview output file (optional) with clusters full data
#define TEST_pathConfig "configs\\MiniPIX-D05-W0037.xml"// input data source device config/calibration file (optional) XML or A,B,C,T files

unsigned ignoreYunder = 10;                             // ignore pixels with y < ignoreYunder
uint32_t devWidth=256, devHeight=256;                   // input data device width and height


ofstream clustersMaxsFout;
clhandle_t clHandle = nullptr;

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

// message callback in the Windows CLR app
void ClMessageCallbackFn(bool error, const char* message, void* userData) { // ====================
    cout << "clb Msg: err:" << (int)error << ", msg:'" << message << "'\n";
}

void ClProgressCallbackFn(bool finished, double progress, void* userData) { // ====================
    cout << "clb progress:" << progress << " %, " << "fin:" << finished << " (A to abort)\n";
    // note: Placing abort here looks more elegant, but aborting called from callbacks takes multiple longer.
}

float maxClEglob = 0.0, maxClHglob = 0.0;
unsigned short maxClsizGlob = 0; // ===============================================================
void ClNewClustersCallbackFn(PXPCluster* clusters, size_t clusterCount, size_t acqIndex, void* userData) { 
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

const size_t cMaxStoredPixels = 15000;
PXPClusterWithPixels    biggestCluster = { .energy = -1234, .size = 0, .height = -1234, .pixels = new PXPPixel[cMaxStoredPixels] };
PXPClusterWithPixels    energiestCluster = { .energy = -1234, .size = 0, .height = -1234, .pixels = new PXPPixel[cMaxStoredPixels] };
PXPClusterWithPixels    highestCluster = { .energy = -1234, .size = 0, .height = -1234, .pixels = new PXPPixel[cMaxStoredPixels] };

// simple logaritmic test view ====================================================================
string clusterPreview(PXPClusterWithPixels* cluster, string pref, string comment, unsigned frWid, unsigned frHei) {
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

    out << pref << comment << ":\n";
    out << pref << "------------------------------------------------------------------\n";

    string desc[16] = {
        "E [keV]", "in block", "-------",
		". >0", "+ >4", "* >16", "o >64", "O >256", "8 >1024", "# >4096", "", 
        "ToA: " + format("{:.2e}", cluster->toa) + " ns",
        "dt:  " + format("{:.2f}", toaMax - toaMin) + " ns",
        "siz: " + to_string(cluster->size) + " px", 
        "hei: " + format("{:.2f}", cluster->height) + " keV",
        "E:   " + format("{:.3f}", cluster->energy/1000.0) + " MeV"
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
        out << "| " << (y < 16 ? desc[y] : "") << "\n";
    }
    out << pref << "------------------------------------------------------------------\n";
    return out.str();
}
void clusterPreview(PXPClusterWithPixels* cluster, string pref, string comment) {
    cout << clusterPreview(cluster, pref, comment, devWidth, devHeight);
    cout << pref << "pixels (position, time relative [ns], energy [keV]):\n" << pref;
    for (unsigned n = 0; n < cluster->size; n++) {
        cout << "xy:" << cluster->pixels[n].x << "," << cluster->pixels[n].y << " tr:" << cluster->pixels[n].toa - cluster->toa << " e:" << cluster->pixels[n].energy << ", ";
        if (n > 12) { cout << "..."; break; }
        if (n % 3 == 2 && n < cluster->size - 1) cout << endl << pref;
    }
    cout << "\n" << pref << "==================================================================\n";
}
void clusterPreviewSave(PXPClusterWithPixels* cluster, string comment, ofstream* fout) {
	if (!fout->is_open()) return;
    fout->precision(3);
    (*fout) << clusterPreview(cluster, (string)"", comment, devWidth, devHeight);
    (*fout) << "pixels (position, time relative [ns], energy [keV]):\n";
    for (unsigned n = 0; n < cluster->size; n++) {
        (*fout) << "xy:" << cluster->pixels[n].x << "," << cluster->pixels[n].y << " tr:" << cluster->pixels[n].toa - cluster->toa << " e:" << cluster->pixels[n].energy << ", ";
        if (n % 4 == 3 && n < cluster->size - 1) (*fout) << endl;
    }
    (*fout) << "\n" << "==================================================================\n";
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

// 1. Load the Pixet core without try starting any devices
// 2. Create the Clustering handle
// 3. Load calibration to the Clustering
// 4. Set the callbacks
// 5. Do the experiment
int main() { // ###################################################################################
    int rc; // return code

    cout << "PXCAPI example - for details see https://wiki.advacam.cz/wiki/Binary_Clustering_API\n\n";

#ifdef PATH_TO_API
#ifndef CHDIRS_toAPI_off
    auto cwdOrig = filesystem::current_path();
    cout << "Original working directory:" << cwdOrig.string() << "\n";
    auto changeDirToAPI = [cwdOrig]() {
        try {
            filesystem::current_path(STR(PATH_TO_API));
        } catch (const filesystem::filesystem_error& e) {
            cerr << "Error changing working directory to " << STR(PATH_TO_API) << ":\n" << e.what() << '\n';
            return -1;
        }
        cout << "Changed WD to PATH_TO_API: " << filesystem::current_path().string() << "\n";
        return 0;
    };
    if (auto chrc = changeDirToAPI() != 0) return chrc;
#endif // !CHDIRS_toAPI_off
#endif // PATH_TO_API

    cout << "pxpClLoadPixetCore...\n";
    rc = pxpClLoadPixetCore("pxcore.dll");
    errHandler(rc, "pxpClLoadPixetCore");

    clHandle = pxpClCreate(0);
    if (clHandle == CL_INVALID_HANDLE) cout << "pxpClCreate INVALID\n";
    else cout << "pxpClCreate OK\n";

#ifdef PATH_TO_API
#ifndef CHDIRS_toAPI_off
#ifndef CHDIRS_back_off
    filesystem::current_path(cwdOrig);
    cout << "Returned WD to original:   " << filesystem::current_path().string() << "\n";
#endif // !CHDIRS_back_off
#endif // !CHDIRS_toAPI_off
#endif // PATH_TO_API
    // pxpClLoadCalibrationFromFiles(clhandle_t handle, const char* filePaths);
    rc = pxpClLoadCalibrationFromFiles(clHandle, TEST_pathConfig);
    errHandler(rc, "pxpClLoadCalibrationFromFiles");

    //pxpClSetMessageCallback(clhandle_t handle, ClMessageCallback callback, void* userData)
    rc = pxpClSetMessageCallback(clHandle, ClMessageCallbackFn, NULL);
    errHandler(rc, "pxpClSetMessageCallback");

    //pxpClSetProgressCallback(clhandle_t handle, ClProgressCallback callback, void* userData);
    rc = pxpClSetProgressCallback(clHandle, ClProgressCallbackFn, NULL);
    errHandler(rc, "pxpClSetProgressCallback");

    // arriving with data containing properties of the clusters with all their pixels
    rc = pxpClSetNewClustersWithPixelsCallback(clHandle, ClNewClustersWithPixelsCallbackFn, NULL);
    errHandler(rc, "pxpClSetNewClustersWithPixelsCallback");

    // arriving with data containing properties of the clusters with parameters only, no pixel data
    //rc = pxpClSetNewClustersCallback(clHandle, ClNewClustersCallbackFn, NULL);
    //errHandler(rc, "pxpClSetNewClustersCallback");

    // note: If registered both clusters callbacks, only second one will be called.

    string fName = (string)TEST_dir TEST_fileClusters;
    clustersMaxsFout.open(fName, ios::out);
    if (clustersMaxsFout.is_open()) {
        cout << "Opened file '" << fName << "' for cluster preview output.\n";
        cout << "Stream state: good:" << clustersMaxsFout.good() << ", fail:" << clustersMaxsFout.fail() << ", bad:" << clustersMaxsFout.bad() << "\n";
	} else {
		cout << "Failed to open file '" << fName << "' for cluster preview output. - View only.\n";
	}

    cout.precision(3);

    // use if there is a lot of data and clustering is slow due to noisy pixels
    //pxpClEnableFilteringOfNoisyPixels(clhandle_t handle, bool enable);
    //rc = pxpClEnableFilteringOfNoisyPixels(clHandle, true);
    //errHandler(rc, "pxpClEnableFilteringOfNoisyPixels");

	cout << "Start replaying data file '" << (TEST_dir TEST_fileT3pa) << "' ...\n";
    //pxpClReplayData(clhandle_t handle, const char* filePath, const char* outputFilePath, bool blocking);
    rc = pxpClReplayData(clHandle, (TEST_dir TEST_fileT3pa), (TEST_dir TEST_fileClog), false);
	errHandler(rc, "pxpClReplayData");

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
    if (clustersMaxsFout.is_open()) {
        clustersMaxsFout.close();
    }

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

    cout << "pxpClUnloadPixetCore...\n";
    pxpClUnloadPixetCore();
    cout << "pxpClUnloadPixetCore: done\n";
}