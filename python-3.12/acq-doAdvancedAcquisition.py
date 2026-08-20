# pixet/device:
# (c) 2026 Pavel hudecek, Advacam
#
# Example of basic using of
# - doAdvancedAcquisition() - measure frames to memory (and file(s)) or measure pixels to memory (and file)
# - doAdvancedIntegralAcquisition() - integrate frames to one and store it in memory (and file)
#
# doAdvancedAcquisition(count, time, acqType, acqMode, fileType, fileFlags, fileName)
# doAdvancedIntegralAcquisition(count, time, acqType, acqMode, fileType, fileFlags, fileName)
#
# acqType Acquisition type (frames / test pulses / data-driven) 
#   Examples: pixet.PX_ACQTYPE_FRAMES, PX_ACQTYPE_DATADRIVEN
# acqMode Acquisition mode (normal / triggered / …)
#   Warning: This is not operation mode. Use the dev.setOperationMode method. 
#   Examples: pixet.PX_ACQMODE_NORMAL, PX_ACQMODE_TRG_HWSTART, …
# fileType - To prevent mistakes, allways use pixet.PX_FTYPE_AUTODETECT if not very special conditions
# fileFlags - Flags - For sparse files (pixet.PX_FRAMESAVE_SPARSEX / SPARSEXY)
#   Override to using binary data - PX_FRAMESAVE_BINARY (apply to .pmf for example)
#   Aux files and separating subframes - PX_FRAMESAVE_NODSC, NOSUBFRAMES, SUBFRAMES_ONEFILE, SUBFRAMES_SAVEMAINFRAME
#   Flags can be ORed if not colliding

import sys, os

outPath = "test-files" # Output path for output saving.
apiPath = "..\\cpp-Windows-MSVS+Cmake\\x64\\Debug" # Path to API package or Pixet installed
sys.path.append(apiPath)
os.environ["PATH"] = apiPath + ";" + os.environ["PATH"]
# Alternatively use path to installed Pixet Pro, it cause sharing automatic configurations.
# Or simply copy the script to Pixet or API directory and run it from there.

import pypixet

print("pixet core init...")
pypixet.start()
pixet=pypixet.pixet
devices = pixet.devices()

chipTypes = ["(unknown 0)", "MXR", "TPX", "MPX3", "TPX3", "TPX2", "MPX4", "TPX4"]
print("Devices list (idx, device name, [chips list], chip type, material):")
for n in range(len(devices)):
    dev = devices[n]
    print("  ", n, ":", dev.fullName(), dev.chipIDs(), chipTypes[dev.chipType()], dev.sensorType(0))

if len(devices)==0 or devices[0].fullName()=="FileDevice 0":
    print("  No devices connected")
    print("Exit pixet...")
    pypixet.exit()
    exit()
    
print("---------------------------------")
print("Device 0 selected")
print()
dev = devices[0]

rc = dev.loadFactoryConfig()
print("dev.loadFactoryConfig() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")

try:# device with whole chip operation mode (modify for other than Tpx3)
    rc = dev.setOperationMode(pixet.PX_TPX3_OPM_TOATOT) # PX_TPX3_OPM_TOA PX_TPX3_OPM_EVENT_ITOT
    print("setOperationMode rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
except: # device with every pixel OPMs (modify for other than Timepix)
    pixcfg = dev.pixCfg() # Create the pixels configuration object 
    rc = pixcfg.setModeAll(pixet.PX_TPXMODE_TOT)
    print("pixcfg.setModeAll() rc:", rc, "(0 is OK)")

print("----------------- Frame acquisition examples --------------------")

print("doAdvancedAcquisition...")
#doAdvancedAcquisition(count, time, acqType, acqMode, fileType, fileFlags, fileName)
rc = dev.doAdvancedAcquisition(
    5, 0.2, pixet.PX_ACQTYPE_FRAMES, pixet.PX_ACQMODE_NORMAL,
    pixet.PX_FTYPE_AUTODETECT, 0, ""
)
print("doAdvancedAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
if rc==0:
    print("dev.acqFrameCount():", dev.acqFrameCount())
    print("Now are frames in memory, can be used later. Frames are stored to next acquisition start")
    print("See the subframes example to access it.")
    print("For fully online processing can be registered the pixet.PX_EVENT_ACQ_FINISHED callback")

print()
print("doAdvancedAcquisition...")
rc = dev.doAdvancedAcquisition(
    5, 0.2, pixet.PX_ACQTYPE_FRAMES, pixet.PX_ACQMODE_NORMAL, pixet.PX_FTYPE_AUTODETECT,
    pixet.PX_FRAMESAVE_SPARSEX, os.path.join(outPath, "doAdvancedAcquisition.txt")
)
print("doAdvancedAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
if rc==0:
    print("Frames were saved to files with 'sparse-pxIndex' saving settings, see File saving flags for more options.")
    print("See https://wiki.advacam.cz/wiki/File_types#File_saving_flags_summary for more informations.")
    print("(Frames also remain in memory)")
    print("To manage saved frames metadata (dsc file), use callback registered by the dev.registerBeforeSaveDataEvent.")

print()
print("doAdvancedIntegralAcquisition...")
#doAdvancedIntegralAcquisition(count, time, acqMode, fileType, fileFlags, fileName)
rc = dev.doAdvancedIntegralAcquisition(
    5, 0.2, pixet.PX_ACQTYPE_FRAMES, pixet.PX_ACQMODE_NORMAL, pixet.PX_FTYPE_AUTODETECT,
    pixet.PX_FRAMESAVE_SPARSEX, os.path.join(outPath, "doAdvancedIntegralAcquisition.txt")
)
print("doAdvancedIntegralAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
if rc==0:
    print("All frames were integrated and saved to one file.")
    print("(Integrated frame also remain in memory)")

print()
print("----------------- Data-driven (pixels) acquisition examples --------------------")

if dev.chipType() not in [pixet.PX_CHIPTYPE_TPX3, pixet.PX_CHIPTYPE_TPX4]:
    print("Data-driven acquisition is not supported for this readout chip type (tpx3/tpx4 only)")
    print("Exit pixet...")
    pypixet.exit()
    exit()

print("Note: Data-driven acquisition has sense only with ToA+ToT operation modes and acqCount=1")
print("doAdvancedAcquisition...")
#doAdvancedAcquisition(count=1, time, acqType, acqMode, fileType, fileFlags, fileName)
rc = dev.doAdvancedAcquisition(
    1, 2.5, pixet.PX_ACQTYPE_DATADRIVEN, pixet.PX_ACQMODE_NORMAL,
    pixet.PX_FTYPE_AUTODETECT, 0, ""
)
print("doAdvancedAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
if rc==0:
    pixels = dev.lastAcqPixelsRefInc()
    pxCntTot = pixels.totalPixelCount()
    print("pxCntTot:", pxCntTot, "(no pixels measured now)" if pxCntTot==0 else "")
    print("Pixels were stored in memory, can be used later. Pixels are stored to next acquisition start")
    print("See the pixels example to access it.")
    print("For fully online processing can be registered the pixet.PX_EVENT_ACQ_NEW_DATA callback")

print()
print("doAdvancedAcquisition...")
rc = dev.doAdvancedAcquisition(
    1, 2.5, pixet.PX_ACQTYPE_DATADRIVEN, pixet.PX_ACQMODE_NORMAL, pixet.PX_FTYPE_AUTODETECT,
    0, os.path.join(outPath, "doAdvancedAcquisition-datadriven.t3pa")  # t3pa / t3p / (t3r in very special cases)
)
print("doAdvancedAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
if rc==0:
    print("Pixels were saved to file with default saving settings, see File saving flags for more options.")
    print("See https://wiki.advacam.cz/wiki/File_types#Timepix3_specific_data_files for more informations.")
    print("See https://wiki.advacam.cz/wiki/Binary_core_API#Data-driven_special_settings for more data-driven settings.")
    print("(Data-driven saving cannot be combined with online processing from memory)")

print()
print("---------------------------------")
print("Exit pixet...")
rc = pypixet.exit()
print("pypixet.exit() rc:", rc, "(0 is OK)")