# The SpectraImg object 
The SpectraImg is designed for easy working with an energy spectras. It can work with 
previous saved data using the **replayData** method or with physical device. For offline mode 
create the SpectraImg using **pypxproc.SpectraImaging()**. For online mode, the device must be 
normally initialized by the Pixet object, convert to IDev using device’s **asIDev()** method and 
connected using pypxproc.SpectraImaging(idev). Pixels in the generated frames have values 
from clusters detected in the input data.
**Warning:** Online processing can cause data loss due to insufficient computing power. 

Example (online): 
```python
import pypixet, pypxproc 
print("pixet core init...") 
pypixet.start() 
pixet=pypixet.pixet 
devices = pixet.devicesByType(pixet.PX_DEVTYPE_TPX3) 
dev = devices[0] 
si = pypxproc.SpectraImaging(dev.asIDev()) 
```

Example (offline): 
```python
import pypxproc 
si = pypxproc.SpectraImaging()
```

**Steps for using this object in the online mode:**
1. Initialize the Pixet core and create the device object (skip core init if starting from the Pixet program). 
2. Optionally use the sensor refresh or dummy acq. 
3. Create the SpectraImg object using IDev object converted from the device. 
4. Set-up the callbacks. 
5. Load device calibration (loadCalibrationFromDevice or loadCalibrationFromFiles). 
6. Set measurement parameters using the setMeasParams and possibly setXrfParams methods. 
7. Start the measurement using startMeasurement. 
8. Wait for measurement and processing is complete (and display the progress) using while-isRunning. 
9. Use some get/save… method and use the data. 
10. Deinitialize the Pixet core.

**Steps for using this object in the offline mode:**
1. Create the Spectraimg object using empty parentheses. 
2. Set-up the callbacks. 
3. Load device calibration using loadCalibrationFromFiles. 
4. Set measurement parameters using the setMeasParams and possibly setXrfParams methods. 
5. Use the replayData instead of a measurement and process data from the t3pa file. 
6. Wait for processing complete (and display the progress) using while-isRunning. 
(If goal is only get the clog file, this is end) 
7. Use some get/save… method and use the data. 

**Using BSTG files to save processing time:**
1. After processing is complete (end of waiting steps above), use the saveToFile("file.bstg") method. 
2. Anytime use the loadFromFile("file.bstg") method and continue using the data as it was processed. 

**See:**
* https://wiki.advacam.cz/wiki/Pixet_SDK#Cluster_processing
* https://wiki.advacam.cz/wiki/Pixet_SDK#Spectral_imaging
