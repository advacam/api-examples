#include <iostream>
#include <filesystem>  
using namespace std;

// (c) 2026 Pavel Hudecek, Advacam, https://advacam.cz, https://wiki.advacam.cz/wiki/Binary_core_API
//
// This example: 1. measure some frames and do something with it in the callback functions.
//               2. measure in the data-driven mode and do something with the data in the callback functions.



/* Example CMakeList.txt, if You want to use CMake: -------------------------------------
cmake_minimum_required(VERSION 3.10)
project(example-callbacks2)

# include_directories(${CMAKE_SOURCE_DIR})
# link_directories(${CMAKE_SOURCE_DIR})
add_library(pxcore SHARED IMPORTED)
set_property(TARGET pxcore PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/pxcore.dll")
set_property(TARGET pxcore PROPERTY IMPORTED_IMPLIB  "${CMAKE_SOURCE_DIR}/pxcore.lib")
add_executable(example-callbacks2 example-callbacks2.cpp)
target_link_libraries(example-callbacks2 pxcore)
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

bool errHandler(int rc, const char* funcName, bool silent = false) { // ===========================
	if (silent && rc == 0) return true; // if silent and OK, do not print anything
	cout << funcName << ": " << rc << " ";
    if (rc == 0) {
        cout << "(0 is OK)\n";
	} else if (rc>0) {
        cout << "(output val)\n";
    } else {
        char buff[500];
        pxcGetLastError(buff, 500);
        cerr << "\n  err msg: '" << buff << "'\n";
    }
    return rc >= 0;
}

unsigned devIdx = 0;
uint32_t devPixelsCount = 0;
double* frameToaITot = nullptr;
unsigned short* frameTotEvent = nullptr;
unsigned pixelsBuffSize = 100000000;
Tpx3Pixel* measuredPixels = nullptr;

void clbFrameMeasured(intptr_t acqCount, intptr_t userData) { // ==================================
    cout << "***\tFrameMeasuredCallback: acqCount:" << acqCount << ", userData:'" << (char*)userData << "'\n";
    int rc;
    unsigned size = devPixelsCount;
    // pxcGetMeasuredFrameTpx3(unsigned deviceIndex, unsigned frameIndex, double* frameToaITot, unsigned short* frameTotEvent, unsigned* size);
    rc = pxcGetMeasuredFrameTpx3(0, acqCount - 1, frameToaITot, frameTotEvent, &size);
    errHandler(rc, "\tpxcGetMeasuredFrameTpx3", true);
    double toaMin = 1e100, toaMax = -1e100;
    unsigned totMin = 65535, totMax = 0;
    unsigned hitPixels = 0;
    for (unsigned i = 0; i < size; i++) {
        if (frameToaITot[i] < toaMin) toaMin = frameToaITot[i];
        if (frameToaITot[i] > toaMax) toaMax = frameToaITot[i];
        if (frameTotEvent[i] < totMin) totMin = frameTotEvent[i];
        if (frameTotEvent[i] > totMax) totMax = frameTotEvent[i];
        if (frameTotEvent[i]>0 || frameToaITot[i]!=0) hitPixels++;
    }
    cout << "\tFrame: ToA min:" << toaMin << " max:" << toaMax << " ns; Tot min:" << totMin << " max:" << totMax << " ticks; Hit pixels:" << hitPixels << "\n";
}

void clbAcqEventFunc(intptr_t eventData, intptr_t userData) { // ==================================
    cout << "***\tAcqEventFunc: eventData:" << eventData << ", userData:'" << (char*)userData << "'\n";
    static int totalPixels = 0;
    int rc;
    unsigned size, received;
    rc = pxcGetMeasuredTpx3PixelsCount(devIdx, &received);
    errHandler(rc, "\tpxcGetMeasuredTpx3PixelsCount", true);
    size = received;
    totalPixels += received;
    if (size > pixelsBuffSize) {
        cout << "\tToo much pixels measured, count:" << size << " > buff:" << pixelsBuffSize << " pixels over will be ignored.\n";
        size = pixelsBuffSize;
    }
    // pxcGetMeasuredTpx3Pixels(unsigned deviceIndex, Tpx3Pixel* pixels, unsigned pixelCount);
    rc = pxcGetMeasuredTpx3Pixels(devIdx, measuredPixels, size);
    errHandler(rc, "\tpxcGetMeasuredTpx3Pixels", true);
    double toaMin = 1e100, toaMax = -1e100;
    unsigned totMin = 65535, totMax = 0;
    for (unsigned i = 0; i < size; i++) {
        if (measuredPixels[i].toa < toaMin) toaMin = measuredPixels[i].toa;
        if (measuredPixels[i].toa > toaMax) toaMax = measuredPixels[i].toa;
        if (measuredPixels[i].tot < totMin) totMin = measuredPixels[i].tot;
        if (measuredPixels[i].tot > totMax) totMax = measuredPixels[i].tot;
    }
    if (received < 1) { cout << "\ttotal:" << totalPixels << " (No pixels received in this clb)\n"; return; }
    cout << "\tProcessed " << size << "/" << received << ": total:" << totalPixels << " ToA min:" << toaMin << " max:" << toaMax << " ns; Tot min:" << totMin << " max:" << totMax << " ticks\n";
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
        rc = pxcGetDeviceName(n, name, 100);
        errHandler(rc, "  pxcGetDeviceName", true);
        cout << "  Dev " << n << ": name:'" << name << "'\n";
    }
	devIdx = 0;
	cout << "Using dev:" << devIdx << "\n";
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
	frameToaITot = new double[devPixelsCount];
    frameTotEvent = new unsigned short[devPixelsCount];
	measuredPixels = new Tpx3Pixel[pixelsBuffSize];

    rc = pxcSetTimepix3Mode(devIdx, PXC_TPX3_OPM_TOATOT); // sets OPM of device with index 0 to ToA+ToT
	errHandler(rc, "pxcSetTimepix3Mode");
    if (rc < 0) {
        cout << "If the device is not Timepix3, use pxcSetTimepixMode/pxcSetTimepix2Mode/pxcSetMedipix3OperationMode.\n";
	}

	cout << "Warning: Measuring immediately after init may cause the first data contains power-on artefacts.\n";
	cout << "pxcMeasureMultipleFramesWithCallback...\n";

	// pxcMeasureMultipleFramesWithCallback(deviceIndex, framesCount, timeoutSec, trgMode, callbackFunc, userData);
    rc = pxcMeasureMultipleFramesWithCallback(devIdx, 3, 2.0, PXC_TRG_NO, clbFrameMeasured, (intptr_t)&devName);
	errHandler(rc, "pxcMeasureMultipleFramesWithCallback");


    cout << "pxcMeasureTpx3DataDrivenMode...\n";
    // pxcMeasureTpx3DataDrivenMode(unsigned deviceIndex, double measTime, const char* fileName, unsigned trgStg = PXC_TRG_NO, AcqEventFunc callback = 0, intptr_t userData = 0);
	rc = pxcMeasureTpx3DataDrivenMode(devIdx, 20, "", PXC_TRG_NO, clbAcqEventFunc, (intptr_t)&devName);
	errHandler(rc, "pxcMeasureTpx3DataDrivenMode");

#ifdef PATH_TO_API
    if (auto chrc = changeDirToAPI() != 0) return chrc;
#endif // PATH_TO_API

    cout << "pxcExit...\n";
    rc = pxcExit();
    cout << "pxcExit: " << rc << " (0 is OK)\n";
}