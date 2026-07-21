# pixet/device: using the callbacks:
# (c) 2026 Pavel hudecek, Advacam
#
# This example uses unnecessarily many callbacks,
# but sometimes some may come in handy :-)
#
# Tests list:
#   doSimpleAcquisition
#   doAdvancedAcquisition - frames/normal
#   doAdvancedAcquisition - frames/sw trig.
#   doAdvancedAcquisition - data-driven
#   doContinuousAcquisition (read warning and uncomment to test)
#
# Tested on MiniPix Tpx3 CdTe

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


def clbDevACQ_MEAS_STARTED(ev): # (ev.data) is meas. count. In this example is 0 (no finished frames).
    print("clb *** ACQ_MEAS_STARTED", ev.data)

def clbDevACQ_FINISHED(ev): # (ev.data) is number of finished measurements.
    print("clb *** ACQ_FINISHED", ev.data)
    print("  Frame", dev.lastAcqFrameRefInc())

def clbDevACQ_NEW_DATA(ev): # new data, (ev.data) is meas. index. In this example is always 0.
    print("clb *** ACQ_NEW_DATA", ev.data)
    print("  Pixels count:", dev.lastAcqPixelsRefInc().totalPixelCount())

def clbDevACQ_FAILED(ev): # Acq failed (ev.data) is error code form the Pixet core.
    print("clb *** ACQ_FAILED", ev.data)
    print("  ", dev.lastError())
    
def callbackDEV_STATUS_CHANGED(ev): # Device status changed
    print("clb *** DEV_STATUS_CHANGED", ev.data) 

def clbDevACQ_SERIE_STARTED(ev):
    print ("clb *** ACQ_SERIE_STARTED", ev.data)
    
def clbDevACQ_SERIE_FINISHED(ev):
    print ("clb *** ACQ_SERIE_FINISHED", ev.data)
    
def clbDevACQ_MEAS_FINISHED(ev):
    print ("clb *** ACQ_MEAS_FINISHED", ev.data)
    
def clbDevACQ_ABORTING(ev):
    print ("clb *** ACQ_ABORTING", ev.data)
    
def clbDevACQ_ABORTED(ev):
    print ("clb *** ACQ_ABORTED", ev.data)
    
def clbDevACQ_SWTRG_READY(ev): # ready and waiting for software trigger
    print ("clb *** ACQ_SWTRG_READY", ev.data)
    # dev.doSoftwareTrigger(parameter reserved for future use)
    print ("  doSoftwareTrigger (0=OK)", dev.doSoftwareTrigger(0))
    
def clbDevPX_EVENT_TPX3STG_CHANGED(ev): # settings of Timepix3 was changed (OPM etc)
    print ("clb *** PX_EVENT_TPX3STG_CHANGED", ev.data)
    
def clbDevPX_EVENT_LOCK_CHANGED(ev):
    print ("clb *** PX_EVENT_LOCK_CHANGED", ev.data)  
    
def clbDevPX_EVENT_PIXCFG_CHANGED(ev):
    print ("clb *** PX_EVENT_PIXCFG_CHANGED", ev.data)

def clbDevPX_DATAEVENT_BEFORE_SAVE(ev):
    print ("clb *** PX_DATAEVENT_BEFORE_SAVE", ev.data)
    
def clbDevPX_EVENT_DEV_CFG_CHANGED(ev):
    print ("clb *** PX_EVENT_DEV_CFG_CHANGED", ev.data)
    
def clbDevPX_EVENT_CFG_LOADING_PROGRESS(ev):
    print ("clb *** PX_EVENT_CFG_LOADING_PROGRESS", ev.data)
    
def clbPxPX_EVENT_NEW_DEVICE(ev): # (ev.data) is device handle (32b integer)
    print ("clb *** PX_EVENT_NEW_DEVICE", ev.data)
    
def clbPxPX_EVENT_DEVICE_REMOVED(ev):
    print ("clb *** PX_EVENT_DEVICE_REMOVED", ev.data)
    
def clbPxPX_EVENT_EXIT(ev):
    print ("clb *** PX_EVENT_EXIT", ev.data)

def clbDevPX_EVENT_BIAS_CHANGED(ev):
    print ("clb *** PX_EVENT_BIAS_CHANGED", ev.data)
    print("  Bias:", ev.obj.bias())

def clbDevPX_DATAEVENT_BEFORE_SAVE(ev): # before every file saving 
    print ("clb *** PX_DATAEVENT_BEFORE_SAVE", ev.data)

print("pixet core init...")
pypixet.start()
pixet=pypixet.pixet

def regHandler(regarr, obj, name, callbackFn): # ----------------------------------------
    rc = obj.registerEvent(name, callbackFn)
    if rc>=0:
        regarr.append(rc)
        print("Registered:", name, "fn:", callbackFn.__name__, "rc:", rc, "(>=0 is OK)")
    else:
        print("Name:", name, "fn:", callbackFn.__name__, "rc:", rc)
        if hasattr(obj, "lastError"):
            print("  Last err:", obj.lastError())
        else:
            print("  Last err: (no msg available)")

regPix = []
print("register Pixet Events...")
# registerEvent(name, (reserved), CallFunc)
regHandler(regPix, pixet, pixet.PX_EVENT_NEW_DEVICE, clbPxPX_EVENT_NEW_DEVICE)
regHandler(regPix, pixet, pixet.PX_EVENT_DEVICE_REMOVED, clbPxPX_EVENT_DEVICE_REMOVED)
regHandler(regPix, pixet, pixet.PX_EVENT_EXIT, clbPxPX_EVENT_EXIT)
#regPix.append(pixet.registerEvent(pixet., clbPx))

input("Insert new device and press enter")
pixet.refreshDevices()
input("Press enter")

print("TPX3 devices:")
devices = pixet.devicesByType(pixet.PX_DEVTYPE_TPX3)
    
devCnt = len(devices)
if devCnt==0:
    print("  No devices connected")
    pixet.exitPixet()
    exit()

for n in range(devCnt):
    dev = devices[n]
    print("  ", n, dev.fullName(), dev.width(), dev.height(), dev.chipCount(), dev.chipIDs(), dev.sensorType(0))

print("Selected device index: 0")
pars = dev.parameters()
print("CPU Temp:", pars.get("TemperatureCpu").getDouble(), end='')
print(", Chip Temp:", pars.get("TemperatureChip").getDouble())
print("FirmwareCpu:", pars.get("FirmwareCpu").getString())
print("==================================================================")
# get first Timepix3 device:
dev = devices[0]

regDev = []
print("register device Events...")
# registerEvent(name, CallFunc)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_MEAS_STARTED, clbDevACQ_MEAS_STARTED)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_FINISHED, clbDevACQ_FINISHED)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_NEW_DATA, clbDevACQ_NEW_DATA)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_FAILED, clbDevACQ_FAILED)
regHandler(regDev, dev, pixet.PX_EVENT_DEV_STATUS_CHANGED, callbackDEV_STATUS_CHANGED)

#dev.registerEvent(pixet.PX_EVENT_, callback)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_SERIE_STARTED , clbDevACQ_SERIE_STARTED)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_SERIE_FINISHED, clbDevACQ_SERIE_FINISHED)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_MEAS_FINISHED, clbDevACQ_MEAS_FINISHED)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_ABORTING, clbDevACQ_ABORTING)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_ABORTED, clbDevACQ_ABORTED)
regHandler(regDev, dev, pixet.PX_EVENT_ACQ_SWTRG_READY, clbDevACQ_SWTRG_READY)

regHandler(regDev, dev, pixet.PX_EVENT_TPX3STG_CHANGED, clbDevPX_EVENT_TPX3STG_CHANGED)
regHandler(regDev, dev, pixet.PX_EVENT_LOCK_CHANGED, clbDevPX_EVENT_LOCK_CHANGED)
regHandler(regDev, dev, pixet.PX_EVENT_PIXCFG_CHANGED, clbDevPX_EVENT_PIXCFG_CHANGED)
regHandler(regDev, dev, pixet.PX_EVENT_DEV_CFG_CHANGED, clbDevPX_EVENT_DEV_CFG_CHANGED)
regHandler(regDev, dev, pixet.PX_EVENT_CFG_LOADING_PROGRESS, clbDevPX_EVENT_CFG_LOADING_PROGRESS)
regHandler(regDev, dev, pixet.PX_EVENT_BIAS_CHANGED, clbDevPX_EVENT_BIAS_CHANGED)

#dev.registerBeforeSaveDataEvent(CallFunc)
bsderc = dev.registerBeforeSaveDataEvent(clbDevPX_DATAEVENT_BEFORE_SAVE)

print("==================================================================")
print()

#opm = pixet.PX_TPX3_OPM_TOATOT
#opm = pixet.PX_TPX3_OPM_EVENT_ITOT # good for imaging / not usable for data-driven
opm = pixet.PX_TPX3_OPM_TOA
#opm = pixet.PX_TPX3_OPM_TOT_NOTOA
print("mode set:", opm)
# set Timepix3 operation mode: (src_gen\ipixetw.cpp)
dev.setOperationMode(opm)

print("doSensorRefresh start")        
rc = dev.doSensorRefresh()
print("doSensorRefresh rc:", rc, "(0 is OK)" if rc==0 else f"errMsg:'{dev.lastError()}'")

print("==================================================================")
print()

testTime = 2

print("doSimpleAcquisition - save file - start")
#doSimpleAcquisition(count, time, fileType, fileName)
rc = dev.doSimpleAcquisition(3, testTime, pixet.PX_FTYPE_AUTODETECT, os.path.join(outPath, "SimpleSave.txt"))
print("dev.doSimpleAcquisition end", rc)

print()
print("==================================================================")
print()

print("doAdvancedAcquisition - frames/normal - start")
#doAdvancedAcquisition(count, time, type, mode, fileType, fileFlags, filePath)
rc = dev.doAdvancedAcquisition(5, testTime, pixet.PX_ACQTYPE_FRAMES, pixet.PX_ACQMODE_NORMAL, pixet.PX_FTYPE_NONE, 0, "")
print("doAdvancedAcquisition end", rc)

print()
print("==================================================================")
print()

print("doAdvancedAcquisition - frames/sw trig. - start")
#doAdvancedAcquisition(count, time, type, mode, fileType, fileFlags, filePath)
rc = dev.doAdvancedAcquisition(5, testTime, pixet.PX_ACQTYPE_FRAMES, pixet.PX_ACQMODE_TRG_SWSTART, pixet.PX_FTYPE_NONE, 0, "")
print("doAdvancedAcquisition end", rc)

print()
print("==================================================================")
print()

print("doAdvancedAcquisition - data-driven - start")
#doAdvancedAcquisition(count, time, type, mode, fileType, fileFlags, filePath)
# in data-driven mode count must be >0, value is ignored
rc = dev.doAdvancedAcquisition(1, testTime, pixet.PX_ACQTYPE_DATADRIVEN, pixet.PX_ACQMODE_NORMAL, pixet.PX_FTYPE_NONE, 0, "")
print("doAdvancedAcquisition end", rc)
print("Note: If no callbacks 'ACQ_NEW_DATA' occurred, must use longer time or particle source") 

"""
print()
print("==================================================================")
print()

print("doContinuousAcquisition start")
#dev.doContinuousAcquisition(BuffCount, time, mode)
rc = dev.doContinuousAcquisition(5, testTime, pixet.PX_ACQMODE_CONTINUOUS)
print("doContinuousAcquisition end", rc)

input("Press Enter to quit doContinuousAcquisition")

print("\nWarning: continuous acquisition cannot be correctly stopped!")
print("You may need to disconnect and reconnect the device before new use.\n")

exit()
"""
print()
print("==================================================================")
print()

print("unregisterEvent pixet...")
for reg in regPix:
    rc = pixet.unregisterEvent(reg)
    print("unregisterEvent", reg, "rc:", rc, "(0 is OK)")
    if rc!=0:
        print("  Last err:", pixet.getLastError())

print("unregisterEvent dev...")
for reg in regDev:
    rc = dev.unregisterEvent(reg)
    print("unregisterEvent", reg, "rc:", rc, "(0 is OK)")
    if rc!=0:
        print("  Last err:", dev.lastError() if hasattr(dev, "lastError") else pixet.getLastError())

rc = dev.unregisterBeforeSaveDataEvent(bsderc)
if rc!=0:
    print("unregisterBeforeSaveDataEvent rc:", rc)
    print("  Last err:", dev.lastError() if hasattr(dev, "lastError") else pixet.getLastError())

print("pixet core exit...")


print()
print("---------------------------------")
print("Exit pixet...")
rc = pypixet.exit()
print("pypixet.exit() rc:", rc, "(0 is OK)")
