# Examples of using binary/C API
Tested on Windows<br>
This dir is root of the **Visual Studio 2026** solution.<br>
* First extract the API package to the PATH_TO_API (examples\cpp-Windows-MSVS+Cmake\x64\Debug)
* Can by used dirrectly by "Play" button
* Can be compiled with Cmake [Binary core API: Building using cmake](https://wiki.advacam.cz/wiki/Binary_core_API#Building_using_cmake_on_Windows_with_Visual_Studio_installed)

## First example: "example"
* Maintaining API package location
* CMAKE example as comment in CPP file.
* Error handling
* Commandline program:
  * Init the Pixet core (optional chdir to API)
  * Load factory config
  * Set operation mode (Timepix3 - other need change)
  * Simple measurment to files
  * Exit Pixet core

## example-callbacks / example-callbacks2
* Error handling
* Using callbacks: regular functions / lambda functions
* Commandline program:
  * Init the Pixet core (optional chdir to API)
  * Load factory config
  * Set operation mode (Timepix3 - other need change)
  * Frame measurement with callbacks for data processing
  * Dada-driven measurement (Timepix3 only) with callbacks for data processing
  * Exit Pixet core

## example-callbacks-faster
* Error handling
* Commandline program:
  * Init the Pixet core (optional chdir to API)
  * Load factory config
  * Set operation mode (Timepix3 - other need change)
  * Change min. data-driven callback repeat time (Timepix3 only)
  * Dada-driven measurement (Timepix3 only) with callbacks for data processing
  * Exit Pixet core

## example-calib
* Some devices can measure energy deposited in each pixel and software can convert measured "time over threshold - ToT" to energy in keV.
* Calibrated frame output can be enabled by enabling calibration before measurement.
* Calibrated frames are accesible:
  * In saved frame files
  * Using "calibrated" variants of frame-get functions
  * Using "calibrated" variants of single-frame-measure functions
* Calibrated data-driven pixels are accesible:
  * Applying calibration function to data-driven measured pixels

# Examples of using binary/C processing API
(required pixel processing library - not in standard package - on demand)

## example-clustering-online / example-clustering-offline
**Clustering:**
* Convert stream of timestamped pixels or frames to stream of clusters

### Online
* Init the Pixet core with searching physical devices
* Connect device to the processing engine
* Load calibration from device
* Measuring with processing pixels to clusters
* Filter, preview and save clusters
* Deinitialize

### Offline
* Load the Pixet core without fhysical devices
* Connect Pixet core to the processing engine
* Load calibration from file(s)
* Processing pixels from input file to clusters
* Filter, preview and save clusters
* Deinitialize

## example-spectralimaging-online / example-spectralimaging-offline
**Spectral imaging:**
* Convert stream of timestamped pixels or frames to stream of clusters
* Compensate XRF effects in sensor chip
* Sort clusters to bins by energy
* Get spectrum of frame or rectangle
* Create frames with selected energies/ranges
* Create a summary image that has a higher resolution than the sensor chip due to the subpixel positions of the clusters.
* Save and load processed data with it's spectral settings to be used in future without repeating processing time.

### Online
* Init the Pixet core with searching physical devices
* Connect device to the processing engine
* Load calibration from device
* Set up spectral settings
* Variants:
  * Measuring with processing pixels to clusters and sorting it to bins
  * Processing pixels from input file to clusters and sorting it to bins
  * Load previous saved BSTG and get it's spectral settings
* Saving processed data to BSTG file
* Get global spectrum and spectrum from rectangle
* Save spectral frames
* Get and preview frames for some single bins
* Get and preview frames for some ranges
* Deinitialize

### Offline
* Load the Pixet core without fhysical devices
* Connect Pixet core to the processing engine
* Load calibration from file(s)
* Set up spectral settings
* Variants:
  * Processing pixels from input file to clusters and sorting it to bins
  * Load previous saved BSTG and get it's spectral settings
* Saving processed data to BSTG file
* Get global spectrum and spectrum from rectangle
* Save spectral frames
* Get and preview frames for some bins
* Get and preview frames for some ranges
* Deinitialize


# Related wiki:
* [Pixet SDK: Binary (C) core API](https://wiki.advacam.cz/wiki/Binary_core_API)
* [Pixet SDK: Binary (C) clustering API](https://wiki.advacam.cz/wiki/Binary_Clustering_API)
* [Pixet SDK: Binary (C) spectral imaging API](https://wiki.advacam.cz/wiki/Binary_Spectral_Imaging_API)
* [Notes for Pixet/API and UI/script differences](https://wiki.advacam.cz/wiki/Pixet_SDK#Notes_for_Pixet-API_and_UI/script_differences)

* [Files and directories of the Pixet and SDK: Main directory of the API-using programs](https://wiki.advacam.cz/wiki/Files_and_directories_of_the_Pixet_and_SDK#Main_directory_of_the_API-using_programs,_independent_on_the_Pixet)
