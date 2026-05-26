# Spectral imaging example using pypxproc - Spectral Imaging
# Advacam, https://www.advacam.com/, https://wiki.advacam.cz/wiki/Pixet_SDK,
# Pavel Hudecek 2026

# Notes:
# - pypxproc is not standard part of API package, it is on demand.
# - Device methods returning return codes, but the pypxproc methods raise exceptions.
# - The acqTime is irelevant on data-driven devices like Tpx3,
# but on frame-only devs like as Tpx/Tpx2 it must be small to prevent clusters overlap, if too small, measurement time efficiency rapidly falls.

import sys, os, traceback, time

outPath = "test-files" # Output path for saving.
apiPath = "C:/Advacam/API-nightly" # Path to API package or Pixet installed
sys.path.append(apiPath)
os.environ["PATH"] = apiPath + ";" + os.environ["PATH"]
# Alternatively use path to installed Pixet Pro, it cause sharing automatic configurations.
# Or simply copy the script to Pixet or API directory and run it from there.

measTime = 40    # Integrated measurement time [s]
meas1replay0 = 1 # 1 - do measurement / 0 - measure to t3pa file and replay data from it

mpFrom = 50     # Starting energy of first bin [keV]
mpTo = 4500     # Ending energy of last bin [keV]
mpStep = 50     # Energy bin step [keV]
mpMaskNP = True # Mask noisy pixels
mpDoSPC = False # Do subpixel correction

import pypixet	# Requires pxcore.dll/so, this requires hwlib(s) of device(s),
                # the hwlib can reguire other file(s).
import pypxproc # Require pxproc.dll/so, this can be used with/without pxcore.

#MessageCallback(error: int, message: str)
def messageCb(error, msg): # ======================================================================
    print(f"** ErrCode: {error}, Message: {msg}")

#ProgressCallback(finished: bool, progress: float)
def progressCb(finished, progress): # =============================================================
    print(f"** Progress: {progress:.2f} %, finished: {finished}", end="\t")
    print(f"Measured {si.measuredPixelsPerSecond():.2e} p/s, Processed {si.processedPixelsPerSecond():.2e} p/s")

def errTest(rc):
    if rc!=0: print("err:", dev.lastError())

def testFrame(frame, pref, wid, hei): # simple logaritmic test view ===============================
    print(f"{pref}------------------------------------------------------------------")
    xdiv = wid//64
    ydiv = hei//32

    for y in range(32):
        print(f"{pref}|", end="")
        for x in range(64):
            val=0
            for yy in range(ydiv):
                for xx in range(xdiv):
                    val += frame[x*xdiv + xx][y*ydiv + yy]
            
            if val>100000:  print("#", end="")
            elif val>10000: print("O", end="")
            elif val>1000:  print("o", end="")
            elif val>100:   print("*", end="")
            elif val>10:    print("+", end="")
            elif val>0:     print(".", end="")
            else:           print(" ", end="")
        print("|")
    print(f"{pref}------------------------------------------------------------------")
# def testFrame()

print("pixet core init...") # ---------------------------------------------------------------------
rc = pypixet.start()
print("pypixet.start rc:", rc, "(0 is OK)")
if rc!=0: print("Last err:", pypixet.getLastError())
pixet=pypixet.pixet
#devices = pixet.devicesByType(pixet.PX_DEVTYPE_TPX3)
devices = pixet.devices()
if len(devices)==0:
    print("No devices detected")
    pypixet.exit()
    exit()

print("Devices:")
chipTypes = {
    pixet.PX_CHIPTYPE_TPX3:"Tpx3", pixet.PX_CHIPTYPE_TPX2:"Tpx2", pixet.PX_CHIPTYPE_TPX:"Tpx",
    pixet.PX_CHIPTYPE_TPX4:"Tpx4", pixet.PX_CHIPTYPE_MPX3:"Mpx3"
}
sel = -1 # selected device index
for n in range(len(devices)):
    dev = devices[n]
    cht = dev.chipType()
    print(f"   {n}: '{dev.fullName()}', type:{chipTypes.get(cht, '(Unknown)')}")
    if sel==-1 and cht in [pixet.PX_CHIPTYPE_TPX3, pixet.PX_CHIPTYPE_TPX4, pixet.PX_CHIPTYPE_TPX]: sel = n

if sel==-1:
    print("No usable device")
    pypixet.exit()
    exit()
dev = devices[sel]
print(f"Selected device {sel}: '{dev.fullName()}' w:{dev.width()} h:{dev.height()}")
print("----------------------------------------------------------------------")

print("hasDefaultConfig", dev.hasDefaultConfig(), "(true = not configured)")

print("loadFactoryConfig...")
rc = dev.loadFactoryConfig()
print("loadFactoryConfig rc:", rc, "(0 is OK)")
errTest(rc)
if rc!=0:
    print("loadFactoryConfig failed: Check the factory directory and exact filename\nSee: https://wiki.advacam.cz/wiki/Files_and_directories_of_the_Pixet_and_SDK#factory")

print("hasDefaultConfig", dev.hasDefaultConfig())

t3paPath = os.path.join(outPath, "test.t3pa")
clogPath = os.path.join(outPath, "test.clog")
bstgPath = os.path.join(outPath, "test.bstg")

if meas1replay0==0:
    print(f"delete file '{t3paPath}' if exists...")
    try:
        if os.path.exists(t3paPath): os.remove(t3paPath)
        print("end")
    except:
        print(f"delete file '{t3paPath}' failed")
        traceback.print_exc()

    rc = dev.setOperationMode(pixet.PX_TPX3_OPM_TOATOT)
    print("setOperationMode rc:", rc, "(0 is OK)")
    errTest(rc)

    print(f"doAdvancedAcquisition({measTime}, '{t3paPath}')...")
    # doAdvancedAcquisition(acqTime: float, measTime: float, acqType: int, acqMode: int, fileType: int, flags: int, outputFilePath: str)
    rc = dev.doAdvancedAcquisition(
        1, measTime, pixet.PX_ACQTYPE_DATADRIVEN, pixet.PX_ACQMODE_NORMAL,
        pixet.PX_FTYPE_AUTODETECT, 0, t3paPath
    )
    print("doAdvancedAcquisition end:", rc, "(0 is OK)")
    errTest(rc)

si = pypxproc.SpectraImaging(dev.asIDev())
print("SpectraImaging created")

si.messageCallback = messageCb
si.progressCallback = progressCb

#loadCalibrationFromFiles("cals/I08-W0060-cal_a.txt|cals/I08-W0060-cal_b.txt|cals/I08-W0060-cal_c.txt|cals/I08-W0060-cal_t.txt")
#loadCalibrationFromFiles("cals/Minipix-I08-W0060.xml")
si.loadCalibrationFromDevice()
print("isCalibrationLoaded", si.isCalibrationLoaded())

#setMeasParams(spectFrom: int, spectTo: int, spectStep: float, maskNoisyPixels: bool, doSubPixCorrection: bool)
#setMeasParams(from, to, step, maskNP, doSPC)
si.setMeasParams(mpFrom, mpTo, mpStep, mpMaskNP, mpDoSPC)
print("setMeasParams done")

if meas1replay0==1:
    acqTime = 1 # single frame time (frame-only devices), no Tpx3
    if measTime>500: acqTime = 10
    outFile = clogPath
    processData=True
    print(f"startMeasurement({acqTime}, {measTime}, '{clogPath}', {processData})...")
    #startMeasurement(acqTime: float, measTime: float, outputFilePath: str, processData: bool)
    si.startMeasurement(acqTime, measTime, outFile, processData)
    print("meas started")
    #print(si.abort())
else:
    print(f"replayData('{t3paPath}', '{clogPath}', False)...")
    #replayData(filePath: str, outputFilePath: str, blocking: bool = False)
    si.replayData(t3paPath, clogPath, False)
    print("replay started")

t = 0
while si.isRunning():
    print(f"t:{t}")
    time.sleep(1)
    t += 1
print("end")

print(f"saveToFile({bstgPath})...")
si.saveToFile(bstgPath)
print(f"loadFromFile({bstgPath})...")
si.loadFromFile(bstgPath)
print("Loaded")

print("save output Data:")
fn = os.path.join(outPath, "saveDataAsSpectrumToFile.txt")
print(f"    saveDataAsSpectrumToFile({fn})...")
si.saveDataAsSpectrumToFile(fn)
fn = os.path.join(outPath, "saveDataAsSpectrumToFile.csv")
print(f"    saveDataAsSpectrumToFile({fn})...")
si.saveDataAsSpectrumToFile(fn)
fn = os.path.join(outPath, "saveDataAsFramesToFile.pmf")
print(f"    saveDataAsFramesToFile({fn})...")
si.saveDataAsFramesToFile(fn, 1) # oneFile=True
fn = os.path.join(outPath, "saveSumFrame.pmf")
print(f"    saveSumFrame({fn})...")
si.saveSumFrame(fn, 1, 0)        # filePath, zoom, correction
print("end")

sumFrame = False
normalize = False
zoom = 1
for enIndex in range(1, 10):
    e = mpFrom + enIndex*mpStep
    # getFrameForEnergy(energyIndex: int, sumFrame: bool, normalize: bool, zoom: int) -> [[float]]
    frame = si.getFrameForEnergy(enIndex, sumFrame, normalize, zoom)
    print(f"getFrameForEnergy idx:{enIndex}, e:{e} keV")
    testFrame(frame, "  ", dev.width(), dev.height())   

enFrom = 1
enTo = 10
e1 = mpFrom + enFrom*mpStep
e2 = mpFrom + enTo*mpStep
# getFrameForEnergyRange(energyFromIndex: int, energyToIndex: int, normalize: bool)
frame = si.getFrameForEnergyRange(enFrom, enTo, normalize)
print(f"getFrameForEnergyRange idx:{enFrom}-{enTo} e:{e1}-{e2} keV")
testFrame(frame, "  ", dev.width(), dev.height())

# getGlobalSpectrum() -> Tuple[[int], float]
spect, step = si.getGlobalSpectrum()
print(f"getGlobalSpectrum len={len(spect)}, step={step}")
print(spect)

# getGlobalSpectrumInRect(left: int, top: int, right: int, bottom: int)
spect, step = si.getGlobalSpectrumInRect(10, 10, 100, 100)
print(f"getGlobalSpectrumInRect len={len(spect)}, step={step}")
print(spect)

print("exit...")
rc = pypixet.exit()
print("pypixet.exit rc:", rc, "(0 is OK)")
errTest(rc)