#include <iostream>
#include <filesystem>  
using namespace std;

// (c) 2026 Pavel Hudecek, Advacam, https://advacam.cz, https://wiki.advacam.cz/wiki/Binary_core_API
// This example simply measure some frames and save it to files.


/* Example CMakeList.txt, if You want to use CMake: -------------------------------------
cmake_minimum_required(VERSION 3.10)
project(example)

# include_directories(${CMAKE_SOURCE_DIR})
# link_directories(${CMAKE_SOURCE_DIR})
add_library(pxcore SHARED IMPORTED)
set_property(TARGET pxcore PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/pxcore.dll")
set_property(TARGET pxcore PROPERTY IMPORTED_IMPLIB  "${CMAKE_SOURCE_DIR}/pxcore.lib")
add_executable(example example-basic.cpp)
target_link_libraries(example pxcore)
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

bool errHandler(int rc, const char* funcName) { // ================================================
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
	cout << "Using dev 0\n";

    rc = pxcLoadFactoryConfig(0);
	errHandler(rc, "pxcLoadFactoryConfig");
    if (rc==-1027) {
        cout << "Create the 'factory' subdir and copy the factory config files there.\n";
        cout << "Or set the FactoryDir= in the [settings] section of the pixet.ini file\n";
    }

    rc = pxcSetTimepix3Mode(0, PXC_TPX3_OPM_TOATOT); // sets OPM of device with index 0 to ToA+ToT
	errHandler(rc, "pxcSetTimepix3Mode");
    if (rc < 0) {
        cout << "If the device is not Timepix3, use pxcSetTimepixMode/pxcSetTimepix2Mode/pxcSetMedipix3OperationMode.\n";
	}

	cout << "Warning: Measuring immediately after init may cause the first data contains power-on artefacts.\n";
	cout << "pxcMeasureMultipleFrames...\n";

    // pxcMeasureMultipleFrames(deviceIndex, frameCount, acqTime, triggerSettings);
    rc = pxcMeasureMultipleFrames(0, 3, 1, PXC_TRG_NO);
	errHandler(rc, "pxcMeasureMultipleFrames");

    // pxcSaveMeasuredFrame(deviceIndex, frameLastIndex, filename);
    rc = pxcSaveMeasuredFrame(0, 0, "test-files/testImg0.png");
	errHandler(rc, "pxcSaveMeasuredFrame 0");
    rc = pxcSaveMeasuredFrame(0, 1, "test-files/testImg1.txt");
    errHandler(rc, "pxcSaveMeasuredFrame 1");
    rc = pxcSaveMeasuredFrame(0, 2, "test-files/testImg2.pbf");
    errHandler(rc, "pxcSaveMeasuredFrame 2");

    cout << "pxcMeasureTpx3DataDrivenMode...\n";
	rc = pxcMeasureTpx3DataDrivenMode(0, 5, "test-files/testDataDriven.t3pa", PXC_TRG_NO);
	errHandler(rc, "pxcMeasureTpx3DataDrivenMode");

#ifdef PATH_TO_API
    if (auto chrc = changeDirToAPI() != 0) return chrc;
#endif // PATH_TO_API

    cout << "pxcExit...\n";
    rc = pxcExit();
    cout << "pxcExit: " << rc << " (0 is OK)\n";
}