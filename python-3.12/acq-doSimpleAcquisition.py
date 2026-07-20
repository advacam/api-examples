# pixet/device:
# (c) 2026 Pavel hudecek, Advacam
#
# Example of basic using of
# - doSimpleAcquisition() - measure frames to memory (and file(s))
# - doSimpleIntegralAcquisition() - integrate frames to one and store it in memory (and file)
#
# doSimpleAcquisition(count, time, fileType, fileName)
# doSimpleIntegralAcquisition(count, time, fileType, fileName)

import sys, os

outPath = "test-files" # Output path for output saving.
apiPath = "../../../API-nightly" # Path to API package or Pixet installed
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

print("doSimpleAcquisition...")
#doSimpleAcquisition(count, time, fileType, fileName)
rc = dev.doSimpleAcquisition(5, 0.2,  pixet.PX_FTYPE_AUTODETECT, "")
print("doSimpleAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
if rc==0:
    print("dev.acqFrameCount():", dev.acqFrameCount())
    print("Now are frames in memory, can be used later. Frames are stored to next acquisition start")
    print("See the subframes example to access it.")
    print("For fully online processing can be registered the pixet.PX_EVENT_ACQ_FINISHED callback")

print()
print("doSimpleAcquisition...")
rc = dev.doSimpleAcquisition(5, 0.2,  pixet.PX_FTYPE_AUTODETECT, os.path.join(outPath, "doSimpleAcquisition.txt"))
print("doSimpleAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
if rc==0:
    print("Frames were saved to files with default saving settings, see the doAdvancedAcquisition example for more options.")
    print("See https://wiki.advacam.cz/wiki/File_types for more information about the file types.")
    print("(Frames also remain in memory)")

print()
print("doSimpleIntegralAcquisition...")
#doSimpleIntegralAcquisition(count, time, fileType, fileName)
rc = dev.doSimpleIntegralAcquisition(5, 0.2,  pixet.PX_FTYPE_AUTODETECT, os.path.join(outPath, "doSimpleIntegralAcquisition.txt"))
print("doSimpleIntegralAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
if rc==0:
    print("All frames were integrated and saved to one file.\nSee the doAdvancedAcquisition example for more options in doAdvancedIntegralAcquisition.")
    print("(Integrated frame also remain in memory)")

print()
print("---------------------------------")
print("Exit pixet...")
rc = pypixet.exit()
print("pypixet.exit() rc:", rc, "(0 is OK)")