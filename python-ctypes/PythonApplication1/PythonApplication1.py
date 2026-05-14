import os, sys

apiPath = "../../../API-nightly"
sys.path.append(apiPath)
os.environ["PATH"] = apiPath + ";" + os.environ["PATH"]

print("pixet core init...") 
import pypixet
rc = pypixet.start() 
print("pypixet.start rc:", rc, "(0 is OK)") 
if rc!=0: print("Last err:", pypixet.getLastError()) 
pixet=pypixet.pixet 
devices = pixet.devices() 
print("Devices found:", len(devices))
for n in range(len(devices)):
    print("  ", n, ":", devices[n].fullName())
dev = devices[0]

# Warning: Data acquired too early afther init can contanis chip power-on artefacts.
print("doSimpleAcquiisition...")
rc = dev.doSimpleAcquisition(5, 1, pixet.PX_FTYPE_AUTODETECT, "test-files/test.png")
print("doSimpleAcquiisition rc:", rc, "(0 is OK)")
if rc!=0: print("Last err:", dev.lastError())

print("pixet core exit...") 
rc = pypixet.exit() 
print("pypixet.exit rc:", rc, "(0 is OK)") 
if rc!=0: print("Last err:", pypixet.getLastError()) 
else:     print("Done")