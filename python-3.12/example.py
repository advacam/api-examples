#! /usr/bin/env python
# -*- coding: utf-8 -*-
# vim:fenc=utf-8

import sys, os

outPath = "test-files" # Output path for output saving.
apiPath = "../../../API-nightly" # Path to API package or Pixet installed
sys.path.append(apiPath)
os.environ["PATH"] = apiPath + ";" + os.environ["PATH"]
# Alternatively use path to installed Pixet Pro, it cause sharing automatic configurations.
# Or simply copy the script to Pixet or API directory and run it from there.

import pypixet

print("pixet core init...")
rc = pypixet.start()
print("pypixet.start rc:", rc, "(0 is OK)")
if rc!=0: print("Last err:", pypixet.getLastError())

pixet = pypixet.pixet
devices = pixet.devices()
print("Devices found:", len(devices))
for n in range(len(devices)):
    print("  ", n, ":", devices[n].fullName())
dev = devices[0]

rc = dev.setOperationMode(pixet.PX_TPX3_OPM_EVENT_ITOT)
print("setOperationMode", rc, "(0 is OK)")
if rc!=0: print("Last err:", dev.lastError())

print("Warning: Data acquired too early afther init can contanis chip power-on artefacts.")

print("dev.doSimpleAcquisition (5 frames per 1 sec)...")
rc = dev.doSimpleAcquisition(5, 1, pixet. PX_FTYPE_AUTODETECT, "test-files/test.png")
print("dev.doSimpleAcquisition - end:", rc, "(0 is OK)")
if rc!=0: print("Last err:", pypixet.getLastError())

print("pixet core exit...")
rc = pypixet.exit()
print("pypixet.exit rc:", rc, "(0 is OK)")