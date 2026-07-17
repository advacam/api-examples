#include <windows.h>

#include <iostream>
#include <filesystem>  
using namespace std;

// (c) 2026 Pavel Hudecek, Advacam, https://advacam.cz, https://wiki.advacam.cz/wiki/Binary_core_API
//
// This example:
// 1. dummy measuring to empty power-on artefacts.
// 2. find and mask noisy pixels.
// 3. change minimum repetition time of the data callback to be faster than default.
// 4. measure in the data-driven mode
// 5. do something with the data in the callback, but only if the 'A' key is pressed, otherwise ignore the data.
//
// It can be used in XRD to maximize usable measuring time in combination with moving of the mechanics.
// The program can simply measuring permanently to avoid dead time around starting and stopping measurements
// The data will be ignored while the mechanics are moving, and processed as long as the mechanics stand.



// See example-basic.cpp about compilation and API package location considerations.
#define PATH_TO_API ../x64/Debug

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
unsigned pixelsBuffSize = 100000000;
Tpx3Pixel* measuredPixels = nullptr;

unsigned clbCnt = 0, totalPixels = 0, ignoredPixels = 0;
void clbAcqEventFunc(intptr_t eventData, intptr_t userData) { // ==================================
    cout << "***\tAcqEventFunc: eventData:" << eventData << ", userData:" << (uint64_t)userData << ", cnt:" << clbCnt << "\n";
	clbCnt++;
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
    if (GetAsyncKeyState('A') & 0x8000) {
        for (unsigned i = 0; i < size; i++) {
            if (measuredPixels[i].toa < toaMin) toaMin = measuredPixels[i].toa;
            if (measuredPixels[i].toa > toaMax) toaMax = measuredPixels[i].toa;
            if (measuredPixels[i].tot < totMin) totMin = measuredPixels[i].tot;
            if (measuredPixels[i].tot > totMax) totMax = measuredPixels[i].tot;
        }
        if (received < 1) { cout << "\ttotal:" << totalPixels << " (No pixels received in this clb)\n"; return; }
        cout << "\tProcessed " << size << "/" << received << ": total:" << totalPixels << " ToA min:" << toaMin << " max:" << toaMax << " ns; Tot min:" << totMin << " max:" << totMax << " ticks\n";
	} else {
		ignoredPixels += received;
		cout << "\t(not pressed 'A' key - " << received << " pixels ignored)\n";
    }
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
	measuredPixels = new Tpx3Pixel[pixelsBuffSize];

	cout << "Warning: Measuring immediately after init may cause the first data contains power-on artefacts.\n";
    
    cout << "Dummy acq...\n";
    rc = pxcMeasureMultipleFrames(devIdx, 5, 1.0);
    errHandler(rc, "pxcMeasureMultipleFrames");
    
    cout << "pxcFindNoisyPixels...";
    // pxcFindNoisyPixels(unsigned deviceIndex, double limitNoisy = 50, double limitSatur = 50, bool doMaskNoisyPixels = false,
    // unsigned* noisyPixelsMatrix = nullptr, unsigned matrixSize = 65536);
    rc = pxcFindNoisyPixels(devIdx, 10, 10, true);
    errHandler(rc, "pxcFindNoisyPixels");

    rc = pxcSetTimepix3Mode(devIdx, PXC_TPX3_OPM_TOATOT); // sets OPM of device with index 0 to ToA+ToT
	errHandler(rc, "pxcSetTimepix3Mode");
    if (rc < 0) {
        cout << "If the device is not Timepix3, use pxcSetTimepixMode/pxcSetTimepix2Mode/pxcSetMedipix3OperationMode.\n";
	}

    cout << "Warning: Setting DDEventMinRepTime too lower than 0.5 sec. if config shared with Pixet GUI can cause freezing of GUI while measuring.\n";

    rc = pxcSetDeviceParameterDouble(devIdx, "DDEventMinRepTime", 0.01);
    errHandler(rc, "pxcSetDeviceParameterDouble DDEventMinRepTime");
    if (rc != 0) cout << "This is new feature in 1.8.6 version since 06.2026 please upgrade\n";
    double dparval = -123.456;
    rc = pxcGetDeviceParameterDouble(devIdx, "DDEventMinRepTime", &dparval);
	errHandler(rc, "pxcGetDeviceParameterDouble DDEventMinRepTime");
    cout << "val:" << dparval << endl;


	double acqTime = 20; // seconds
    clbCnt = 0;
    totalPixels = 0;
	ignoredPixels = 0;
    cout << "pxcMeasureTpx3DataDrivenMode...\n";
    cout << "--------------------------------------------------------------------------------\n";
    // pxcMeasureTpx3DataDrivenMode(unsigned deviceIndex, double measTime, const char* fileName, unsigned trgStg = PXC_TRG_NO, AcqEventFunc callback = 0, intptr_t userData = 0);
    rc = pxcMeasureTpx3DataDrivenMode(devIdx, acqTime, "", PXC_TRG_NO, clbAcqEventFunc, (intptr_t)0);
	cout << "--------------------------------------------------------------------------------\n";
    errHandler(rc, "pxcMeasureTpx3DataDrivenMode");
    cout << "clbCnt:" << clbCnt << ", clb/s:" << (clbCnt / acqTime) << ", totalPixels:" << totalPixels << ", ignoredPixels:" << ignoredPixels << "\n";


	rc = pxcSetDeviceParameterDouble(devIdx, "DDEventMinRepTime", 0.5); // restore default value (need only if configs shared with Pixet GUI)
    errHandler(rc, "pxcSetDeviceParameterDouble DDEventMinRepTime restore default");

#ifdef PATH_TO_API
    if (auto chrc = changeDirToAPI() != 0) return chrc;
#endif // PATH_TO_API

    cout << "pxcExit...\n";
    rc = pxcExit();
    cout << "pxcExit: " << rc << " (0 is OK)\n";
}