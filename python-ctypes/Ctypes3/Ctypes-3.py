import os, sys, ctypes
from ctypes import byref, c_int, c_uint, c_char_p, c_char, c_void_p, POINTER, create_string_buffer

# Use apiPath "." for this .py in the API directory or some full path to API package in other location
#apiPath = "."
apiPath = "C:/Advacam/API-nightly"
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

    #int pxcGetDeviceDimensions(unsigned deviceIndex, unsigned* width, unsigned* height);
    px.pxcGetDeviceDimensions.argtypes = [c_uint, POINTER(c_uint), POINTER(c_uint)]
    px.pxcGetDeviceDimensions.restype

    #int pxcAddBHMask(unsigned* data, unsigned size, double frameTime, double thickness);
    px.pxcAddBHMask.argtypes = [POINTER(c_uint), c_uint, ctypes.c_double, ctypes.c_double]
    px.pxcAddBHMask.restype = c_int

    #int pxcBHMaskCount();
    px.pxcBHMaskCount.argtypes = []
    px.pxcBHMaskCount.restype = c_int

    #int pxcRemoveBHMask(int index);
    px.pxcRemoveBHMask.argtypes = [c_int]
    px.pxcRemoveBHMask.restype = c_int

    #int pxcApplyBHCorrection(unsigned* inData, unsigned size, double frameTime, double* outData);
    px.pxcApplyBHCorrection.argtypes = [POINTER(c_uint), c_uint, ctypes.c_double, POINTER(ctypes.c_double)]
    px.pxcApplyBHCorrection.restype = c_int

    #int pxcGetDeviceBadPixelMatrix(unsigned deviceIndex, unsigned char* badPixelMatrix, unsigned size);
    px.pxcGetDeviceBadPixelMatrix.argtypes = [c_uint, POINTER(c_char), c_uint]
    px.pxcGetDeviceBadPixelMatrix.restype = c_int

    #int pxcGetBHBadPixelMatrix(unsigned char* badPixelMatrix, unsigned size);
    px.pxcGetBHBadPixelMatrix.argtypes = [c_uint, POINTER(c_char), c_uint]
    px.pxcGetBHBadPixelMatrix.restype = c_int

    #int pxcGetDeviceAndBHBadPixelMatrix(unsigned deviceIndex, unsigned char* badPixelMatrix, unsigned size);
    px.pxcGetDeviceAndBHBadPixelMatrix.argtypes = [c_uint, POINTER(c_char), c_uint]
    px.pxcGetDeviceAndBHBadPixelMatrix.restype = c_int

def _showError(px: ctypes.CDLL, prefix="") -> int: # ====================================
    buf = create_string_buffer(4096)
    rc = px.pxcGetLastError(buf, ctypes.sizeof(buf))
    if rc != 0: print("(pxcGetLastError failed", rc, ")")
    else: 
        s = buf.value.decode("utf-8", errors="replace")
        print(f"{prefix}Err: '{s.strip()}'")
    return rc

def _testShowError(name, rc, text=""): # ================================================
    if rc < 0:
        print(f"{name} failed - rc:{rc}", text)
        spacesBeforeCnt = 0
        if name[0]==" ": spacesBeforeCnt = len(name) - len(name.lstrip())
        prefix = ' ' * spacesBeforeCnt
        if name[0]=="\t": prefix="\t"
        _showError(px, prefix)

def loadFrameDataTxt(filePath, width, height):
    data = []
    try:
        with open(filePath, "r") as f:
            for line in f:
                row = [int(x) for x in line.strip().split()]
                data.append(row)
        if len(data) != height or any(len(row) != width for row in data):
            raise ValueError(f"Data dimensions do not match: w:{len(data[0]) if data else -1} h:{len(data)} (expected w:{width} h:{height})")
        return data
    except Exception as e:
        print(f"Error loading file:'{filePath}': {e}")
        return None

def saveFrameDataTxt(filePath, data, width, height): # ==================================
    try:
        with open(filePath, "w") as f:
            for y in range(height):
                for x in range(width):
                    f.write(str(data[y*width + x]) + " ")
                f.write("\n")
            print(f"Saved:'{filePath}'")
    except Exception as e:
        print(f"Error saving file:'{filePath}': {e}")
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

    devPixels = -1
    devIdx = 0
    print("Selected device:", devIdx, "(if it isn't device that was origin of the data, bad pixels correction will not good)\n")
    devWidth = c_uint()
    devHeight = c_uint()
    rc = px.pxcGetDeviceDimensions(devIdx, byref(devWidth), byref(devHeight))
    if rc == 0:
        devPixels = devWidth.value * devHeight.value
        print("Dev dimensions:", devWidth.value, "x", devHeight.value, "pixels:", devPixels)

    else:
        print("pxcGetDeviceDimensions rc:", rc)
        _showError(px)

    wid = devWidth.value
    hei = devHeight.value
    print("Expected data dimmensions:", wid, hei)

    dataPath = os.path.join(thisPath, "../test-data")
    fnamFilesRef = [
        ["BH0mm_1000fr_0.01s_50kV_8mA.txt", 0, 0.01*1000],  # timeIntegrated = singne frame * frames count [seconds]
        ["BH2mm_1000fr_0.01s_50kV_8mA.txt", 2, 0.01*1000], ["BH4mm_1000fr_0.01s_50kV_8mA.txt", 4, 0.01*1000],
        ["BH8mm_1000fr_0.01s_50kV_8mA.txt", 8, 0.01*1000], ["BH16mm_1000fr_0.01s_50kV_8mA.txt", 16, 0.01*1000]
    ]
    # Be careful to reference images and images to correct are match.
    # - Integrated times are true and in same units
    # - Source voltage, current, distance and optional filter is same

    for fileName, thickness, timeIntegrated in fnamFilesRef:
        print(f"    Add file:'{fileName}', tim:{timeIntegrated} s, thc:{thickness} mm")
        data = loadFrameDataTxt(os.path.join(dataPath, fileName), wid, hei)
        if data is None: continue
        flatData = [pixel for row in data for pixel in row]
        dataArray = (c_uint * len(flatData))(*flatData)
        a = sum(flatData)/len(flatData)
        print("    average:", a, "Warning too low counts (>50k recommended)" if a<50000 else "")
        rc = px.pxcAddBHMask(dataArray, len(flatData), timeIntegrated, thickness)
        _testShowError("pxcAddBHMask", rc)
    print("BH masks added:", px.pxcBHMaskCount())
    
    fnamFilesData = [
        ["test_1fr0.0993333s50kev8mA.txt", 0.0993333], ["test_1000fr0.01s50kev8mA.txt", 1000*0.01]
    ]
    badPixelsArr = (c_char * devPixels)()
    
    for fileName, timeIntegrated in fnamFilesData:
        print(f"    File:'{fileName}' time sum:{timeIntegrated} s")
        data = loadFrameDataTxt(os.path.join(dataPath, fileName), wid, hei)
        if data is not None:
            flatData = [pixel for row in data for pixel in row]
            dataArray = (c_uint * len(flatData))(*flatData)
            outDataArray = (ctypes.c_double * len(flatData))()
            a = sum(flatData)/len(flatData)
            print("    average:", a, "Warning too low counts (>50k recommended)" if a<50000 else "")
            rc = px.pxcApplyBHCorrection(dataArray, len(flatData), timeIntegrated, outDataArray)
            _testShowError("pxcApplyBHCorrection", rc)
            # dataArray now containing BH corrected data, but with bad pixels
            rc = px.pxcGetDeviceAndBHBadPixelMatrix(devIdx, badPixelsArr, devPixels)
            _testShowError("    pxcGetDeviceAndBHBadPixelMatrix", rc)
            cnt = sum(1 for i in range(devPixels) if badPixelsArr[i] != b'\0')    
            print("    Bad pixels:", cnt)
            rc = px.pxcInterpolateBadPixels(badPixelsArr, outDataArray, wid, hei)
            # badPixelsArr now containing device bad pixels and pixels that was has data useless for BHC
            _testShowError("    pxcInterpolateBadPixels", rc)
            if rc==-13: # Too many bad pixels - try use only device bad pixels without BH useless pixels
                print("    Too many bad pixels - try use only device bad pixels")
                rc = px.pxcGetDeviceBadPixelMatrix(devIdx, badPixelsArr, devPixels)
                print("    pxcGetDeviceBadPixelMatrix rc:", rc)
                _testShowError("    pxcGetDeviceBadPixelMatrix", rc)
                cnt = sum(1 for i in range(devPixels) if badPixelsArr[i] != b'\0')
                print("    Bad pixels:", cnt)
                rc = px.pxcInterpolateBadPixels(badPixelsArr, outDataArray, wid, hei)
                print("    pxcInterpolateBadPixels rc:", rc)
                _testShowError("    pxcInterpolateBadPixels", rc)
                if rc==-13:
                    print("    Too many bad pixels - Is the device configured properly?")

            opath = os.path.join(dataPath, fileName.replace(".txt", "-BHC.txt"))
            saveFrameDataTxt(opath, outDataArray, wid, hei)
        else:
            print("    (no data readed)")
    
except Exception as e:
    print("Exception:", e)

finally:
    print("pxcore exit...")
    rc2 = px.pxcExit()
    print("pxcExit rc:", rc2, "(0 is OK)")
    if rc2 != 0: _showError(px)
    else:        print("Done")
