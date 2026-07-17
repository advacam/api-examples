# pixet/device:
# (c) 2026 Pavel hudecek, Advacam
#
# Example of accessing the subframes, it's data and saving each subframe

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
rc = dev.doSimpleAcquisition(1, 0.2,  pixet.PX_FTYPE_AUTODETECT, "")
if rc==0: print("doSimpleAcquisition() OK")
else: print("doSimpleAcquisition() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")

frame = dev.lastAcqFrameRefInc()
print("frame:", frame)

frameTypes = ["(unknown 0)", "(unknown 1)", "i16", "(unknown 3)", "(unknown 4)", "u32", "(unknown 6)", "u64", "(unknown 8)", "doub"]
print(f"name:'{frame.frameName()}', w*h:{frame.width()}*{frame.height()}, size:{frame.size()}, byteSize:{frame.byteSize()}, dataFormatUID:{frame.dataFormatUID()}, dataFormat:{frame.dataFormat()}, frameType:{frameTypes[frame.frameType()]}")
print("(main frame from devices with subframes normally has no name and data is raw data of all subframes before decoding)")
print()

print("frame.subFrameCount():", frame.subFrameCount(), "--------")
for n, sfr in enumerate(frame.subFrames()):
    print(f"   {n}: name:{sfr.frameName()}, byteSize:{sfr.byteSize()}, dataFormatUID:{sfr.dataFormatUID()}, dataFormat:{sfr.dataFormat()}, frameType:{frameTypes[sfr.frameType()]}")
    sStart = sfr.height()/2 * sfr.width() + sfr.width()/2 - 10
    print(f"   Sample data:", sfr.data()[int(sStart):int(sStart)+20], "...")
    #save(dataMgr: IDataMgr, fileName: str, fileType: int, flags: int = 0) -> int"}
    rc = sfr.save(pixet.dataMgr(), os.path.join(outPath, f"subframe_{n}.txt"), pixet.PX_FTYPE_AUTODETECT, 0)
    print(f"   sfr.save() rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")
 
print("---------------------------------")

print()

print("Exit pixet...")
rc = pypixet.exit()
print("pypixet.exit() rc:", rc, "(0 is OK)")