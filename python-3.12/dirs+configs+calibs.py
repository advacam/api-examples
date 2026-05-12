# pixet/device:
# (c) 2022 Pavel hudecek, Advacam
#
# The dirrectories, configs, using calibration, chips order example
#
# Tested on the MiniPix-Tpx3, MiniPix-Timepix, Widepix-Mpx3

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
print("-----------------------------------------------------------------------------------")

print("Devices list (idx, device name, chips count, [chips list], material:")
for n in range(len(devices)):
    dev = devices[n]
    print("  ", n, ":", dev.fullName(), dev.chipCount(), dev.chipIDs(), dev.sensorType(0))

if devices[0].fullName()=="FileDevice 0":
    print("  No devices connected")
    pixet.exitPixet()
    pypixet.exit()
    exit()
    
print()
print("---------------------------------")
print("Device 0 selected")
print()
dev = devices[0]

print("Operation modes management:")
try:# device with whole chip operation mode (modify for other than Tpx3)
    dev.setOperationMode(pixet.PX_TPX3_OPM_TOA) # PX_TPX3_OPM_TOATOT PX_TPX3_OPM_EVENT_ITOT
    print("   This device have common OPM for all and can use the setOperationMode method")
except: # device with every pixel OPMs (modify for other than Timepix)
    pixcfg = dev.pixCfg() # Create the pixels configuration object 
    pixcfg.setModeAll(pixet.PX_TPXMODE_TOT)
    print("   This device have pixels separately configurable and OPM can be set using the pixCfg object")
print("---------------------------------")
      
print("ApplicationDirectory: Full path to the core library:\n  ", parSet.getValue("ApplicationDirectory"))
print()
print("ApplicationDataDirectory: Full path to general data loading/saving:\n  ", parSet.getValue("ApplicationDataDirectory"))
print()
print("FactoryDataDirectory: Full path to the factory data:\n  ", parSet.getValue("FactoryDataDirectory"))
print("   (This directory is used to load the configuration using the device.loadFactoryConfig() method.")
print("   The installer or user must save the default configuration supplied with the device from the manufacturer here.)")
print("   Note: If the device have internal NVM with default config (Minipix-Tpx3), this dirrectory is not used.")
print()
print("LoggingDirectory: Full path where logs are saved:\n  ", parSet.getValue("LoggingDirectory"))
print("   (If not exist, created automatically at the pixet core start)")
print()
print("ConfigsDirectory: Full path to configs loading/saving:\n  ", parSet.getValue("ConfigsDirectory"))
print("   (When the pixet.exitPixet() method is used, the Pixet core save actual config here.")
print("   In the future, when the pixet.start() used, the Pixet core loads this.")
print("   If not exist, and pixet.exitPixet() method used, the Pixet core create it.)")
print()
print("CreateFileDevice: Create 'File device 0' if not real dev detected:\n  ", parSet.getValue("CreateFileDevice"))
print()
print("SaveSettingsEnabled: Enable auto-saving dev settings at exit, to the ConfigsDirectory:\n  ", parSet.getValue("SaveSettingsEnabled"))
print()
print("---------------------------------")

rc = dev.isConfigInDeviceSupported()
print("isConfigInDeviceSupported():", rc)
print("   Returns 1 if store of configuration data in the device is supported or 0 if not")
print("   Result:")
if rc==1:
    print("   1: This device have internal non-volatile memory to save config using the saveConfigToDevice() method")
    print("   and at the future load this config to the Pixet core using the loadConfigFromDevice() method.")
    print("   Note: Normally there is stored factory config, but user can overwrite it (not recommended).")
else:
    print("   0: This device haven't internal non-volatile memory to load/save config")
    print("   for using with loadConfigFromDevice()/saveConfigToDevice() methods.")
print()

if rc==1: # ConfigInDeviceSupported
    rc = dev.hasConfigInDevice()
    print("hasConfigInDevice()", rc)
    print("   Returns 1 if configuration data are stored in the device or 0 if memory is free")
    print("   Result:")
    if rc==1:
        print("   1: The device have some config stored in the non-volatile memory")
        print("   and this config can be used by the loadConfigFromDevice() method.")
    else:
        print("   0: The device have a free non-volatile memory to saving config for future use.")
    print()

print("defaultConfigFileName(): Returns default full path for device config file")
print("  ", dev.defaultConfigFileName())
print("   (See back to the configsDir() for details.)")
print()
print("---------------------------------")

# If you want to use a different configuration than the one stored in the file
# defined by dev.defaultConfigFileName(), you can use one of the following options:
# 1. Use the loadConfigFromDevice()
rc = dev.loadConfigFromDevice()
print("dev.loadConfigFromDevice()", rc, "(0 is OK)")
if rc==0: # 0 no errors
    print("   The config was suscesfully loaded from the device internal NVM")
    print("   and can be used by the Pixet core.")
else:
    print("   Error: The dev.loadConfigFromDevice() was failed!")
    print("  ", dev.lastError())
print()
# 2. Use the loadFactoryConfig()
rc = dev.loadFactoryConfig()
print("dev.loadFactoryConfig()", rc, "(0 is OK)")
if rc==0: # 0 no errors
    print("   The config was suscesfully loaded from the factory config file or from")
    print("   the device internal NVM and can be used by the Pixet core.")
else:
    print("   Error: The dev.loadFactoryConfig() was failed!")
    print("  ", dev.lastError())
print()
# 3. Use the loadConfigFromFile(path)
rc = dev.loadConfigFromFile("SomePath")
print("dev.loadConfigFromFile(path)", rc, "(0 is OK)")
if rc==0: # 0 no errors
    print("   The config was suscesfully loaded from the file using the")
    print("   loadConfigFromFile(path) method and can be used by the Pixet core.")
else:
    print("   Error: The dev.loadConfigFromFile(path) was failed!")
    print("  ", dev.lastError())
print()
print("---------------------------------")

try:
    rc = dev.useCalibration(1)
    print("useCalibration(value)", rc, "(0 is OK)")
    if rc==0: # 0 no errors
        print("   Calibration enable value was suscesfully set to", dev.isUsingCalibration())
    else:
        print("   Error: The dev.useCalibration(value) was failed!")
        print("  ", dev.lastError())
except:
    print("This device have not the useCalibration(use) method")
    # Mpx3, for example
print()

try:
    rc = dev.isUsingCalibration()
    print("isUsingCalibration(): Returns 1 if calibration of a frame data is enabled or 0 if not.")
    print("   Result:")
    if rc==1:
        print("   1: In frame mode, ToT data will be converted to keV")
    else:
        print("   0: Calibration is disabled. Use dev.useCalibration(1) to enable.")
except:
    print("This device have not the isUsingCalibration() method")
    # Mpx3, for example
print()

try:
    rc = dev.isConvertToaTimeEnabled()
    print("isConvertToaTimeEnabled(): Returns 1 if ToA convert is enabled or 0 if not.")
    print("   Result:")
    if rc==1:
        print("   1: Convert ToA enabled: On devices with combined OPM support (like the Tpx3),")
        print("   in frame mode, in the ""ToA"" named subframe is values in nanosecs.")
        print("   Or on single-mode devices (like the Medipix), frame from the ToA mode contains nanosecs.")
    else:
        print("   0: Convert ToA is disabled: On devices with combined OPM support (like the Tpx3),")
        print("   in frame mode, two separate ToA subframes will be created:")
        print("      ""ToA"" subframe with raw ToA counter values")
        print("      ""FTOA"" subframe with raw fine ToA counter values")
except:
    print("This device have not the isConvertToaTimeEnabled() method.")
    # Timepix, Mpx3, for example (Tpx3 is OK)
print()
print("---------------------------------")    

w = [int(0)]
h = [int(0)]
o = [int(0)] * 4
a = [int(0)] * 4
if dev.chipCount()>1:
    print("dev.chipLayout: The device has more than 1 chip. Chip layout/order information:")
    rc = dev.chipLayout(w, h, o, a)
    print("  dev.chipLayout(w, h, o, a)", rc, "0 is OK")
    print("  Width [chips], Height [chips], Chips order, Chips angles:")
    print("   ", w, h, o, a)
else:
    print("dev.chipLayout: The device has no more than 1 chip.\n  The chip layout/order information is not useful in this case.")
    
print()
print("---------------------------------")     
print("Exit pixet...\n(Terminating all devices and saving their current configurations to pixet.configsDir)")
rc = pypixet.exit()
print("pypixet.exit() rc:", rc, "(0 is OK)")