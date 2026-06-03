#include <iostream>
#include <filesystem>  
using namespace std;








/*******************************
 
Warning - this is not complete       <<<-------------------------------

*******************************/















// (c) 2026 Pavel Hudecek, Advacam, https://advacam.cz, https://wiki.advacam.cz/wiki/Binary_core_API
//
// This example: 1. measure some frames and do something with it in the callback in lambda fn.
//               2. measure in the data driven mode and do something with the data in the callback in lambda fn.



/* Example CMakeList.txt, if You want to use CMake: -------------------------------------
cmake_minimum_required(VERSION 3.10)
project(example-callbacks)

# include_directories(${CMAKE_SOURCE_DIR})
# link_directories(${CMAKE_SOURCE_DIR})
add_library(pxcore SHARED IMPORTED)
set_property(TARGET pxcore PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/pxcore.dll")
set_property(TARGET pxcore PROPERTY IMPORTED_IMPLIB  "${CMAKE_SOURCE_DIR}/pxcore.lib")
add_executable(example-callbacks example-callbacks.cpp)
target_link_libraries(example-callbacks pxcore)
---------------------------------------------------------------------------------------*/

#define PATH_TO_API ../x64/Debug
// Not defined: Copy whole API package to the project directory and add pxcore.lib to the linker/input in project
//   settings or in CMakeLists.txt. Than create the 'factory' subdir and copy the factory config files there,
//   or set the FactoryDir= in the [settings] section of the pixet.ini file.
// - You can select minimal files needed, allways pxcapi.h, common.h, pxcore.dll, pxcore.lib, pixet.ini, lic.info
//   and hwlib with files needed, like as minipix.dll+ftd2xx64.dll for Minipix / zest.dll+zest.ini+zestwpx.bit for Widepix with Eth.
// Defined: Set PATH_TO_API to the path to the API package, and add pxcore.lib with path of API package
//   to the linker/input in project settings or in CMakeLists.txt.  Factory config same as "Not defined".
// - The example will:  change the working directory to PATH_TO_API before starting the Pixet core,
//                      than change it back, running the example,
//                      than change to PATH_TO_API again before exiting the core.
// Notes:
// - The pxcapi.h including common.h if it not in project directory, include it separatelly before the pxcapi.
// - The pxcore.dll must be in directory wit the executable, eq. "../x64/Debug" in both cases.
//  Therefore, the easiest way is to copy the API package there so that it can be shared by all projects in the solution.

#ifdef PATH_TO_API
#define STR2(x) #x
#define STR(x) STR2(x)
#define API_HEADER(file) STR(PATH_TO_API/file)
#include API_HEADER(common.h)
#include API_HEADER(pxcapi.h)
#else
#include "pxcapi.h"
#endif // PATH_TO_API

bool errHandler(int rc, const char* funcName, bool silent = false, const char* prefix = "") { // ======
	if (silent && rc == 0) return true; // if silent and OK, do not print anything
	cout << prefix << funcName << ": " << rc << " ";
    if (rc == 0) {
        cout << "(0 is OK)\n";
	} else if (rc>0) {
        cout << "(output val)\n";
    } else {
        char buff[500];
        pxcGetLastError(buff, 500);
        cerr << endl << prefix << "  err msg: '" << buff << "'\n";
    }
    return rc >= 0;
}

unsigned devIdx = 0;
DevType devType = (DevType)(-1);
uint32_t devPixelsCount = 0;
double* frameDouble = nullptr;
float* frameFloat = nullptr;
unsigned short* frame16b = nullptr;
unsigned* frame32b1 = nullptr;
unsigned* frame32b2 = nullptr;
unsigned pixelsBuffSize = 100000000;
Tpx3Pixel* measuredPixels = nullptr;

template <typename T, typename U> void showSampleData(const T* data1, const U* data2, const char* prefix="") { // ========
	T data1Min = std::numeric_limits<T>::max(), data1Max = std::numeric_limits<T>::min();
    U data2Min = std::numeric_limits<U>::max(), data2Max = std::numeric_limits<U>::min();
    uint32_t n;
	for (n = 0; n < devPixelsCount; n++) {
		if (data1[n] < data1Min) data1Min = data1[n];
		if (data1[n] > data1Max) data1Max = data1[n];
		if (data2[n] < data2Min) data2Min = data2[n];
		if (data2[n] > data2Max) data2Max = data2[n];
	}
	cout << prefix << "Min-max: data1:" << data1Min << "-" << data1Max << " data2:" << data2Min << "-" << data2Max << "\n";
    n = devPixelsCount / 2 - 20;
    cout << prefix << "Sample frorm idx:" << n << " data1,data2 ----------------------------" << endl << "  ";
    for (; n < devPixelsCount / 2 + 20; n++) {
        cout << data1[n] << "," << data2[n] << "  ";
        if (n % 10 == 0) cout << "\n" << prefix << "  ";
    }
    cout << "\n---------------------------------------------------------------\n";
}

void clbFrameMeasured(intptr_t acqCount, intptr_t userData) { // ==================================
    cout << "***\tFrameMeasuredCallback: acqCount:" << acqCount << ", userData:'" << (char*)userData << "'\n";
    int rc;
    unsigned size = devPixelsCount;
    switch (devType) {
        case DevType::TPX3:
            // pxcGetMeasuredFrameTpx3(unsigned deviceIndex, unsigned frameIndex, double* frameToaITot, unsigned short* frameTotEvent, unsigned* size);
            rc = pxcGetMeasuredFrameTpx3(0, acqCount - 1, frameDouble, frame16b, &size);
            errHandler(rc, "pxcGetMeasuredFrameTpx3", true, "\t");
	        if (rc == 0) showSampleData(frameDouble, frame16b, "\t");

            cout << "pxcGetMeasuredCalibratedFrameTpx3 (not implemented at this time)\n";

            break;
        case DevType::TPX2:
		    // pxcGetMeasuredFrameTpx2(unsigned deviceIndex, unsigned frameIndex, unsigned* frameData1, unsigned* frameData2, unsigned* size);
		    rc = pxcGetMeasuredFrameTpx2(0, acqCount - 1, frame32b1, frame32b2, &size);
		    errHandler(rc, "pxcGetMeasuredFrameTpx2", true, "\t");
		    if (rc == 0) showSampleData(frame32b1, frame32b2, "\t");

            size = devPixelsCount;
            // pxcGetMeasuredCalibratedFrameTpx2(unsigned deviceIndex, unsigned frameIndex, double* frameData1, unsigned* frameData2, unsigned* size);
		    rc = pxcGetMeasuredCalibratedFrameTpx2(0, acqCount - 1, frameDouble, frame32b2, &size);
		    errHandler(rc, "pxcGetMeasuredCalibratedFrameTpx2", true, "\t");
		    if (rc == 0) showSampleData(frameDouble, frame32b2, "\t");
		    break;
	    default:
		    cout << "\tUnknown device type, cannot get measured frame data.\n";
    }

}

void clbAcqEventFunc(intptr_t eventData, intptr_t userData) { // ==================================
    cout << "***\tAcqEventFunc: eventData:" << eventData << ", userData:'" << (char*)userData << "'\n";
    static int totalPixels = 0;
    int rc;
    unsigned size, received;
    rc = pxcGetMeasuredTpx3PixelsCount(devIdx, &received);
    errHandler(rc, "pxcGetMeasuredTpx3PixelsCount", true, "\t");
    size = received;
    totalPixels += received;
    if (size > pixelsBuffSize) {
        cout << "\tToo much pixels measured, count:" << size << " > buff:" << pixelsBuffSize << " pixels over will be ignored.\n";
        size = pixelsBuffSize;
    }
	if (received < 1) { cout << "\ttotal:" << totalPixels << " (No pixels received in this clb)\n"; return; }

    // pxcGetMeasuredTpx3Pixels(unsigned deviceIndex, Tpx3Pixel* pixels, unsigned pixelCount);
    rc = pxcGetMeasuredTpx3Pixels(devIdx, measuredPixels, size);
    errHandler(rc, "pxcGetMeasuredTpx3Pixels", true, "\t");
    double toaMin = 1e100, toaMax = -1e100;
    float totMin = 65535, totMax = 0;
    float totCalibMin = 1e10, totCalibMax = -1e10;
    for (unsigned i = 0; i < size; i++) {
        if (measuredPixels[i].toa < toaMin) toaMin = measuredPixels[i].toa;
        if (measuredPixels[i].toa > toaMax) toaMax = measuredPixels[i].toa;
        if (measuredPixels[i].tot < totMin) totMin = measuredPixels[i].tot;
        if (measuredPixels[i].tot > totMax) totMax = measuredPixels[i].tot;
    }
    // pxcCalibrateTpx3PixelsAndFilter(unsigned deviceIndex, Tpx3Pixel* pixels, unsigned* pixelCount, double minEnergy, double maxEnergy);
    rc = pxcCalibrateTpx3PixelsAndFilter(devIdx, measuredPixels, &size, 20, 1e10);
    errHandler(rc, "pxcCalibrateTpx3PixelsAndFilter", true, "\t");
    for (unsigned i = 0; i < size; i++) {
        if (measuredPixels[i].tot < totCalibMin) totCalibMin = measuredPixels[i].tot;
        if (measuredPixels[i].tot > totCalibMax) totCalibMax = measuredPixels[i].tot;
    }
    cout.precision(2);
    cout << scientific;
    cout << "\tProcessed " << size << "/" << received << ": total:" << totalPixels << " ToA min-max:" << toaMin << "-" << toaMax << " ns\n";
    cout << "\tToT min-max:" << fixed << totMin << "-" << totMax << " ticks; Energy (ToT calib) min-max:" << scientific << totCalibMin << "-" << totCalibMax << " keV\n";
}

int main() { // ===================================================================================
    int rc; // return code

	cout << "PXCAPI example - for details see https://wiki.advacam.cz/wiki/Binary_core_API\n\n";

#ifdef PATH_TO_API
    auto cwdOrig = filesystem::current_path();
    cout <<     "Original working directory:" << cwdOrig << "\n";
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
#endif // PATH_TO_API

    cout << "pxcInitialize...\n";
    rc = pxcInitialize();
    if (!errHandler(rc, "pxcInitialize")) return rc;

#ifdef PATH_TO_API
	filesystem::current_path(cwdOrig);
	cout <<    "Returned WD to original:   " << filesystem::current_path() << "\n";
#endif
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
        pxcGetDeviceName(n, name, 100);
        cout << "  Dev " << n << ": name:'" << name << "'\n";
    }
    devIdx = 0;
    devType = (DevType)pxcGetDeviceInfo(devIdx, NULL);
    const char* dtStrings[] = { "Tpx", "Mpx3", "Tpx3", "Tpx2" };
    cout << "Using dev:" << devIdx << " Type:" << dtStrings[(int)devType - 1] << "\n";
    char devName[100];
    rc = pxcGetDeviceName(devIdx, devName, 100);
    errHandler(rc, "pxcGetDeviceName");

    rc = pxcLoadFactoryConfig(devIdx);
	errHandler(rc, "pxcLoadFactoryConfig");
    if (rc==-1027) {
        cout << "Create the 'factory' subdir and copy the factory config files there.\n";
        cout << "Or set the FactoryDir= in the [settings] section of the pixet.ini file\n";
    }

    uint32_t devWidth, devHeight;
	rc = pxcGetDeviceDimensions(devIdx, &devWidth, &devHeight);
    errHandler(rc, "pxcGetDeviceDimensions");
    if (rc == 0) {
        devPixelsCount = devWidth * devHeight;
    } else {
		cout << "Cannot get device dimensions, trying using default 256x256.\n";
        devWidth = 256; devHeight = 256;
		devPixelsCount = devWidth * devHeight;
    }
	frameDouble = new double[devPixelsCount];
    frameFloat = new float[devPixelsCount];
    frame16b = new unsigned short[devPixelsCount];
	frame32b1 = new unsigned[devPixelsCount];
	frame32b2 = new unsigned[devPixelsCount];
    measuredPixels = new Tpx3Pixel[pixelsBuffSize];

	cout << "Warning: Measuring immediately after init may cause the first data contains power-on artefacts.\n";

	unsigned size = devPixelsCount;
    switch (devType) 
        case DevType::TPX3: {
            rc = pxcSetTimepix3Mode(devIdx, PXC_TPX3_OPM_TOATOT);
            errHandler(rc, "pxcSetTimepix3Mode");

            rc = pxcSetTimepix3CalibrationEnabled(devIdx, false);
            errHandler(rc, "pxcSetTimepix3CalibrationEnabled-0");

            cout << "pxcMeasureSingleFrameTpx3...\n";
            //pxcMeasureSingleFrameTpx3(unsigned deviceIndex, double frameTime, double* frameToaITot, unsigned short* frameTotEvent, unsigned* size, unsigned trgStg = PXC_TRG_NO);
            rc = pxcMeasureSingleFrameTpx3(devIdx, 1.0, frameDouble, frame16b, &size, PXC_TRG_NO);
            errHandler(rc, "pxcMeasureSingleFrameTpx3");

		    if (rc == 0) showSampleData(frameDouble, frame16b);

            rc = pxcSetTimepix3CalibrationEnabled(devIdx, true);
            errHandler(rc, "pxcSetTimepix3CalibrationEnabled-1");
            rc = pxcIsTimepix3CalibrationEnabled(devIdx);
			errHandler(rc, "pxcIsTimepix3CalibrationEnabled");
            size = devPixelsCount;

            cout << "pxcMeasureSingleCalibratedFrameTpx3 (not implemented at this time)\n";

            cout << "pxcMeasureSingleFrameTpx3...\n";
            rc = pxcMeasureSingleFrameTpx3(devIdx, 1.0, frameDouble, frame16b, &size, PXC_TRG_NO);
            errHandler(rc, "pxcMeasureSingleFrameTpx3");

            if (rc == 0) showSampleData(frameDouble, frame16b);

            cout << "pxcMeasureTpx3DataDrivenMode...\n";
            // pxcMeasureTpx3DataDrivenMode(unsigned deviceIndex, double measTime, const char* fileName, unsigned trgStg = PXC_TRG_NO, AcqEventFunc callback = 0, intptr_t userData = 0);
            rc = pxcMeasureTpx3DataDrivenMode(devIdx, 20, "", PXC_TRG_NO, clbAcqEventFunc, (intptr_t)&devName);
            errHandler(rc, "pxcMeasureTpx3DataDrivenMode");

            break;
        case DevType::TPX2:
            rc = pxcSetTimepix2Mode(devIdx, PXC_TPX2_OPM_TOT10_TOA18);
            errHandler(rc, "pxcSetTimepix2Mode");

			rc = pxcSetTimepix2CalibrationEnabled(devIdx, false);
			errHandler(rc, "pxcSetTimepix2CalibrationEnabled-0");

            cout << "pxcMeasureSingleFrameTpx2...\n";
		    rc = pxcMeasureSingleFrameTpx2(devIdx, 1.0, frame32b1, frame32b2, &size, PXC_TRG_NO);
            errHandler(rc, "pxcMeasureSingleFrameTpx2");

            cout << "Dada sample: idx:data1,data2 -------------------" << endl << "  ";
			for (uint32_t n = size / 2 + devWidth/2 - 20; n < size / 2 + devWidth/2 + 20; n++) {
				cout << n << ":" << frame32b1[n] << "," << frame32b2[n] << "  ";
				if (n>3 && n % 6 == 0) cout << "\n  ";
			}
            cout << "\n------------------------------------------------\n";

			rc = pxcSetTimepix2CalibrationEnabled(devIdx, true);
			errHandler(rc, "pxcSetTimepix2CalibrationEnabled-1");
            size = devPixelsCount;

            cout << "pxcMeasureSingleCalibratedFrameTpx2...\n";
            //pxcMeasureSingleCalibratedFrameTpx2(unsigned deviceIndex, double frameTime, double* frameData1, unsigned* frameData2, unsigned* size, unsigned trgStg);
			rc = pxcMeasureSingleCalibratedFrameTpx2(devIdx, 1.0, frameDouble, frame32b2, &size, PXC_TRG_NO);
            errHandler(rc, "pxcMeasureSingleCalibratedFrameTpx2");

			cout << "Dada sample: idx:data1,data2 -------------------" << endl << "  ";
			for (uint32_t n = size / 2 + devWidth/2 - 20; n < size / 2 + devWidth/2 + 20; n++) {
				cout << n << ":" << frameDouble[n] << "," << frame32b2[n] << "  ";
				if (n>3 && n % 6 == 0) cout << "\n  ";
			}
            cout << "\n------------------------------------------------\n";

            break;
        case DevType::TPX:
            rc = pxcSetTimepixMode(devIdx, PXC_TPX_MODE_TOT);
            errHandler(rc, "pxcSetTimepixMode");

			rc = pxcSetTimepixCalibrationEnabled(devIdx, false);
			errHandler(rc, "pxcSetTimepixCalibrationEnabled-0");

			cout << "pxcMeasureSingleFrame (Tpx)...\n";
		    rc = pxcMeasureSingleFrame(devIdx, 1.0, frame16b, &size, PXC_TRG_NO);
            errHandler(rc, "pxcMeasureSingleFrame");

			cout << "Dada sample: idx:ToT -----------------------------" << endl << "  ";
			for (uint32_t n = size / 2 + devWidth/2 - 20; n < size / 2 + devWidth/2 + 20; n++) {
				cout << n << ":" << frame16b[n] << "  ";
				if (n>3 && n % 8 == 0) cout << "\n  ";
			}
            cout << "\n------------------------------------------------\n";

			rc = pxcSetTimepixCalibrationEnabled(devIdx, true);
			errHandler(rc, "pxcSetTimepixCalibrationEnabled-1");

			cout << "pxcMeasureSingleCalibratedFrame (Tpx) is not implemented at this time\n";

			cout << "pxcMeasureSingleFrame (Tpx)...\n";
			rc = pxcMeasureSingleFrame(devIdx, 1.0, frame16b, &size, PXC_TRG_NO);
            errHandler(rc, "pxcMeasureSingleFrame");
			if (rc != 0) break;

			cout << "Dada sample: idx:ToT -----------------------------" << endl << "  ";
			for (uint32_t n = size / 2 + devWidth / 2 - 20; n < size / 2 + devWidth / 2 + 20; n++) {
				cout << n << ":" << frame16b[n] << "  ";
				if (n > 3 && n % 8 == 0) cout << "\n  ";
			}
            cout << "\n------------------------------------------------\n";

            break;
        default:
			if (devType == DevType::MPX3) cout << "Medipix3 device\n";
            else                          cout << "(Unknown device type)\n";
            cout << "No energy measurement - no calibration\n";
            cout << "pxcExit...\n";
            rc = pxcExit();
            errHandler(rc, "pxcExit");
            return 0;
    }

    // pxcMeasureMultipleFramesWithCallback(deviceIndex, framesCount, timeoutSec, trgMode, callbackFunc, userData);
    cout << "pxcMeasureMultipleFramesWithCallback...\n";
    rc = pxcMeasureMultipleFramesWithCallback(devIdx, 3, 2.0, PXC_TRG_NO, clbFrameMeasured, (intptr_t)&devName);
    errHandler(rc, "pxcMeasureMultipleFramesWithCallback");

    rc = pxcSetTimepix3CalibrationEnabled(devIdx, true);
    errHandler(rc, "pxcSetTimepix3CalibrationEnabled-1");



#ifdef PATH_TO_API
    if (auto chrc = changeDirToAPI() != 0) return chrc;
#endif // PATH_TO_API

    cout << "pxcExit...\n";
    rc = pxcExit();
    errHandler(rc, "pxcExit");
}