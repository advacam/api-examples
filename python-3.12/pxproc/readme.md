# The pypxproc object
This object can create data processing objects that can work with clusters from data that comes best from data-driven mode.  
**Clustering(dev)** Get the Clustering object. It can processing a single clusters to looking for interesting single particles and the like.  
**SpectraImaging(dev)** Get the SpectraImg object. It can process many clusters into pixels in images, select clusters by energy range, or sort to energy channels..  

The methods listed above can be used with or without parameter. If not used, only offline processing is possible. 

**Notes**:
* It is **on demand**, not standard part of the API package
* There is **exceptions-based error handling**, in comparsion with other Pixet API parts where an return codes used.  

## The Clustering object
The Clustering object is designed for easy work with clusters of the pixels. Typically used to determine energy and other parameters of high energy particles. 

### The Clustering methods 
---
**loadCalibrationFromDevice()** Load the calibration data from IDev connected to the Cl.  
**loadCalibrationFromFiles(path)** Loads ABCT calibration from single XML file. Device config file can be used.  
**loadCalibrationFromFiles("pathA|pathB|pathC|pathT")** Load ABCT calibration data from set of text files.  
**isCalibrationLoaded()** Returns 1 if calibration is loaded or 0 if not.  

**replayData(pathIn, pathOut, blocking)** Process data from the input file.  
* Input can be tpx3 pixels: t3pa, t3r, t3p.  
* Process the data and calls the corresponding callbacks for further cluster processing.  
* If calibration is loaded, energy values will be calibrated.  
* If the output path defined and ending with .clog, cluster log will be saved.  
* If the blocking is true, program wait to process end, if false, processing is started in a separate thread.  

Example:
```python
cl.replayData("input.t3pa", "output.clog", True)
```

**startMeasurement(acqTime, measTime, pathOut)** Start the measurement.  
Only if the SI connected to the IDev.  
Measurement works in the background. Use while-isRunning() to wait for end, if need it. 
*  **measTime:** Total time [s] of the measurement. Use 0 to endless measurement (progress will always 100%). 
* **acqTime:** Primary for frame-based devices (Medipixes, Timepix, no Tpx3): This is single frame time. 
  Use a short enough time to prevent clusters overlapping. Too short time can cause too many losses 
between frames. On data-driven devices (Timepix3, no Timepix), this is the ToA limit. After exceeds, ToA is resets and acqIndex in the newClusters… callbacks is incremented. 
* **pathOut:** Output file path. For data-driven devices (eq Timepix3) required pixel files: t3pa, t3r, t3p 
  For Frame-based devices (eq Medipix, Timepix) required cluster log files: clog 
* **processData:** True/false, enable/disable online processing.  
Warning: Online processing can cause data loss due to insufficient computing power.  

**isRunning()** Returns 1 if clustering process is running or 0 if not.  

Example: 
```python
print("starting measurement...") 
cl.startMeasurement(1, 100, "") 
while cl.isRunning(): 
    pass 
print("meas. end, rc:", rc, "(0 is OK)") 
```

### The Clustering properties 
---
**messageCallback** Name of the callback function for message receiving (errCode, messageString).  
**progressCallback** Name of the callback function for process progress monitoring (progPercent, finishedNum). Occurs approximately twice per second.  

**acqStartedCallback** Name of the callback function for acquisition started (acqIndex). Can make sense only with frame-based devices.  
**acqFinishedCallback** Name of the callback function for acquisition finished (acqIndex). Can make sense only with frame-based devices.  
**newClustersCallback** Name of the callback function for new clusters parameters using (clusters, acqIndex).  
**newClustersWithPixelsCallback** Name of callback function for new clusters data processing (clusters, acqIndex).  

The callback parameter “clusters” get the **Clusters object**. This can be simply used like us array to get the Cluster object. The Cluster object from the newClustersWithPixelsCallback can be used to get array of the Pixel objects.

### The Clusters object 
---
The Clustering object have the callback functions named newClustersCallback and newClustersWithPixelsCallback. 
His first parameter can be used to get the Clusters object containing a clusters. Normally is used as an array of the Cluster objects. 
**Example:**
```python
def newClustersCb(clusters, acqIndex): 
    for i in range(len(clusters)): 
        cluster = clusters[i]
```

### The Cluster object 
---
The Cluster object contains a single cluster data. Total size, total energy, position, roundness and can include list of his pixels. 
* **id** Order number of the cluster.
* **toa** Time of arrival of first cluster’s pixel.
* **x, y** Position of the cluster. This is not normal integer position of one pixel, it’s computed cluster center.
* **size** Number of pixels in the cluster.
* **height** Pixel size of the cluster.
* **roundness** 1 for an ideal circle with area equal to cluster.size, decreasing by a number related to the ratio of the inscribed circle vs. circumscribed circle.
* **e** Total energy in the cluster. If the calibration loaded, e is energy in keV, if not, e is sum of pixels ToTs.
* **pixels** Array of the Pixel objects. Filled only if the cluster is from the newClustersWithPixelsCallback.

## The Pixel object 
Single pixel data from the cluster which was obtained from the newClustersWithPixelsCallback. 
* **toa** Time of arrival of first cluster’s pixel.
* **x, y** Position of the pixel.
* **e** Energy absorbed in the pixel. If the calibration loaded, e is energy in keV, if not, e is the ToT value.

## The SpectraImg object 
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

# See:
* https://wiki.advacam.cz/wiki/Pixet_SDK#Cluster_processing
* https://wiki.advacam.cz/wiki/Pixet_SDK#Spectral_imaging
