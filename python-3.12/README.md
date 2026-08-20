# Examples of using Python API 1.8.4/5/6 in Python 3.12.x
Commandline examples designed for using from system commandline like as

**python example.py**

Need set the **apiPath**
* Default in in **examples\\cpp-Windows-MSVS+Cmake\\x64\\Debug** to be one for Py and C examples. Extract the package there.
* or set to directory with extracted Pixet API package 
* or set to directory of installed Pixet.

## example.py
* Maintaining API package location
* Initialization
* Error handling
* Measuring frames to files
* Commandline program:
  * Init the Pixet core
  * Show devices list
  * Load factory config
  * Set operation mode
  * Measurement some frames to files
  * deinit Pixet core

## acq-doSimpleAcquisition.py
* Maintaining API package location
* Initialization
* Error handling
* Measuring frames and integrated frames
* Commandline program:
  * Init the Pixet core
  * Show devices list with it's properties
  * Load factory config
  * Set operation mode
  * Measuring to memory
  * Measuring to files
  * Measuring integral frames
  * deinit Pixet core

## acq-doAdvancedAcquisition.py
* Maintaining API package location
* Initialization
* Error handling
* Measuring to frames and pixels streams (data-driven)
* File saving flags can be used
* Commandline program:
  * Init the Pixet core
  * Show devices list with it's properties
  * Load factory config
  * Set operation mode
  * Measuring to frames
  * Measuring to integral frames
  * Measuring to pixels stream and remain it in memory
  * Measuring to pixels stream with saving to file
  * deinit Pixet core

## frame-subframes.py
Access to subframes example<br>
Thit is extension for doSimpleAcquisition and doSimpleAcquisition examples.

## frame-copy.py
Copying of frame<br>
Can be used in "chery-picking" from high framerate.

## callbacks-examples.py
Understanding of Pixet core callbacks
* Register all possible callbacks
* Do measurements and other situations generating callbacks
* Ungegister registered callbacks

## dirs+configs+calibs.py
Understanding settings/parameters:
* Directories used by Pixet core
* Device congiguration in files and device
* Calibration
* ToA conversion
* Chip layout
