# Examples of using binary/C API
Tested on Windows<br>
This dir is root of the **Visual Studio 2026** solution.<br>
* Can by used dirrectly by "Play" button
* Can be compiled with Cmake [Binary core API: Building using cmake](https://wiki.advacam.cz/wiki/Binary_core_API#Building_using_cmake_on_Windows_with_Visual_Studio_installed)

## First example: "example":
* Contains all need files except Your lic.info and (factory) config of Your device
* Maintaining API package location
* CMAKE example as comment in CPP file.
* Error handling
* Commandline program:
  * Init the Pixet core (optional chdir to API and back)
  * Load factory config
  * Set operation mode (Timepix3 only)
  * Simple measurment to files
  * Exit Pixet core (optional chdir to API)

# Related wiki:
* [Pixet SDK: Binary (C) API](https://wiki.advacam.cz/wiki/Pixet_SDK#Binary_(C/C++)_APIs)
* [Files and directories of the Pixet and SDK: Main directory of the API-using programs](https://wiki.advacam.cz/wiki/Files_and_directories_of_the_Pixet_and_SDK#Main_directory_of_the_API-using_programs,_independent_on_the_Pixet)
