import os, sys, ctypes
from ctypes import byref, c_int, c_uint, c_char_p, c_char, c_void_p, POINTER, create_string_buffer

# Use apiPath "." for this .py in the API directory or some full path to API package in other location
#apiPath = "."
apiPath = "../../../API-nightly"
thisPath = os.path.dirname(os.path.abspath(__file__))

def _load_pxcore(api_path: str) -> ctypes.CDLL: # =======================================
    dll_path = os.path.join(api_path, "pxcore.dll")
    if not os.path.isfile(dll_path):
        raise FileNotFoundError(f"pxcore.dll not found at: {dll_path}")
    return ctypes.WinDLL(dll_path)


# typedef struct _Tpx3Pixel {double toa; float tot; unsigned int index;} Tpx3Pixel;
class clTpx3Pixel(ctypes.Structure):
    _fields_ = [("toa", ctypes.c_double), ("tot", ctypes.c_float), ("index", c_uint)]

def _bind_exports(px: ctypes.CDLL) -> None: # ===========================================
    px.pxcInitialize.argtypes = [c_char_p]
    px.pxcInitialize.restype = c_int

    px.pxcExit.argtypes = []
    px.pxcExit.restype = c_int

    px.pxcGetDevicesCount.argtypes = []
    px.pxcGetDevicesCount.restype = c_int

    px.pxcGetDeviceName.argtypes = [c_uint, POINTER(c_char), c_uint]
    px.pxcGetDeviceName.restype = c_int

    px.OPM_TOATOT = 0
    px.OPM_EVENT_ITOT = 2

    #int pxcSetTimepix3Mode(unsigned deviceIndex, int mode);
    px.pxcSetTimepix3Mode.argtypes = [c_uint, c_int]
    px.pxcSetTimepix3Mode.restype = c_int

    #int pxcGetDeviceDimensions(unsigned deviceIndex, unsigned* width, unsigned* height);
    px.pxcGetDeviceDimensions.argtypes = [c_uint, POINTER(c_uint), POINTER(c_uint)]
    px.pxcGetDeviceDimensions.restype

    #int pxcMeasureSingleFrameTpx3(unsigned deviceIndex, double frameTime, double* frameToaITot, unsigned short* frameTotEvent, unsigned* size, unsigned trgStg = PXC_TRG_NO);
    px.pxcMeasureSingleFrameTpx3.argtypes = [c_uint, ctypes.c_double, POINTER(ctypes.c_double), POINTER(ctypes.c_ushort), POINTER(c_uint), c_uint]
    px.pxcMeasureSingleFrameTpx3.restype = c_int

    #int pxcMeasureMultipleFramesWithCallback(unsigned deviceIndex, unsigned frameCount, double frameTime, unsigned trgStg = PXC_TRG_NO, FrameMeasuredCallback callback = 0, intptr_t userData = 0);
    px.pxcMeasureMultipleFramesWithCallback.argtypes = [c_uint, c_uint, ctypes.c_double, c_uint, ctypes.CFUNCTYPE(None, c_uint, c_void_p), c_void_p]
    px.pxcMeasureMultipleFramesWithCallback.restype = c_int

    #int pxcGetMeasuredFrameCount(unsigned deviceIndex);
    px.pxcGetMeasuredFrameCount.argtypes = [c_uint]
    px.pxcGetMeasuredFrameCount.restype = c_int

    #int pxcGetMeasuredFrameTpx3(unsigned deviceIndex, unsigned frameIndex, double* frameToaITot, unsigned short* frameTotEvent, unsigned* size);
    px.pxcGetMeasuredFrameTpx3.argtypes = [c_uint, c_uint, POINTER(ctypes.c_double), POINTER(ctypes.c_ushort), POINTER(c_uint)]
    px.pxcGetMeasuredFrameTpx3.restype = c_int

    #int pxcMeasureTpx3DataDrivenMode(unsigned deviceIndex, double measTime, const char* fileName, unsigned trgStg = PXC_TRG_NO, AcqEventFunc callback = 0, intptr_t userData = 0);
    px.pxcMeasureTpx3DataDrivenMode.argtypes = [c_uint, ctypes.c_double, c_char_p, c_uint, ctypes.CFUNCTYPE(None, c_void_p, c_char_p), c_void_p]
    px.pxcMeasureTpx3DataDrivenMode.restype = c_int

    #int pxcGetMeasuredTpx3PixelsCount(unsigned deviceIndex, unsigned* pixelCount);
    px.pxcGetMeasuredTpx3PixelsCount.argtypes = [c_uint, POINTER(c_uint)]
    px.pxcGetMeasuredTpx3PixelsCount.restype = c_int

    #int pxcGetMeasuredTpx3Pixels(unsigned deviceIndex, Tpx3Pixel* pixels, unsigned pixelCount);
    px.pxcGetMeasuredTpx3Pixels.argtypes = [c_uint, POINTER(clTpx3Pixel), POINTER(c_uint)]
    px.pxcGetMeasuredTpx3Pixels.restype = c_int

    px.PXC_ACQEVENT_ACQ_FAILED = b"AcqFailed"
    px.PXC_ACQEVENT_MEAS_STARTED = b"AcqMeasStarted"
    px.PXC_ACQEVENT_MEAS_FINISHED = b"AcqMeasFinished"

    #int pxcRegisterAcqEvent(unsigned deviceIndex, const char* event, AcqEventFunc func, intptr_t userData);
    px.pxcRegisterAcqEvent.argtypes = [c_uint, c_char_p, ctypes.CFUNCTYPE(None, c_uint, c_void_p), c_void_p]
    px.pxcRegisterAcqEvent.restype = c_int

    #int pxcUnregisterAcqEvent(unsigned deviceIndex, const char* event, AcqEventFunc func, intptr_t userData);
    px.pxcUnregisterAcqEvent.argtypes = [c_uint, c_char_p, ctypes.CFUNCTYPE(None, c_uint, c_void_p), c_void_p]
    px.pxcUnregisterAcqEvent.restype = c_int

def _showError(px: ctypes.CDLL) -> int: # ===============================================
    buf = create_string_buffer(4096)
    rc = px.pxcGetLastError(buf, ctypes.sizeof(buf))
    if rc != 0: print("(pxcGetLastError failed", rc, ")")
    else: print("Err:", buf.value.decode("utf-8", errors="replace"))
    return rc

def _testShowError(name, rc, text=""): # ================================================
    if rc < 0:
        print(f"{name} failed - rc:{rc}", text)
        _showError(px)
# ---------------------------------------------------------------------------------------

devPixels = 65536 # singlechip default
frameToaITot = None #(ctypes.c_double * devPixels)()
frameTotEvent = None #(ctypes.c_ushort * devPixels)()

# typedef void (*FrameMeasuredCallback)(intptr_t acqCount, intptr_t userData);
FrameMeasuredCallback = ctypes.CFUNCTYPE(None, ctypes.c_uint, ctypes.c_void_p)

@FrameMeasuredCallback
def frameMeasuredCallback(acqCount, userData): # ========================================
    print(f"***clb-frame: acqCount:{acqCount} usr:{userData}")

    size = ctypes.c_uint(devPixels)
    rc = px.pxcGetMeasuredFrameTpx3(devIdx, acqCount-1, frameToaITot, frameTotEvent, ctypes.byref(size))
    _testShowError("    pxcGetMeasuredFrameTpx3", rc, f"size:{size.value}")

    ToaItMin = min(frameToaITot)
    ToaItMax = max(frameToaITot)
    TotEvMin = min(frameTotEvent)
    TotEvMax = max(frameTotEvent)

    hits = 0
    for n in range(devPixels):
        if frameToaITot[n] != 0 | frameTotEvent[n] != 0: hits += 1

    print("    Hits:", hits, "ToaIt min/max:", ToaItMin, ToaItMax, "TotEv min/max:", TotEvMin, TotEvMax)

@FrameMeasuredCallback
def acqStartCallback(acqCount, userData): # =============================================
    print(f"***clb-acqStart: acqCount:{acqCount} usr:{userData}")

@FrameMeasuredCallback
def acqFinCallback(acqCount, userData): # ===============================================
    print(f"***clb-acqFinish: acqCount:{acqCount} usr:{userData}")

@FrameMeasuredCallback
def acqErrCallback(val, userData): # ====================================================
    print(f"***clb-acqErr: rc:{val-4294967295} usr:{userData}")
    buf = create_string_buffer(4096)
    rc = px.pxcGetLastError(buf, ctypes.sizeof(buf))
    if rc != 0: print("(pxcGetLastError failed", rc, ")")
    else: print("    Err:", buf.value.decode("utf-8", errors="replace"))

# ---------------------------------------------------------------------------------------

tpx3PixelsBuffSize = 1000000
tpx3PixelsBuff = (clTpx3Pixel * tpx3PixelsBuffSize)()

# typedef void (*AcqEventFunc)(intptr_t eventData, intptr_t userData);
PixelsMeasuredCllback = ctypes.CFUNCTYPE(None, ctypes.c_void_p, ctypes.c_char_p)

@PixelsMeasuredCllback
def pixelsMeasuredCallback(eventData, userData): # ======================================
    tag = userData.decode("utf-8", errors="replace") if userData else None
    print(f"***clb-pixels: eventData:{eventData} usr:{tag}")
    pxCnt = c_uint()
    rc = px.pxcGetMeasuredTpx3PixelsCount(devIdx, byref(pxCnt))
    _testShowError("    pxcGetMeasuredTpx3PixelsCount", rc, f"pxCnt:{pxCnt}")
    print("    Pixels count:", pxCnt.value)

    pixelsToRead = min(pxCnt.value, tpx3PixelsBuffSize)

    rc = px.pxcGetMeasuredTpx3Pixels(devIdx, tpx3PixelsBuff, byref(c_uint(pixelsToRead)))
    _testShowError("    pxcGetMeasuredTpx3Pixels", rc, f"pixelsToRead:{pixelsToRead}")

    totMin = 1e10
    totMax = -1
    toaMin = 1e100
    toaMax = -1
    for n in range(pixelsToRead):
        if tpx3PixelsBuff[n].tot < totMin: totMin = tpx3PixelsBuff[n].tot
        if tpx3PixelsBuff[n].tot > totMax: totMax = tpx3PixelsBuff[n].tot
        if tpx3PixelsBuff[n].toa < toaMin: toaMin = tpx3PixelsBuff[n].toa
        if tpx3PixelsBuff[n].toa > toaMax: toaMax = tpx3PixelsBuff[n].toa
        if n<10:
            print(f"    - Pixel {n}: index:{tpx3PixelsBuff[n].index} tot:{tpx3PixelsBuff[n].tot} toa:{tpx3PixelsBuff[n].toa/1e9} s")
    if pixelsToRead>10:
        print(f"    (pixels to {pixelsToRead-1} not listed)")

    print("    Tot min/max:", totMin, totMax, "Toa min/max:", toaMin, toaMax)
# ---------------------------------------------------------------------------------------

if apiPath != ".": os.chdir(apiPath) # change dir to API root, otherwise some devices cannot be initialized

px = _load_pxcore(apiPath)
_bind_exports(px)

print("pxcore init...")
rc = px.pxcInitialize(b"pixet.ini")
print("pxcInitialize rc:", rc, "(0 is OK)")
if rc != 0:
    _showError(px)
    exit()

if apiPath != ".": os.chdir(thisPath) # change dir back or somewhere

try:
    devIdx = 0
    devCnt = px.pxcGetDevicesCount()
    if devCnt < 0:
        _showError(px)
        raise RuntimeError ("pxcGetDevicesCount failed")

    print("Devices found:", devCnt)
    if devCnt <= 0: raise RuntimeError("No devices found")

    name_buf = create_string_buffer(512)
    for n in range(devCnt):
        rc = px.pxcGetDeviceName(n, name_buf, ctypes.sizeof(name_buf))
        if rc == 0:
            dname = name_buf.value.decode("utf-8", errors="replace")
            print("  n:", n, "name:", dname)
            if dname[0] in "MA": devIdx = n # Minipix / Advapix
        else:
            print("  n: pxcGetDeviceName rc:", rc, end=" ")
            _showError(px)

    print("Selected device:", devIdx, "(if it isn't Tpx3, next code cannot work properly)\n")
    devWidth = c_uint()
    devHeight = c_uint()
    rc = px.pxcGetDeviceDimensions(devIdx, byref(devWidth), byref(devHeight))
    if rc == 0:
        devPixels = devWidth.value * devHeight.value
        print("Dev dimensions:", devWidth.value, "x", devHeight.value, "pixels:", devPixels)
        frameToaITot = (ctypes.c_double * devPixels)()
        frameTotEvent = (ctypes.c_ushort * devPixels)()
    else:
        print("pxcGetDeviceDimensions rc:", rc)
        _showError(px)

    rc = px.pxcSetTimepix3Mode(devIdx, px.OPM_EVENT_ITOT)
    print("pxcSetTimepix3Mode rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)

    rc = px.pxcRegisterAcqEvent(devIdx, px.PXC_ACQEVENT_MEAS_STARTED, acqStartCallback, None)
    print("pxcRegisterAcqEvent (start) rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)

    rc = px.pxcRegisterAcqEvent(devIdx, px.PXC_ACQEVENT_MEAS_FINISHED, acqFinCallback, None)
    print("pxcRegisterAcqEvent (finish) rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)

    rc = px.pxcRegisterAcqEvent(devIdx, px.PXC_ACQEVENT_ACQ_FAILED , acqErrCallback, None)
    print("pxcRegisterAcqEvent (failed) rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)

    print("pxcMeasureMultipleFramesWithCallback...")
    rc = px.pxcMeasureMultipleFramesWithCallback(devIdx, 5, 1.0, 0, frameMeasuredCallback, None)
    print("pxcMeasureMultipleFramesWithCallback rc:", rc, "(0 is OK)")
    if rc != 0:
        _showError(px)
        raise RuntimeError ("pxcMeasureMultipleFramesWithCallback failed")

    rc = px.pxcSetTimepix3Mode(devIdx, px.OPM_TOATOT)
    print("pxcSetTimepix3Mode rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)

    # userData can be used to transfer device info to callback in multi-device setup 
    user_tag = ctypes.create_string_buffer(name_buf.value) #f"devIdx:{devIdx}".encode("utf-8"))
    print("pxcMeasureTpx3DataDrivenMode...")
    rc = px.pxcMeasureTpx3DataDrivenMode(devIdx, 10.0, None, 0, pixelsMeasuredCallback, user_tag)
    print("pxcMeasureTpx3DataDrivenMode rc:", rc, "(0 is OK)")
    if rc != 0:
        _showError(px)
        raise RuntimeError ("pxcMeasureTpx3DataDrivenMode failed")

    rc = px.pxcUnregisterAcqEvent(devIdx, px.PXC_ACQEVENT_MEAS_STARTED, acqStartCallback, None)
    print("pxcUnregisterAcqEvent (start) rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)

    rc = px.pxcUnregisterAcqEvent(devIdx, px.PXC_ACQEVENT_MEAS_FINISHED, acqFinCallback, None)
    print("pxcUnregisterAcqEvent (finish) rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)

    rc = px.pxcUnregisterAcqEvent(devIdx, px.PXC_ACQEVENT_ACQ_FAILED, acqErrCallback, None)
    print("pxcUnregisterAcqEvent (failed) rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)


except Exception as e:
    print("Exception:", e)

finally:
    print("pxcore exit...")
    rc2 = px.pxcExit()
    print("pxcExit rc:", rc2, "(0 is OK)")
    if rc2 != 0: _showError(px)
    else:        print("Done")