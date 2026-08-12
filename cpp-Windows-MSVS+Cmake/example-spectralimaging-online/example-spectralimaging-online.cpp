#include <windows.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>
using namespace std;

// (c) 2026 Pavel Hudecek, Advacam, https://advacam.cz, https://wiki.advacam.cz/wiki/Binary_core_API
//
// Spectral imaging: Measure data -> convert to clusters -> sort by energy -> place clusters centers as pixels to image with energy filtering
// 
// Example with device connected. This example has variants by measMode values m/r/b:
// 1.m: measure with online processing,
//   r: replay t3pa data file (and measure to file if not exist),
//   b: load BSTG saved in previous run. Filename contains the measTime value.
//   (load BSTG can be also used by aborting a meas/replay by holding the B key)
// 2. Save the BSTG file after measurement or replay.
// 3. Show output of the pxpSiGetGlobalSpectrum.
// 4. Run the pxpSiSaveDataAsSpectrumToFile and pxpSiSaveDataAsFramesToFile functions.
// 5. Preview outputs of the pxpSiGetFrameForEnergy and pxpSiGetFrameForEnergyRange functions.
// 6. Show outputs of the pxpSiGetGlobalSpectrumInRect in the corner and in the middle of the chip.
// (default settings optimized for Minipix-Tpx3/CdTe with source gold-coated 241Am, 2.5 kBq placed on opened window)

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
#include API_HEADER(spectraimgapi.h)
#else
#include "common.h"
#include "pxcapi.h"
#include "spectraimgapi.h"
#endif // PATH_TO_API

char measMode = 'b'; // 'm' = measure with online processing, 'r' = replay data file (and measure to file if not exist), 'b' = load previous BSTG
// Note: 'b' loads data with it's settings. Next settings in this section does not affect the result.
double measTime = 40;               // set the total acquisition time [s]
int spectFrom = 10, spectTo = 3600; // set the energy range [keV]
double spectStep = 20;              // set the energy step [keV]
bool maskNoisyPixels = true, doSubPixCorrection = false;
bool correctXrf = true;             // use XRF corr. - default for CdTe sensor only, false for Si
// or use pxpSiSetXrfCorrectionParams to set the XRF correction parameters usable for Your sensor material.

#define TEST_dir "../x64/Debug/test-files/"
#define TEST_fileT3pa "test.t3pa"
#define TEST_fileClog "test.clog"

string bstgFilePath(string endStr="") {
	ostringstream oss;
	oss << TEST_dir "test-" << fixed << setprecision(1) << measTime << endStr << ".bstg";
	return oss.str();
}

sihandle_t siHandle = nullptr;
void* iPixet = nullptr;

bool errHandler(int rc, string funcName, bool silent = false) { // ===========================
    if (silent && rc == 0) return true; // if silent and OK, do not print anything
    cout << funcName << ": " << rc << " ";
    if (rc == 0) {
        cout << "(0 is OK)\n";
    } else if (rc > 0) {
        cout << "(output val)\n";
    } else {
        char buff[500];
        pxpSiGetLastError(siHandle, buff, 500);
        cerr << "\n  err msg: '" << buff << "'\n";
    }
    return rc >= 0;
}

unsigned devIdx = 0;
uint32_t devWidth, devHeight;
uint32_t devPixelsCount = 0;

// message callback in the Windows CLR app
void siMessageCallbackFn(bool error, const char* message, void* userData) { // ====================
    cout << "clb Msg: err:" << (int)error << ", msg:'" << message << "'\n";
}

void siProgressCallbackFn(bool finished, double progress, void* userData) { // ====================
    cout << "clb progress:" << fixed << setprecision(2) << progress << " %, " << "fin:" << finished << " . Hold key 1s A: abort / B: load '" << bstgFilePath() << "'\n";
    // notes: 1. Placing abort here looks more elegant, but aborting called from callbacks takes multiple longer.
    //        2. Keypress testing method is global for all running programs (hold "a" in Notepad causes abort)
}

// simple logaritmic test view ====================================================================
string framePreview(double* frame, string pref, string comment, unsigned frWid, unsigned frHei, int prec=3) {
    ostringstream out;
    unsigned xdiv = frWid / 64;
    unsigned ydiv = frHei / 32;
	double min = 1e100, max = -1e100, sum = 0.0;

    out << pref << comment << ":\n" << pref;
    out << pref << "------------------------------------------------------------------\n";

	const int cDescLines = 10;
    string desc[cDescLines] = {
        "Hits", "in block", "-------",
        ". >0", "+ >4", "* >16", "o >64", "O >256", "8 >1024", "# >4096"
    };
    for (unsigned y = 0; y < 32; y++) {
        out << pref << "|";
        for (unsigned x = 0; x < 64; x++) {
            double val = 0;
            for (unsigned yy = 0; yy < ydiv; yy++) {
                for (unsigned xx = 0; xx < xdiv; xx++) {
                    unsigned pxIdx = (x * xdiv + xx) + (y * ydiv + yy) * frWid;
                    if (pxIdx < frWid * frHei) {
                        double v = frame[pxIdx];
                        if (v < min) min = v;
                        if (v > max) max = v;
						sum += v;
                        val += v;
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

    string str = out.str();
    out.str("");
	int pos = -1;
	for (int i = 0; i < cDescLines + 4; i++) pos = str.find("\n", pos + 1);
	out << str.substr(0, pos);
    out << setprecision(prec) << "Min: " << min << "\n";
	int pos2 = str.find("\n", pos + 1);
	out << str.substr(pos + 1, pos2 - pos - 1);
	out << setprecision(prec) << "Max: " << max << "\n";
	int pos3 = str.find("\n", pos2 + 1);
	out << str.substr(pos2 + 1, pos3 - pos2 - 1);
    out << setprecision(prec) << "Sum: " << sum << "\n";
	out << str.substr(pos3 + 1);

    return out.str();
}

// 1. Initialize the Pixet core
// 2. Get/show devices info
// 3. Load factory config
// 4. (Do dummy acquisition if will be measuring)
// 5. (Measure to T3PA file if will be replaying and the file does not exist)
// 6. Set-up the IPixet and create the Spectral imaging handle
// 7. Load calibration to the Spectral imaging
// 8. Set the callbacks
// 9. Do the experiments
int main() { // ###################################################################################
    int rc; // return code

    cout << "PXCAPI example - for details see https://wiki.advacam.cz/wiki/Binary_core_API\n\n";

#ifdef PATH_TO_API
#ifndef CHDIRS_toAPI_off
    auto cwdOrig = filesystem::current_path();
    cout << "Original working directory:" << cwdOrig << "\n";
    auto changeDirToAPI = [cwdOrig]() {
        try {
            filesystem::current_path(STR(PATH_TO_API));
        } catch (const filesystem::filesystem_error& e) {
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
        cout << "  Cannot get device dimensions, trying using default 256x256.\n";
        devWidth = 256; devHeight = 256;
        devPixelsCount = devWidth * devHeight;
    }
    cout << "  " << devWidth << "x" << devHeight << " pxCnt:" << devPixelsCount << "\n";

    rc = pxcLoadFactoryConfig(devIdx);
    errHandler(rc, "pxcLoadFactoryConfig");
    if (rc == -1027) {
        cout << "Create the 'factory' subdir and copy the factory config files there.\n";
        cout << "Or set the FactoryDir= in the [settings] section of the pixet.ini file\n";
    }

    auto dummyAcq = [](string pref = "") { // ---------------------------------
        cout << pref << "Warning: Measuring immediately after init may cause the first data contains power-on artefacts.\n";
        cout << pref << "Dummy acq...\n";
        int rc = pxcMeasureMultipleFrames(devIdx, 5, 1.0);
        errHandler(rc, pref + "pxcMeasureMultipleFrames");
    };

    if (measMode == 'r') {
        string t3paPath;
        if (std::filesystem::path(TEST_dir).is_absolute())
            t3paPath = TEST_dir TEST_fileT3pa;  // absolute path
        else
            // relative path 
            t3paPath = cwdOrig.string() + (string("/\\").find(TEST_dir[0]) == string::npos ? "/" : "") + TEST_dir TEST_fileT3pa;

        cout << "Replay data file:'" << t3paPath << "'\n";
        if (!filesystem::exists(t3paPath)) {
            cout << "The file not exist, new will be measured:\n";
            dummyAcq("  ");

            rc = pxcFindNoisyPixels(devIdx, 20.0, 20.0, true);
            errHandler(rc, "  pxcFindNoisyPixels");

            rc = pxcSetTimepix3Mode(devIdx, PXC_TPX3_OPM_TOATOT);
            errHandler(rc, "  pxcSetTimepix3Mode");

            cout << "  Measuring new, wait for " << measTime << " seconds...\n";
            cout << "  pxcMeasureTpx3DataDrivenMode...\n";
            rc = pxcMeasureTpx3DataDrivenMode(devIdx, measTime, t3paPath.c_str());
            errHandler(rc, "  pxcMeasureTpx3DataDrivenMode");
        }
    }
    if (measMode == 'm') dummyAcq();

    // Warning: Use the pxcGetIPixet from pxcapi.h, not pxpSiGetIPixet from spectraimagingapi.h
    iPixet = pxcGetIPixet();
    if (iPixet == nullptr) cerr << "pxcGetIPixet returned nullptr\n";
    else cout << "pxcGetIPixet OK\n";

    pxpSiSetIPixet(iPixet);

    siHandle = pxpSiCreate(devIdx);
    if (siHandle == SI_INVALID_HANDLE) cout << "pxpSiCreate INVALID\n";
    else cout << "pxpSiCreate OK\n";

#ifdef PATH_TO_API
#ifndef CHDIRS_toAPI_off
#ifndef CHDIRS_back_off
    filesystem::current_path(cwdOrig);
    cout << "Returned WD to original:   " << filesystem::current_path() << "\n";
#endif // !CHDIRS_back_off
#endif // !CHDIRS_toAPI_off
#endif // PATH_TO_API

    rc = pxpSiLoadCalibrationFromDevice(siHandle);
    errHandler(rc, "pxpSiLoadCalibrationFromDevice");

    // offline alternative or for replay data measured by other than now running device
    // pxpSiLoadCalibrationFromFiles(sihandle_t handle, const char* filePaths);
    //rc = pxpSiLoadCalibrationFromFiles(siHandle, "..\\x64\\Debug\\configs\\MiniPIX-D05-W0037.xml");
    //errHandler(rc, "pxpSiLoadCalibrationFromFiles");
	// alternative can be used exported abct files 'calib-a.txt|calib-b.txt|calib-c.txt|calib-t.txt'

    //pxpClSetMessageCallback(clhandle_t handle, ClMessageCallback callback, void* userData)
    rc = pxpSiSetMessageCallback(siHandle, siMessageCallbackFn, NULL);
    errHandler(rc, "pxpSiSetMessageCallback");

    //pxpClSetProgressCallback(clhandle_t handle, ClProgressCallback callback, void* userData);
    rc = pxpSiSetProgressCallback(siHandle, siProgressCallbackFn, NULL);
    errHandler(rc, "pxpSiSetProgressCallback");

    //pxpSiSetMeasParams(sihandle_t handle, int spectFrom, int spectTo, double spectStep, bool maskNoisyPixels, bool doSubPixCorrection, bool correctXrf);
	rc = pxpSiSetMeasParams(siHandle, spectFrom, spectTo, spectStep, maskNoisyPixels, doSubPixCorrection, correctXrf);
    errHandler(rc, "pxpSiSetMeasParams");

	bool bstgLoaded = false;
	bool aborted = false;

    switch (measMode) {
        case 'm':
            cout << "Start measuring with online processing for " << measTime << " s (" << measTime/3600 << " hod)...\n";
            //pxpSiStartMeasurement(sihandle_t handle, double acqTime, double measTime, const char* outputFilePath, bool processData);
            rc = pxpSiStartMeasurement(siHandle, measTime, measTime, (TEST_dir TEST_fileClog), true);
            //rc = pxpSiStartMeasurement(siHandle, measTime, measTime, "", true);
            errHandler(rc, "pxpSiStartMeasurement");
            break;
        case 'r':
            // pxpSiReplayData(sihandle_t handle, const char* filePath, const char* outputFilePath);
            rc = pxpSiReplayData(siHandle, (TEST_dir TEST_fileT3pa), "");//(TEST_dir TEST_fileClog));
            errHandler(rc, "pxpSiReplayData");
            break;  
	    case 'b':
		    cout << "pxpSiLoadFromFile(" << bstgFilePath() << ")...\n";
		    //pxpSiLoadFromFile(sihandle_t handle, const char* filePath);
		    rc = pxpSiLoadFromFile(siHandle, bstgFilePath().c_str());
		    errHandler(rc, "pxpSiLoadFromFile");
		    if (rc == 0) bstgLoaded = true;
            break;
        default:
            cout << "Unknown measMode value '" << measMode << "', use 'm', 'r' or 'b'\n";
            break;
    }

    cout << "===============================================================================\n";
    while (pxpSiIsRunning(siHandle) > 0 && measMode != 'b') {
        Sleep(100);
		static int abortCnt = 0;
        if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState('B') & 0x8000) {
			bool bPressed = (GetAsyncKeyState('B') & 0x8000) != 0;
			abortCnt++;
            if (abortCnt < 8) continue;
			aborted = true;
            cout << "while (pxpSiIsRunning: aborting (" << (bPressed ? "B" : "A") << ")...\n";
            rc = pxpSiAbort(siHandle);
            errHandler(rc, "pxpSiAbort");
            while (pxpSiIsRunning(siHandle) > 0) Sleep(100);
			if (bPressed) {
                string bstgPath = bstgFilePath();
                cout << "pxpSiLoadFromFile(" << bstgPath << ")...\n";
				rc = pxpSiLoadFromFile(siHandle, bstgPath.c_str());
				errHandler(rc, "pxpSiLoadFromFile");
				if (rc == SI_ERR_CANNOT_OPEN_FILE) {
                    bstgPath = bstgFilePath("a");
                    cout << "pxpSiLoadFromFile(" << bstgPath << ")...\n";
                    rc = pxpSiLoadFromFile(siHandle, bstgPath.c_str());
                    errHandler(rc, "pxpSiLoadFromFile");
				}
                if (rc == 0) bstgLoaded = true;
            }
            if (bstgLoaded) cout << "aborted: Last BSTG data loaded instead of measured data.\n";
            else cout << "aborted: Dataset is smaller than the requested measTime.\n";
            // note: Placing it in a progress callback looks more elegant, but aborting called from callbacks takes multiple longer.
        } else { 
			abortCnt = 0;
        }
    }
    cout << "===============================================================================\n";

    if (rc==0 && !bstgLoaded) {
		string bstgPath = bstgFilePath(aborted ? "a" : "");
		cout << "pxpSiSaveToFile(" << bstgPath << ")...\n";
		// pxpSiSaveToFile(sihandle_t handle, const char* filePath);
		rc = pxpSiSaveToFile(siHandle, bstgPath.c_str());
		errHandler(rc, "pxpSiSaveToFile");
	}

	if (bstgLoaded) {
		//pxpSiGetMeasParams(sihandle_t handle, int* spectFrom, int* spectTo, double* spectStep, bool* maskNoisyPixels, bool* doSubPixCorrection, bool* correctXrf);
		rc = pxpSiGetMeasParams(siHandle, &spectFrom, &spectTo, &spectStep, &maskNoisyPixels, &doSubPixCorrection, &correctXrf);
		errHandler(rc, "pxpSiGetMeasParams");
		cout << "  Meeas params of loaded data:\n  spectFrom:" << spectFrom << ", spectTo:" << spectTo << ", spectStep:" << spectStep
			<< ", maskNoisyPixels:" << maskNoisyPixels << ", doSubPixCorrection:" << doSubPixCorrection
			<< ", correctXrf:" << correctXrf << "\n";
	}

    //pxpSiSpectrumSize(sihandle_t handle);
	rc = pxpSiSpectrumSize(siHandle);
	errHandler(rc, "pxpSiSpectrumSize");
	int spectCount = (rc>0) ? rc : (spectTo - spectFrom) / spectStep;

	unsigned* spData = new unsigned[spectCount];
    double spStep = spectStep;
	size_t spSize = spectCount;
    //pxpSiGetGlobalSpectrum(sihandle_t handle, unsigned* data, double* step, size_t* size);
	rc = pxpSiGetGlobalSpectrum(siHandle, spData, &spStep, &spSize);
	errHandler(rc, "pxpSiGetGlobalSpectrum");
	cout << "Spectrum count: " << spectCount << ", step: " << spStep << ", size: " << spSize << "\nData:";
	for (unsigned ei = 0; ei < spectCount; ei++) {
		cout << spData[ei] << " ";
	}
	cout << "\n\n";

	cout << "pxpSiSaveDataAsSpectrumToFile...\n";
    // pxpSiSaveDataAsSpectrumToFile(sihandle_t handle, const char* filePath);
	rc = pxpSiSaveDataAsSpectrumToFile(siHandle, (TEST_dir "/spectrum.txt"));
    errHandler(rc, "pxpSiSaveDataAsSpectrumToFile");

	cout << "pxpSiSaveDataAsFramesToFile...\n";
    // pxpSiSaveDataAsFramesToFile(sihandle_t handle, const char* filePath, bool oneFile);
	rc = pxpSiSaveDataAsFramesToFile(siHandle, (TEST_dir "/frames.txt"), true);
    errHandler(rc, "pxpSiSaveDataAsFramesToFile");

	double* frameData = new double[devWidth * devHeight * 4*4];
	size_t frameWidth = devWidth, frameHeight = devHeight;

    auto estr = [=](int ei, string endStr="") { // energy info string ---------
        ostringstream oss;
        if (spectFrom > 10 && spectTo > 1000) {
            oss << ei << ":" << (int)(spectFrom + ei * spectStep) << endStr;
		}
		else oss << fixed << setprecision(2) << ei << ":" << (spectFrom + ei * spectStep) << endStr;
        return oss.str();
    };

	bool normalize = false;
    for (unsigned ei = 0; ei < spectCount/30; ei++) {
        // pxpSiGetFrameForEnergy(sihandle_t handle, unsigned energyIndex, bool sumFrame, bool normalize, int zoom, double* frameData, size_t* width, size_t* height);
        rc = pxpSiGetFrameForEnergy(siHandle, ei, false, normalize, 1, frameData, &frameWidth, &frameHeight);
        errHandler(rc, "pxpSiGetFrameForEnergy");
        if (rc == 0) cout << framePreview(frameData, "", "GetFrameForEnergy " + estr(ei, " keV") + " norm:" + (normalize ? "1" : "0"), frameWidth, frameHeight) << "\n";
    }

	for (unsigned ei = spectCount/3; ei < spectCount-5-1; ei+=5) {
        unsigned e1 = ei;
        unsigned e2 = ei + 5;
        //pxpSiGetFrameForEnergyRange(sihandle_t handle, unsigned energyIndexFrom, unsigned energyIndexTo, bool normalize, double* frameData, size_t* width, size_t* height);
        rc = pxpSiGetFrameForEnergyRange(siHandle, e1, e2, normalize, frameData, &frameWidth, &frameHeight);
        errHandler(rc, "pxpSiGetFrameForEnergyRange");
		if (rc==0) cout << framePreview(frameData, "", "GetFrameForEnergyRange " + estr(e1, " to ") + estr(e2, " keV"), frameWidth, frameHeight) << "\n";
	}

    // pxpSiGetFrameForEnergy(sihandle_t handle, unsigned energyIndex, bool sumFrame, bool normalize, int zoom, double* frameData, size_t* width, size_t* height);
    rc = pxpSiGetFrameForEnergy(siHandle, 0, true, normalize, 1, frameData, &frameWidth, &frameHeight);
    errHandler(rc, "pxpSiGetFrameForEnergy - sumframe");
    if (rc == 0) cout << framePreview(frameData, "", "GetFrameForEnergy sum ", frameWidth, frameHeight) << "\n";

    int x1 = 10, y1 = 10, x2 = 70, y2 = 70;
    //pxpSiGetGlobalSpectrumInRect(sihandle_t handle, unsigned x1, unsigned y1, unsigned x2, unsigned y2, unsigned* data, double* step, size_t* size);
    rc = pxpSiGetGlobalSpectrumInRect(siHandle, x1, y1, x2, y2, spData, &spStep, &spSize);
    errHandler(rc, "pxpSiGetGlobalSpectrumInRect - corner");
    if (rc == 0) {
        cout << "xy1: " << x1 << "," << y1 << " xy2: " << x2 << "," << y2 << "\nData:";
        for (unsigned ei = 0; ei < spectCount; ei++) {
            cout << spData[ei] << " ";
        }
        cout << "\n\n";
    }

    x1 = devWidth/2-20, y1 = devHeight / 2 - 20, x2 = devWidth / 2 + 20, y2 = devHeight / 2 + 20;
    rc = pxpSiGetGlobalSpectrumInRect(siHandle, x1, y1, x2, y2, spData, &spStep, &spSize);
    errHandler(rc, "pxpSiGetGlobalSpectrumInRect - center");
	if (rc == 0) {
        cout << "xy1: " << x1 << "," << y1 << " xy2: " << x2 << "," << y2 << "\nData:";
        for (unsigned ei = 0; ei < spectCount; ei++) {
            cout << spData[ei] << " ";
        }
        cout << "\n\n";
    }

#ifdef PATH_TO_API
#ifndef CHDIRS_toAPI_off
    if (auto chrc = changeDirToAPI() != 0) return chrc;
#endif // !CHDIRS_toAPI_off
#endif // PATH_TO_API

    cout << "pxcExit...\n";
    rc = pxcExit();
    cout << "pxcExit: " << rc << " (0 is OK)\n";
}