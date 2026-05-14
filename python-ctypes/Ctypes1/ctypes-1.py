import os, sys, ctypes
from ctypes import byref, c_int, c_uint, c_char_p, c_char, POINTER, create_string_buffer

# Use apiPath "." for this .py in the API directory or some full path to API package in other location
#apiPath = "."
apiPath = "../../../API-nightly"
thisPath = os.path.dirname(os.path.abspath(__file__))

def _load_pxcore(api_path: str) -> ctypes.CDLL:
    dll_path = os.path.join(api_path, "pxcore.dll")
    if not os.path.isfile(dll_path):
        raise FileNotFoundError(f"pxcore.dll not found at: {dll_path}")
    return ctypes.WinDLL(dll_path)

def _bind_exports(px: ctypes.CDLL) -> None:
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


def _showError(px: ctypes.CDLL) -> int:
    buf = create_string_buffer(4096)
    rc = px.pxcGetLastError(buf, ctypes.sizeof(buf))
    if rc != 0: print("(pxcGetLastError failed", rc, ")")
    else: print("Err:", buf.value.decode("utf-8", errors="replace"))
    return rc

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
            print("  n:", n, "name:", name_buf.value.decode("utf-8", errors="replace"))
        else:
            print("  n: pxcGetDeviceName rc:", rc, end=" ")
            _showError(px)

    pxIdx = 0
    print("Selected device:", pxIdx, "(if it isn't Tpx3, next code cannot work properly)")
    devWidth = c_uint()
    devHeight = c_uint()
    devPixels = 65536 # default val. for singlechip
    rc = px.pxcGetDeviceDimensions(pxIdx, byref(devWidth), byref(devHeight))
    if rc == 0:
        devPixels = devWidth.value * devHeight.value
        print("Dev dimensions:", devWidth.value, "x", devHeight.value, "pixels:", devPixels)
    else:
        print("pxcGetDeviceDimensions rc:", rc)
        _showError(px)

    rc = px.pxcSetTimepix3Mode(pxIdx, px.OPM_EVENT_ITOT)
    print("pxcSetTimepix3Mode rc:", rc, "(0 is OK)")
    if rc!= 0: _showError(px)

    frameToaITot = (ctypes.c_double * devPixels)()
    frameTotEvent = (ctypes.c_ushort * devPixels)()
    size = ctypes.c_ulong(devPixels)

    print("pxcMeasureSingleFrameTpx3...")
    rc = px.pxcMeasureSingleFrameTpx3(pxIdx, 1.0, frameToaITot, frameTotEvent, byref(size), 0)
    print("pxcMeasureSingleFrameTpx3 rc:", rc, "(0 is OK) size:", size.value)
    if rc != 0:
        _showError(px)
        raise RuntimeError ("pxcMeasureSingleFrameTpx3 failed")

    ToaItMin = min(frameToaITot)
    ToaItMax = max(frameToaITot)
    TotEvMin = min(frameTotEvent)
    TotEvMax = max(frameTotEvent)

    hits = 0
    for n in range(devPixels):
        if frameToaITot[n] != 0 | frameTotEvent[n] != 0: hits += 1

    print("Frame stats: hits:", hits, "ToaIt min/max:", ToaItMin, ToaItMax, "TotEv min/max:", TotEvMin, TotEvMax)

except Exception as e:
    print("Exception:", e)

finally:
    print("pxcore exit...")
    rc2 = px.pxcExit()
    print("pxcExit rc:", rc2, "(0 is OK)")
    if rc2 != 0: _showError(px)
    else:        print("Done")