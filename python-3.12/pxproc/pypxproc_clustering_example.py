# Clustering example using pypxproc - Clustering
# Advacam, https://www.advacam.com/, https://wiki.advacam.cz/wiki/Pixet_SDK,
# Pavel Hudecek 2026

# Notes:
# - pypxproc is not standard part of API package, it is on demand.
# - Device methods returning return codes, but the pypxproc methods raise exceptions.
# - The acqTime is irelevant on data-driven devices like Tpx3/Tpx4, only start/finish callbacks depending on it,
# but on frame-only devs like as Tpx/Tpx2 it must be small to prevent clusters overlap, if too small, measurement time efficiency rapidly falls.

import sys, os, traceback, time

outPath = "test-files" # Output path for saving.
apiPath = "C:/Advacam/API-nightly" # Path to API package or Pixet installed
sys.path.append(apiPath)
os.environ["PATH"] = apiPath + ";" + os.environ["PATH"]
# Alternatively use path to installed Pixet Pro, it cause sharing automatic configurations.
# Or simply copy the script to Pixet or API directory and run it from there.

measTime = 40
meas1replay0 = 1
minSize = 10

import pypixet	# Requires pxcore.dll/so, this requires hwlib(s) of device(s),
                # the hwlib can reguire other file(s).
import pypxproc

def errTest(rc): # ================================================================================
    if rc!=0: print("err:", dev.lastError())

#MessageCallback(error: int, message: str)
def messageCb(error, msg): # ======================================================================
    print(f"*** ErrCode: {error}, msg: {msg}")

#ProgressCallback(finished: bool, progress: float)
def progressCb(finished, progress): # =============================================================
    print(f"*** Progress: {progress:.2f} %, finished={finished}")

#AcqFinishedCallback(acqIndex: int)
def acqFinishedCb(acqIndex): # ====================================================================
    print(f"*** AcqFinished: acqIndex={acqIndex}")

#AcqStartedCallback(acqIndex: int)
def acqStartedCb(acqIndex): # =====================================================================
    print(f"*** AcqStarted: acqIndex={acqIndex}")

#NewClustersCallback(clusters: [Cluster], acqIndex: int) - clusters has empty pixels
def newClustersCb(clusters, acqIndex): # ==========================================================
    try:
        print(f"*** NewClusters: count:{len(clusters)},  acqIndex:{acqIndex}")
        ignored = 0
        shown = 0
        for ci in range(len(clusters)):
            cluster = clusters[ci]
            if cluster.size < minSize:
                ignored += 1
                continue
            print(f"  Cluster {ci}: xy:{int(cluster.x)},{int(cluster.y)}, e:{cluster.e:.2f}, toa:{cluster.toa:.2e}, size:{cluster.size}, pxLen:{len(cluster.pixels)}, height:{cluster.height:.2f}, rou:{cluster.roundness:.2f}, id:{cluster.id}")
            shown += 1
            if shown > 50:
                print(f"  (and more clusters)")
                break
        print(f"  Clusters ignored:{ignored}, shown:{shown}, more not tested:{len(clusters)-ignored-shown}")
    except:
        traceback.print_exc()

#NewClustersWithPixelsCallback(clusters: [Cluster], acqIndex: int) - clusters has pixels with full data
def newClustersWithPixelsCb(clusters, acqIndex): # ================================================
    try:
        print(f"*** NewClustersWithPixels: count:{len(clusters)},  acqIndex:{acqIndex}")
        ignored = 0
        shown = 0
        for ci in range(len(clusters)):
            cluster = clusters[ci]
            if cluster.size < minSize:
                ignored += 1
                continue
            print(f"  Cluster {ci}: xy:{int(cluster.x)},{int(cluster.y)}, toa:{cluster.toa:.2e}, e:{cluster.e:.2f}, size:{cluster.size}, pxLen:{len(cluster.pixels)}, height:{cluster.height:.2f}, rou:{cluster.roundness:.2f}, id:{cluster.id}")
            pxs = 0
            for pi in range(len(cluster.pixels)):
                pix = cluster.pixels[pi]
                print(f"      [{int(pix.x):<3},{int(pix.y):<3}, t:{pix.toa:.2f}, e:{pix.e:.2f}]")
                pxs += 1
                if pxs > 30:
                    print(f"      (and more pixels)")
                    break
            shown += 1
            if shown > 50:
                print(f"  (and more clusters)")
                break
        print(f"  Clusters ignored:{ignored}, shown:{shown}, more not tested:{len(clusters)-ignored-shown}")
    except:
        traceback.print_exc()

print("pixet core init...") # ---------------------------------------------------------------------
pypixet.start()
pixet=pypixet.pixet
devices = pixet.devicesByType(pixet.PX_DEVTYPE_TPX3)
if len(devices)==0: devices = pixet.devicesByType(pixet.PX_DEVTYPE_TPX2)
if len(devices)==0: devices = pixet.devicesByType(pixet.PX_DEVTYPE_TPX)

#devices = pixet.devices()
if len(devices)==0:
    print("No Tpx3/Tpx2/Tpx device")
    pypixet.exit()
    exit()

dev = devices[0]
print(f"Selected device: '{dev.fullName()}' w:{dev.width()} h:{dev.height()}")
print("----------------------------------------------------------------------")

print("hasDefaultConfig  ", dev.hasDefaultConfig())

print("loadFactoryConfig...")
rc = dev.loadFactoryConfig()
print("loadFactoryConfig rc:", rc, "(0 is OK)")
errTest(rc)
if rc!=0:
    print("loadFactoryConfig failed: Check the factory directory and exact filename\nSee: https://wiki.advacam.cz/wiki/Files_and_directories_of_the_Pixet_and_SDK#factory")

print("hasDefaultConfig  ", dev.hasDefaultConfig())

acqTime = 1 # single frame time (frame-only devices), no Tpx3
if measTime>500: acqTime = 10
t3paPath = os.path.join(outPath, "test.t3pa")
clogPath = os.path.join(outPath, "test.clog")

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
        1, measTime, pixet.PX_ACQTYPE_DATADRIVEN, pixet.PX_ACQMODE_NORMAL, pixet.PX_FTYPE_AUTODETECT, 0, t3paPath
    )
    print("doAdvancedAcquisition end:", rc)
    errTest(rc)

cl = pypxproc.Clustering(dev.asIDev())
cl.messageCallback = messageCb
cl.progressCallback = progressCb
cl.acqStartedCallback = acqStartedCb
cl.acqFinishedCallback = acqFinishedCb
cl.newClustersCallback = newClustersCb
cl.newClustersWithPixelsCallback = newClustersWithPixelsCb

#loadCalibrationFromFiles("cals/I08-W0060-cal_a.txt|cals/I08-W0060-cal_b.txt|cals/I08-W0060-cal_c.txt|cals/I08-W0060-cal_t.txt")
#loadCalibrationFromFiles("cals/Minipix-I08-W0060.xml")
cl.loadCalibrationFromDevice()
print("Load calib")

if meas1replay0==1:
    print(f"startMeasurement({acqTime}, {measTime}, '{clogPath}')...")
    # startMeasurement(acqTime: float, measTime: float, outputFilePath: str)
    cl.startMeasurement(acqTime, measTime, clogPath)
    print("meas started")
else:
    # replayData(filePath: str, outputFilePath: str, blocking: bool)
    print(f"replayData('{t3paPath}', '{clogPath}', False)...")
    cl.replayData(t3paPath, clogPath, False)
    print("replay started")

t = 0
while cl.isRunning():
    print(f"t:{t}")
    time.sleep(1)
    t += 1
    if t > measTime*2 + measTime/acqTime*2: # safety exit, should not be needed
        print("Time limit reached, aborting...")
        cl.abort()
        print("aborted")
        break

print("end")

print("exit...")
rc = pypixet.exit()
print("pypixet.exit rc:", rc, "(0 is OK)")