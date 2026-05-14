/**
 * Copyright (C) 2020 ADVACAM
 * @author   Daniel Turecek <daniel.turecek@advacam.com>
 */
#ifndef PIXCAPI_H
#define PIXCAPI_H
#include <cstdio>
#include <cstdlib>
#include <inttypes.h>
#include "common.h"

#define PXCERR_NOT_INITIALIZED      -1
#define PXCERR_INVALID_DEVICE_INDEX -2
#define PXCERR_INVALID_ARGUMENT     -3
#define PXCERR_COULD_NOT_SAVE       -4
#define PXCERR_ACQ_FAILED           -5
#define PXCERR_DEVICE_ERROR         -6
#define PXCERR_ACQ_ABORTED          -7
#define PXCERR_CANNOT_RECONNECT     -8
#define PXCERR_NOT_ALLOWED          -9
#define PXCERR_NOT_SUPPORTED        -10
#define PXCERR_BUFFER_SMALL         -11
#define PXCERR_CANNOT_CALIBRATE     -12
#define PXCERR_TOO_MANY_BAD_PIXELS  -13
#define PXCERR_ZEST_NOT_LOADED      -14
#define PXCERR_EVENT_REGISTER       -15
#define PXCERR_EVENT_UNREGISTER     -16
#define PXCERR_UNEXPECTED_ERROR     -1000

#define PXC_ACQEVENT_MEAS_STARTED    "AcqMeasStarted"
#define PXC_ACQEVENT_ACQ_FINISHED    "AcqFinished"
#define PXC_ACQEVENT_ACQ_FAILED      "AcqFailed"
#define PXC_ACQEVENT_MEAS_FINISHED   "AcqMeasFinished"
#define PXC_ACQEVENT_TPX3_DATA       "AcqNewData"

#define PXC_TRG_NO                   0x00
#define PXC_TRG_HWSTART              0x01
#define PXC_TRG_HWSTOP               0x02
#define PXC_TRG_HWSTARTSTOP         (0x01|0x02)
#define PXC_TRG_SWSTART              0x04

#define PXC_TPX_MODE_MEDIPIX        0
#define PXC_TPX_MODE_TOT            1
#define PXC_TPX_MODE_TIMEPIX        3

#define PXC_MPX3_OPM_SPM_1CH        0
#define PXC_MPX3_OPM_SPM_2CH        1
#define PXC_MPX3_OPM_CSM            2
#define PXC_MPX3_OPM_SPM_1CH_COLOR  3
#define PXC_MPX3_OPM_SPM_2CH_COLOR  4
#define PXC_MPX3_OPM_CSM_COLOR      5
#define PXC_MPX3_OPM_CUSTOM         10

#define PXC_MPX3_GAIN_MOD_NARROW    1
#define PXC_MPX3_GAIN_MOD_BROAD     3

#define PXC_MPX3_GAIN_SUPERHIGH     0
#define PXC_MPX3_GAIN_HIGH          1
#define PXC_MPX3_GAIN_LOW           2
#define PXC_MPX3_GAIN_SUPERLOW      3
#define PXC_MPX3_CNT_LOW            0
#define PXC_MPX3_CNT_HIGH           1
#define PXC_MPX3_CNT_BOTH           2
#define PXC_MPX3_CNTD_1B            0
#define PXC_MPX3_CNTD_6B            1
#define PXC_MPX3_CNTD_12B           2
#define PXC_MPX3_CNTD_24B           3
#define PXC_MPX3_COLB_ALL           0
#define PXC_MPX3_ROWB_ALL           0

#define PXC_TPX2_OPM_TOT10_TOA18      0
#define PXC_TPX2_OPM_TOT14_TOA14      1
#define PXC_TPX2_OPM_CONT_TOT10_CNT4  2
#define PXC_TPX2_OPM_CONT_TOT14       3
#define PXC_TPX2_OPM_CONT_TOA10       4
#define PXC_TPX2_OPM_CONT_TOA14       5
#define PXC_TPX2_OPM_CONT_CNT10       6
#define PXC_TPX2_OPM_CONT_CNT14       7
#define PXC_TPX2_OPM_ITOT10_TOA18     8
#define PXC_TPX2_OPM_ITOT14_TOA14     9
#define PXC_TPX2_OPM_CONT_ITOT10_CNT4 10
#define PXC_TPX2_OPM_CONT_ITOT14      11

#define PXC_TPX3_OPM_TOATOT         0
#define PXC_TPX3_OPM_TOA            1
#define PXC_TPX3_OPM_EVENT_ITOT     2
#define PXC_TPX3_OPM_TOT_NOTOA      3

#define PXC_PIXEL_MASKED            0
#define PXC_PIXEL_UNMASKED          1

#define PXC_HVSRC_POLARITY_NEG      0
#define PXC_HVSRC_POLARITY_POS      1
#define PXC_HVSRC_POLARITY_ERR      -1



#ifndef WIN32
    #define PXCAPI extern "C" __attribute__ ((visibility("default")))
#else
    #define PXCAPI extern "C" __declspec(dllexport)
#endif

// order of DACs for Timepix
typedef enum _PXC_DACS_TPX
{
    PXC_TPX_IKRUM, PXC_TPX_DISC, PXC_TPX_PREAMP, PXC_TPX_BUFFA, PXC_TPX_BUFFB, PXC_TPX_HIST, PXC_TPX_THLFINE,
    PXC_TPX_THLCOARSE, PXC_TPX_VCAS, PXC_TPX_FBK, PXC_TPX_GND, PXC_TPX_THS, PXC_TPX_BIASLVDS, PXC_TPX_REFLVDS
} PXC_DACS_TPX;

// order of DACs for Medipix3
typedef enum _PX_DACS_MPX3
{
    PXC_MPX3_TH0, PXC_MPX3_TH1, PXC_MPX3_TH2, PXC_MPX3_TH3, PXC_MPX3_TH4, PXC_MPX3_TH5, PXC_MPX3_TH6, PXC_MPX3_TH7, PXC_MPX3_PREAMP,
    PXC_MPX3_IKRUM, PXC_MPX3_SHAPER, PXC_MPX3_DISC, PXC_MPX3_DISC_LS, PXC_MPX3_SHAPER_TEST, PXC_MPX3_DAC_DISC_L, PXC_MPX3_DAC_TEST,
    PXC_MPX3_DAC_DISC_H, PXC_MPX3_DELAY, PXC_MPX3_TP_BUFF_IN, PXC_MPX3_TP_BUFF_OUT, PXC_MPX3_RPZ, PXC_MPX3_GND, PXC_MPX3_TP_REF, PXC_MPX3_FBK,
    PXC_MPX3_CAS, PXC_MPX3_TP_REFA, PXC_MPX3_TP_REFB,
} PXC_DACS_MPX3;

// order of DACs for Timepix3
typedef enum _PXC_DACS_TPX3
{
    PXC_TPX3_IBIAS_PREAMP_ON, PXC_TPX3_IBIAS_PREAMP_OFF, PXC_TPX3_VPREAMP_NCAS, PXC_TPX3_IBIAS_IKRUM,
    PXC_TPX3_VFBK, PXC_TPX3_VTHRESHOLD, PXC_TPX3_IBIAS_DISCS1_ON, PXC_TPX3_IBIAS_DISC1_OFF,
    PXC_TPX3_IBIAS_DISCS2_ON, PXC_TPX3_IBIAS_DISCS2_OFF, PXC_TPX3_IBIAS_PIXELDAC, PXC_TPX3_IBIAS_TPBUFF_IN,
    PXC_TPX3_IBIAS_TPBUFF_OUT, PXC_TPX3_VTP_COARSE, PXC_TPX3_VTP_FINE, PXC_TPX3_IBIAS_CP_PLL, PXC_TPX3_PLL_VCNTRL
} PXC_DACS_TPX3;

// order of DACs for Timepix2
typedef enum _PXC_DACS_TPX2
{
    PXC_TPX2_VBIAS_PREAMP_ON, PXC_TPX2_VBIAS_PREAMP_OFF, PXC_TPX2_VBIAS_LS_ON, PXC_TPX2_VBIAS_LS_OFF,
    PXC_TPX2_VCASC_PREAMP, PXC_TPX2_VFBK, PXC_TPX2_VTH_COARSE, PXC_TPX2_VTH_FINE, PXC_TPX2_VBIAS_IKRUM,
    PXC_TPX2_VBIAS_DISCPMOS, PXC_TPX2_VBIAS_DISCNMOS, PXC_TPX2_VCASC_DISC, PXC_TPX2_VBIAS_THS,
    PXC_TPX2_VGND, PXC_TPX2_VTP_COARSE, PXC_TPX2_VTP_FINE, PXC_TPX2_VBIAS_SLVS, PXC_TPX2_VCM_SLVS,
    PXC_TPX2_VBIAS_RES
} PXC_DACS_TPX2;



// Pixel Matrix Configurations
#pragma pack(push, 1)
typedef struct _TPXPixCfg {
    // 0 = masked, 0 = test bit ON, timepix mode = 0 - medipix, 1-TOT, 2-1-hit, 3-Timepix
    unsigned char maskBit: 1; unsigned char testBit: 1; unsigned char thl: 4; unsigned char mode: 2;
} TPXPixCfg;

typedef struct _MPX3PixCfg {
    unsigned char maskBit: 1; unsigned char testBit: 1; unsigned char dummy1: 1; unsigned char thl: 5; unsigned char thh:  5; unsigned char dummy2: 3;
} MPX3PixCfg;

typedef struct _TPX3PixCfg {
    unsigned char maskBit: 1; unsigned char thl: 4; unsigned char testBit: 1; unsigned char dummy: 2;
} TPX3PixCfg;

typedef struct _TPX2PixCfg {
    unsigned char maskBit: 1; unsigned char testBit: 1; unsigned char thl: 5; unsigned char dummy: 1;
} TPX2PixCfg;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct _RawTpx3Pixel
{
    u32 index:      24;
    u64 toa:        64;
    u8 overflow:   1;
    u8 ftoa:       5;
    u16 tot:        10;
} RawTpx3Pixel;
#pragma pack(pop)

typedef struct _Tpx3Pixel
{
    double toa;
    float tot;
    unsigned int index;
} Tpx3Pixel;

typedef void (*FrameMeasuredCallback)(intptr_t acqCount, intptr_t userData);
typedef void (*AcqEventFunc)(intptr_t eventData, intptr_t userData);

extern "C"
{
// Sets the configuration files directory and directory for log files
// Has to be set before calling the pxcInitialize function
PXCAPI int pxcSetDirectories(const char* configsDir, const char* logsDir);

// Initializes the Pixet and all connected devices.
// Must be called as first function before all the other functions.
PXCAPI int pxcInitialize(const char *iniFile="pixet.ini");

// Deinitializes all the devices and Pixet. Must be called at end.
PXCAPI int pxcExit();

// Returns the version of Pixet software (e.g. 1.7.3.934)
PXCAPI int pxcGetVersion(char* versionBuffer, unsigned size);

// Returns number of connected and initialized devices.
// Devices can be than accessed by its deviceIndex (starting from 0 for the first device)
PXCAPI int pxcGetDevicesCount();

// Refresh devices list - disconnected devices are removed, Pixet search for newly connected devices
PXCAPI int pxcRefreshDevices();

// Try to reconnected a device
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcReconnectDevice(unsigned deviceIndex);

// Returns to the supplied buffer (nameBuffer) full name of the device under the index deviceIndex
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] nameBuffer  - buffer where name of the device will be copied
// [out] size        - size of the supplied buffer
PXCAPI int pxcGetDeviceName(unsigned deviceIndex, char* nameBuffer, unsigned size);

// Returns number of chips in the device
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcGetDeviceChipCount(unsigned deviceIndex);

// Returns to the supplied buffer (chipIDBuffer) ID of the Medipix/Timepix chip
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  chipIndex   - index of the chip (indexing starting from 0)
// [out] nameBuffer  - buffer where chipID of the device will be copied
// [in] size         - size of the supplied buffer
PXCAPI int pxcGetDeviceChipID(unsigned deviceIndex, unsigned chipIndex, char* chipIDBuffer, unsigned size);

// Returns to device serial number
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcGetDeviceSerial(unsigned deviceIndex);

// Gets the dimension of the device in pixels
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] width - pointer to unsigned variable where the width of the device will be returned
// [out] height - pointer to unsigned variable where the height of the device will be returned
PXCAPI int pxcGetDeviceDimensions(unsigned deviceIndex, unsigned* width, unsigned* height);

// Gets the high voltage (bias voltage) of the sensor on Medipix/Timepix chip
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] bias - pointer to double variable where current bias will be returned
PXCAPI int pxcGetBias(unsigned deviceIndex, double* bias);

// Gets the range of allowed minimal and maximal bias values
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] minBias - pointer to double variable where the minimal allowed bias value will be returned
// [out] maxBias - pointer to double variable where the maximal allowed bias value will be returned
PXCAPI int pxcGetBiasRange(unsigned deviceIndex, double* minBias, double* maxBias);

// Sets the high voltage (bias voltage) of the sensor on Medipix/Timepix chip
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  bias - value of bias voltage in volts
PXCAPI int pxcSetBias(unsigned deviceIndex, double bias);

// Gets the threshold of Medipix/Timepix device.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  thresholdIndex - index of the device threshold to get. Timepix and Timepix3 have only one threshold. Medipix3 can have up to 7.
// [out] threshold - pointer to double variable where threshold will be returned
PXCAPI int pxcGetThreshold(unsigned deviceIndex, int thresholdIndex, double* threshold);

// Gets the range of allowed values for threshold of Medipix/Timepix device.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  thresholdIndex - index of the device threshold to get. Timepix and Timepix3 have only one threshold. Medipix3 can have up to 7.
// [out] minThreshold - pointer to double variable where the minimal allowed threshold will be returned
// [out] maxThreshold - pointer to double variable where the maximal allowed threshold will be returned
PXCAPI int pxcGetThresholdRange(unsigned deviceIndex, int thresholdIndex, double* minThreshold, double* maxThreshold);

// Sets the threshold of Medipix/Timepix device in keV
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  thresholdIndex - index of the device threshold to set. Timepix and Timepix3 have only one threshold. Medipix3 can have up to 7.
// [in]  threshold - new threshold value in keV
PXCAPI int pxcSetThreshold(unsigned deviceIndex, int thresholdIndex, double threshold);

// Gets the DAC parameter of the chip
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  chipIndex - index of chip (indexing starting from 0)
// [in]  dacIndex - index of the dac (PXC_TPX_... values)
// [out] value - DAC value
PXCAPI int pxcGetDAC(unsigned deviceIndex, unsigned chipIndex, unsigned dacIndex, unsigned short* value);

// Sets the DAC parameter of the chip
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  chipIndex - index of chip (indexing starting from 0)
// [in]  dacIndex - index of the dac (PXC_TPX_... values)
// [in]  value - new DAC value
PXCAPI int pxcSetDAC(unsigned deviceIndex, unsigned chipIndex, unsigned dacIndex, unsigned short value);

// Gets the current value of measurement clock for Timepix detector (in MHz)
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] clock - pointer to double variable where clock will be returned
PXCAPI int pxcGetTimepixClock(unsigned deviceIndex, double* clock);

// Sets the measurement clock for Timepix detector (in MHz)
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  clock - new clock value
PXCAPI int pxcSetTimepixClock(unsigned deviceIndex, double clock);

// Gets the current values of measurement clock and ToA divider for Timepix2 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  totClock - pointer to double variable where toaClock will be returned
// [in]  toaClock - pointer to double variable where totClock will be returned
// [in]  divider - pointer to unsifned int variable where ToA divider will be returned
PXCAPI int pxcGetTimepix2Clock(unsigned deviceIndex, double* totClock, double* toaClock, unsigned* divider);

// Sets the measurement clock and ToA divider for Timepix2 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  clock - new clock value (in MHz)
// [in]  divider - ToA divider
PXCAPI int pxcSetTimepix2Clock(unsigned deviceIndex, double clock, unsigned divider);

// Sets the pixel matrix timepix mode of Timepix detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  timepix mode - PXC_TPX_MODE_XX values
PXCAPI int pxcSetTimepixMode(unsigned deviceIndex, int mode);

// Gets the pixel matrix timepix mode of Timepix detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcGetTimepixMode(unsigned deviceIndex);

// Enables/disables the calibration of Timepix2 ToT counts to energy in keV
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  enabled - if calibration is enabled
PXCAPI int pxcSetTimepix2CalibrationEnabled(unsigned deviceIndex, bool enabled);

// Gets if the calibration of Timepix2 ToT counts to energy in keV is enabled (enabled > 0, disabled = 0, error < 0)
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcIsTimepix2CalibrationEnabled(unsigned deviceIndex);

// Enables/disables the calibration of Timepix ToT counts to energy in keV
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  enabled - if calibration is enabled
PXCAPI int pxcSetTimepixCalibrationEnabled(unsigned deviceIndex, bool enabled);

// Gets if the calibration of Timepix ToT counts to energy in keV is enabled (enabled > 0, disabled = 0, error < 0)
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcIsTimepixCalibrationEnabled(unsigned deviceIndex);

// Loads device configuration from xml file
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  filePath - path to xml configuration file
PXCAPI int pxcLoadDeviceConfiguration(unsigned deviceIndex, const char* filePath);

// Saves device configuration to xml file
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  filePath - path to xml configuration file
PXCAPI int pxcSaveDeviceConfiguration(unsigned deviceIndex, const char* filePath);

// Loads device configuration from factory xml file or internal memory
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcLoadFactoryConfig(unsigned deviceIndex);

// Enables/Disables and setups parameters of the test pulse measurements
// [in] deviceIndex - index of the device (indexing starting from 0)
// [in] tpEnabled - enables/disables test pulses measurement (when measuring frames)
// [in] height - test pulse height (0 - 1.5 V)
// [in] period - single test pulse period (1 - 256 us)
// [in] count - test pulses count (1 - 10000)
// [in] spacing - spacing that is used during measurement
PXCAPI int pxcSetupTestPulseMeasurement(unsigned deviceIndex, bool tpEnabled, double height, double period, unsigned count, unsigned spacing);

// Performs a measurement of single frame and returns its data. Only for Medipix2 and Timepix detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameTime - time of the measurement in seconds
// [out] frameData - pointer to buffer where data will be saved. For single detector size should be 65536. Can be NULL, then no data returned, just measured.
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable. Can be NULL, then no data returned, just measured.
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
PXCAPI int pxcMeasureSingleFrame(unsigned deviceIndex, double frameTime, unsigned short* frameData, unsigned* size, unsigned trgStg = PXC_TRG_NO);

// Performs a measurement of single frame and returns its data. This is only for Medipix3 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameTime - time of the measurement in seconds
// [out] frameData1 - pointer to buffer where data from first counter will be saved. For single detector size should be 65536.
// [out] frameData2 - pointer to buffer where data from second counter will be saved. For single detector size should be 65536.
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
PXCAPI int pxcMeasureSingleFrameMpx3(unsigned deviceIndex, double frameTime, unsigned* frameData1, unsigned* frameData2, unsigned* size, unsigned trgStg = PXC_TRG_NO);

// Performs a measurement of single frame and returns its data. This is only for Timepix2 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameTime - time of the measurement in seconds
// [out] frameData1 - pointer to buffer where data from ToA/ToT/Event counter will be saved. For single detector size should be 65536
// [out] frameData2 - pointer to buffer where data from ToA/Event counter will be saved. For single detector size should be 65536
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
PXCAPI int pxcMeasureSingleFrameTpx2(unsigned deviceIndex, double frameTime, unsigned* frameData1, unsigned* frameData2, unsigned* size, unsigned trgStg);

// Performs a measurement of single frame and returns its data. The ToT will be calibrated. This is only for Timepix2 detector. Calibration must be enabled by pxcSetTimepix2CalibrationEnabled function;
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameTime - time of the measurement in seconds
// [out] frameData1 - pointer to buffer where data from ToT counter will be saved. For single detector size should be 65536
// [out] frameData2 - pointer to buffer where data from ToA/Event counter will be saved. For single detector size should be 65536
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
PXCAPI int pxcMeasureSingleCalibratedFrameTpx2(unsigned deviceIndex, double frameTime, double* frameData1, unsigned* frameData2, unsigned* size, unsigned trgStg);

// Performs a measurement of single frame and returns its data. This is only for Timepix3 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameTime - time of the measurement in seconds
// [out] frameToaItot - pointer to buffer where data from Toa/iTot counter will be saved. For single detector size should be 65536
// [out] frameTotEvent - pointer to buffer where data from Tot/Event counter will be saved. For single detector size should be 65536
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
PXCAPI int pxcMeasureSingleFrameTpx3(unsigned deviceIndex, double frameTime, double* frameToaITot, unsigned short* frameTotEvent, unsigned* size, unsigned trgStg = PXC_TRG_NO);

// Performs a measurement of several frames to memory
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameCount - number of frames to measure
// [in]  frameTime - time of the measurement in seconds
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
PXCAPI int pxcMeasureMultipleFrames(unsigned deviceIndex, unsigned frameCount, double frameTime, unsigned trgStg = PXC_TRG_NO);

// Performs a measurement of several frames to memory
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameCount - number of frames to measure
// [in]  frameTime - time of the measurement in seconds
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
// [in]  callback - pointer to function that will be called after each measured frame
// [in]  userData - pointer to user data that will be passed as argument to the callback function
PXCAPI int pxcMeasureMultipleFramesWithCallback(unsigned deviceIndex, unsigned frameCount, double frameTime, unsigned trgStg = PXC_TRG_NO, FrameMeasuredCallback callback = 0, intptr_t userData = 0);

// Performs a continuous frame measurement until aborted. The frames are saved in circular buffer
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameBufferSize - number of frames in circular buffer
// [in]  frameTime - time of the measurement in seconds
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
// [in]  callback - pointer to function that will be called after each measured frame
// [in]  userData - pointer to user data that will be passed as argument to the callback function
PXCAPI int pxcMeasureContinuous(unsigned deviceIndex, unsigned frameBufferSize, double frameTime, unsigned trgStg = PXC_TRG_NO, FrameMeasuredCallback callback = 0, intptr_t userData = 0);

// Performs a measurement with Timepix3 detector in Data driven mode (sparse pixel mode)
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  measTime - measurement time in seconds
// [in]  fileName - output file name
// [in]  trgStg - settings of external trigger. One of the PXC_TRG_XXX values.
PXCAPI int pxcMeasureTpx3DataDrivenMode(unsigned deviceIndex, double measTime, const char* fileName, unsigned trgStg = PXC_TRG_NO, AcqEventFunc callback = 0, intptr_t userData = 0);

// Aborts running measurement
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcAbortMeasurement(unsigned deviceIndex);

// Returns number of measured frames in memory
PXCAPI int pxcGetMeasuredFrameCount(unsigned deviceIndex);

// Save a measured frame to file
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameIndex - index of the frame (starting from 0)
// [in]  filePath - file path where frame will be saved
PXCAPI int pxcSaveMeasuredFrame(unsigned deviceIndex, unsigned frameIndex, const char* filePath);

// Gets data of specified measured frame from memory. Only for Medipix2 and Timepix detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameIndex - index of the frame (starting from 0)
// [out] frameData - pointer to buffer where data will be saved. For single detector size should be 65536
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
PXCAPI int pxcGetMeasuredFrame(unsigned deviceIndex, unsigned frameIndex, unsigned short* frameData, unsigned* size);

// Gets data of specified measured frame from memory. Only for Medipix3 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameIndex - index of the frame (starting from 0)
// [out] frameData1 - pointer to buffer where data from first counter will be saved. For single detector size should be 65536.
// [out] frameData2 - pointer to buffer where data from second counter will be saved. For single detector size should be 65536.
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
PXCAPI int pxcGetMeasuredFrameMpx3(unsigned deviceIndex, unsigned frameIndex, unsigned* frameData1, unsigned* frameData2, unsigned* size);

// Gets data of specified measured frame from memory. Only for Timepix3 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameIndex - index of the frame (starting from 0)
// [out] frameToaITot - pointer to buffer where data from Toa/iTot counter will be saved. For single detector size should be 65536
// [out] frameTotEvent - pointer to buffer where data from Tot/Event counter will be saved. For single detector size should be 65536
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
PXCAPI int pxcGetMeasuredFrameTpx3(unsigned deviceIndex, unsigned frameIndex, double* frameToaITot, unsigned short* frameTotEvent, unsigned* size);

// Gets data of specified measured frame from memory. Only for Timepix2 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameIndex - index of the frame (starting from 0)
// [out] frameData1 - pointer to buffer where data from Toa/ToT/Event counter will be saved. For single detector size should be 65536
// [out] frameData2 - pointer to buffer where data from Toa/Event counter will be saved. For single detector size should be 65536
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
PXCAPI int pxcGetMeasuredFrameTpx2(unsigned deviceIndex, unsigned frameIndex, unsigned* frameData1, unsigned* frameData2, unsigned* size);

// Gets data of specified measured frame from memory. Only for Timepix2 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  frameIndex - index of the frame (starting from 0)
// [out] frameData1 - pointer to buffer where data from ToT counter will be saved. For single detector size should be 65536
// [out] frameData2 - pointer to buffer where data from Toa/Event counter will be saved. For single detector size should be 65536
// [out] size - pointer to variable specifying the size of buffer. The actual size will be output to this variable
PXCAPI int pxcGetMeasuredCalibratedFrameTpx2(unsigned deviceIndex, unsigned frameIndex, double* frameData1, unsigned* frameData2, unsigned* size);

// Gets the number of measured tpx3 pixels
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] pixelCount - pointer to variable where pixel count is saved
PXCAPI int pxcGetMeasuredTpx3PixelsCount(unsigned deviceIndex, unsigned* pixelCount);

// Gets the measured pixels from Timepix3 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out]  pixels - buffer where pixels will be saved
// [in]  pixelCount - size of pixels buffer (number of pixels)
PXCAPI int pxcGetMeasuredTpx3Pixels(unsigned deviceIndex, Tpx3Pixel* pixels, unsigned pixelCount);

// Gets the measured pixels from Timepix3 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out]  pixels - buffer where pixels will be saved
// [in]  pixelCount - size of pixels buffer (number of pixels)
PXCAPI int pxcGetMeasuredRawTpx3Pixels(unsigned deviceIndex, RawTpx3Pixel* pixels, unsigned pixelCount);

// Calibrate the pixel data and filter them by energy threshold
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [inout]  pixels - buffer with pixel data that will be calibrated and filtered
// [inout]  pixelCount - size of pixels buffer (number of pixels). It will be filled with the number of pixel remaining after filtration
// [in]  minEnergy - minimal allowed energy of each pixel
// [in]  maxEnergy - maximum allowed energy of each pixel
PXCAPI int pxcCalibrateTpx3PixelsAndFilter(unsigned deviceIndex, Tpx3Pixel* pixels, unsigned* pixelCount, double minEnergy, double maxEnergy);

// Enables/disables the calibration of Timepix3 ToT counts to energy in keV
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  enabled - if calibraiton is enabled
PXCAPI int pxcSetTimepix3CalibrationEnabled(unsigned deviceIndex, bool enabled);

// Gets if the calibration of Timepix3 ToT counts to energy in keV is enabled (enabled > 0, disabled = 0, error < 0)
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcIsTimepix3CalibrationEnabled(unsigned deviceIndex);

// Gets measured data meta data value
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  dataIndex - index of the measured frame or 0 for other measured data (e.g. tpx3 pixels)
// [in]  metaDataName - name of the meta data to get
// [out] valuebuffer  - buffer where the value of the meta data as string will be stored
// [out] size        - size of the supplied buffer
PXCAPI int pxcGetMetaDataValue(unsigned deviceIndex, unsigned dataIndex, const char* metaDataName, char* valueBuffer, unsigned* size);

// Returns the value of device parameter
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  parameterName - name of the device parameter
PXCAPI int pxcGetDeviceParameter(unsigned deviceIndex, const char* parameterName);

// Sets a value of device parameter
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  parameterName - name of the device parameter
// [in]  parameterValue - new value of the parameter
PXCAPI int pxcSetDeviceParameter(unsigned deviceIndex, const char* parameterName, int parameterValue);

// Returns the value of device parameter
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  parameterName - name of the device parameter
// [out] parameterValue - pointer to double variable where parameter value will be stored
PXCAPI int pxcGetDeviceParameterDouble(unsigned deviceIndex, const char* parameterName, double* parameterValue);

// Sets a value of device parameter
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  parameterName - name of the device parameter
// [in]  parameterValue - new value of the parameter
PXCAPI int pxcSetDeviceParameterDouble(unsigned deviceIndex, const char* parameterName, double parameterValue);

// Returns the value of device parameter
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  parameterName - name of the device parameter
// [out] parameterValue - buffer where the value of the paramter will be stored
// [in]  size - size of the output buffer
PXCAPI int pxcGetDeviceParameterString(unsigned deviceIndex, const char* parameterName, char* parameterValue, unsigned size);

// Sets a value of device parameter
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  parameterName - name of the device parameter
// [in]  parameterValue - new value of the parameter
PXCAPI int pxcSetDeviceParameterString(unsigned deviceIndex, const char* parameterName, const char* parameterValue);

// Sets the operation mode of Timepix3 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  timepix3 mode - PXC_TPX3_OPM_XX values
PXCAPI int pxcSetTimepix3Mode(unsigned deviceIndex, int mode);

// Sets the operation mode of Timepix2 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  timepix2 mode - PXC_TPX2_OPM_XX values
PXCAPI int pxcSetTimepix2Mode(unsigned deviceIndex, int mode);

// Sets the operation mode of Medipix3 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  opMode - PXC_MPX3_OPM_XX values
PXCAPI int pxcSetMedipix3OperationMode(unsigned deviceIndex, int opMode);

// Sets the operation gain mode of Medipix3 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  gain mode - gain mode of Medipix3 detector, PXC_MPX3_GAIN_MOD_XX values
PXCAPI int pxcSetMedipix3GainMode(unsigned deviceIndex, int gain);

// Sets voltage of hvsrc device
// [in] deviceIndex - index of the device (indexing starting from 0)
// [in] index - not used
// [in] voltage - voltage value
PXCAPI int pxcSetHvSrcVoltage(unsigned deviceIndex, int index, int voltage);

// Sets the operation gain mode of Timepix2 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  adaptiveGainOn - switch on/off the adaptive gain mode of Timepix2 detector
PXCAPI int pxcSetTimepix2AdaptiveGainMode(unsigned deviceIndex, bool adaptiveGainOn);

// Sets the analogue masking mode of Timepix2 detector
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  analogueMaskingOn - switch on/off the Analogue Masking mode of Timepix2 detector
PXCAPI int pxcSetTimepix2AnalogueMaskingMode(unsigned deviceIndex, bool analogueMaskingOn);

// Sets the parameters of acquisition. This is only for Medipix3 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  colorMode - if color mode is enabled
// [in]  csm - if charge summing mode is enabled
// [in]  gain - the valute of the gain (PXC_MPX3_GAIN_XX constants)
// [in]  equalize - if the equalizatio bit is active
PXCAPI int pxcSetMedipix3AcqParams(unsigned deviceIndex, bool colorMode, bool csm, int gain, bool equalize);

// Sets the parameters of pixel matrix. This is only for Medipix3 detector.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  depth - depth of the counters (PXC_MPX3_CNT_1B, 6B, 12B, ..)
// [in]  counter - selected counter for readout (PXC_MPX3_CNT_LOW, HIGH, BOTH)
// [in]  colBlock - what column blocks will be read. Default PXC_MPX3_COLB_ALL)
// [in]  rowBlock - what rows will be read. Default PXC_MPX3_ROWB_ALL)
PXCAPI int pxcSetMedipix3MatrixParams(unsigned deviceIndex, int depth, int counter, int colBlock, int rowBlock);

// Sets the Pixel Matrix Configuration. This is a low level function for advanced users.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  pixCfgData - pixel configuration data - TPXPixCfg, MPX3PixCfg or TPX3PixCfg encoded bytes
// [in]  byteSize - size of the pixel configuration data in bytes
PXCAPI int pxcSetPixelMatrix(unsigned deviceIndex, unsigned char* pixCfgData, unsigned byteSize);

// === DEVICE MASK - Bad/noisy pixels that normally masked ===
// Gets the Device Mask Matrix.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] maskMatrix - matrix setting which pixels are masked (PXC_PIXEL_MASKED = 0) and not masked (PXC_PIXEL_UNMASKED = 1)
// [in]  size - size of the matrix (for single chip 65536)
PXCAPI int pxcGetPixelMaskMatrix(unsigned deviceIndex, unsigned char* maskMatrix, unsigned size);

// === USER MASK - Additional masked pixels for user experiments - can be cleared before lost of normal mask ===
// Sets the User Mask Matrix.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  maskMatrix - matrix setting which pixels are masked (PXC_PIXEL_MASKED = 0) and not masked (PXC_PIXEL_UNMASKED = 1)
// [in]  size - size of the matrix (for single chip 65536)
PXCAPI int pxcSetUserMaskMatrix(unsigned deviceIndex, unsigned char* maskMatrix, unsigned size);

// Gets the User Mask Matrix.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] maskMatrix - matrix setting which pixels are masked (PXC_PIXEL_MASKED = 0) and not masked (PXC_PIXEL_UNMASKED = 1)
// [in]  size - size of the matrix (for single chip 65536)
PXCAPI int pxcGetUserMaskMatrix(unsigned deviceIndex, unsigned char* maskMatrix, unsigned size);

// Clears the User Mask (unmasks all pixels in user mask).
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcClearUserMask(unsigned deviceIndex);

// Register acquisition event callback that is called when corresponding event occurs
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  event - event to register callback to (PXC_ACQEVENT_XXX values)
// [in]  func - event callback function
// [in]  userData - data passed as argument to the callback when called
PXCAPI int pxcRegisterAcqEvent(unsigned deviceIndex, const char* event, AcqEventFunc func, intptr_t userData);

// Unregistered acquisition event callback that is called when corresponding event occurs
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  event - event to register callback to (PXC_ACQEVENT_XXX values)
// [in]  func - event callback function
// [in]  userData - data passed as argument to the callback when called
PXCAPI int pxcUnregisterAcqEvent(unsigned deviceIndex, const char* event, AcqEventFunc func, intptr_t userData);

// Sets the sensor refresh sequence text
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  refreshString - sensor refresh string
PXCAPI int pxcSetSensorRefresh(unsigned deviceIndex, const char* refreshString);

// Performs the sensor refresh
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcDoSensorRefresh(unsigned deviceIndex);

// Enables automatic sensor refresh before each acquisition series and at periodic intervals
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  enabled - if automatic sensor refresh is enabled
// [in]  refreshTime - sensor refresh is performed repeatedly after this time in seconds. If time is 0, then
//                     the refresh is done only once before the measurement.
PXCAPI int pxcEnableSensorRefresh(unsigned deviceIndex, bool enabled, double refreshTime=0);

// Enables TDI (Time Delayed Integration) measurements
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [in]  enabled - if TDI is enabled for the detectro for all measurement functions
PXCAPI int pxcEnableTDI(unsigned deviceIndex, bool enabled);

// Checks if the device is ready to accept software trigger
// [in]  deviceIndex - index of the device (indexing starting from 0)
// returns 1 if ready, 0 if not ready, and negative if error
PXCAPI int pxcIsReadyForSoftwareTrigger(unsigned deviceIndex);

// Sends the software trigger pulse
// [in]  deviceIndex - index of the device (indexing starting from 0)
PXCAPI int pxcDoSoftwareTrigger(unsigned deviceIndex);

// Adds a new mask (frame) for Beam-Hardening calibration.
// [in] data - data of the frame that will be used as BH mask
// [in] size - size of the data (number of pixels width*height)
// [in] frameTime - acquisition time of the frame in seconds
// [in] thickness - thickness of the measured plate
PXCAPI int pxcAddBHMask(unsigned* data, unsigned size, double frameTime, double thickness);

// Returns number of inserted Beam-Hardening masks (frames)
PXCAPI int pxcBHMaskCount();

// Removes an inserted Beam-Hardening mask (frame)
// [in] index - index of the mask to remove
PXCAPI int pxcRemoveBHMask(int index);

// Applies the Beam-Hardening correction to supplied frame.
// [in] inData - data of the frame that will be corrected
// [in] size - size of the data (number of pixels width*height)
// [in] frameTime - acquisition time of the frame to be corrected in seconds
// [out] outData - output data buffer where corrected data will be saved
PXCAPI int pxcApplyBHCorrection(unsigned* inData, unsigned size, double frameTime, double* outData);

// Gets the devices's matrix of bad pixels (badpixel = 1, good = 0). Bad pixels are pixels that are masked.
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] badPixelMatrix - buffer where the bad pixels will be stored
// [in]  size - size of the data (number of pixels width*height)
PXCAPI int pxcGetDeviceBadPixelMatrix(unsigned deviceIndex, unsigned char* badPixelMatrix, unsigned size);

// Gets the device's matrix of user-masked pixels (masked = 1, not masked = 0).
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out] userMaskMatrix - buffer where the user mask will be stored
// [in]  size - size of the data (number of pixels width*height)
PXCAPI int pxcGetDeviceUserMaskMatrix(unsigned deviceIndex, unsigned char* userMaskMatrix, unsigned size);

// Gets the bad pixel matrix from Beam Hardening correction - pixels that cannot be corrected. (bad pixel = 1, good = 0)
// [out] badPixelMatrix - buffer where the bad pixels will be stored
// [in]  size - size of the data (number of pixels width*height)
PXCAPI int pxcGetBHBadPixelMatrix(unsigned char* badPixelMatrix, unsigned size);

// Gets the combined bad pixel matrix from the device and BeamHardening (badpixel = 1, good = 0).
// [in] deviceIndex - index of the device (indexing starting from 0)
// [out] badPixelMatrix - buffer where the bad pixels will be stored
// [in] size - size of the data (number of pixels width*height)
PXCAPI int pxcGetDeviceAndBHBadPixelMatrix(unsigned deviceIndex, unsigned char* badPixelMatrix, unsigned size);

// Interpolates bad pixels in the image. Uses badPixelsMatrix as bad pixels (badPixel = 1, good = 0)
// [in] badPixelsMatrix - matrix of bad pixels
// [in/out]  data - data where bad pixels will be interpolated
// [in] width - width of the image
// [in] height - height of the image
PXCAPI int pxcInterpolateBadPixels(unsigned char* badPixelsMatrix, double* data, unsigned width, unsigned height);

// Returns text of last error. This function can be called even before pxcInitialize()
// [out] errorMsgBuffer - buffer where text will be saved
// [in]  size - size of supplied buffer
PXCAPI int pxcGetLastError(char* errorMsgBuffer, unsigned size);

// Returns a pointer for processing libraries
PXCAPI void* pxcGetIPixet();


struct NetworkDevInfo{
    char ip[15];
    char name[20];
    int serial;
};

// Gets array of ip addresses of available network devices
// [out] devInfos - array of struct NetworkDevInfo{char ip[15]; char name[20]; int serial;};
// [in/out] size - size of devInfos array, output is count of devices found
// Returns 0 if OK or PXCERR_BUFFER_SMALL if the number of devices exceeded the size of the array (the array is still filled and *size is set) 
PXCAPI int pxcListNetworkDevices(NetworkDevInfo* devInfos, unsigned* size);

enum DevType{
    TPX = 1, MPX3, TPX3, TPX2,
};

struct CDevInfo{
    char name[20];
    int serial;
    DevType type;
};

// Returns device ChipType, TPX = 1, MPX3 = 2, TPX3 = 3, TPX2 = 4
// [in]  deviceIndex - index of the device (indexing starting from 0)
// [out]  devInfo - struct DevInfo that will be filled.
PXCAPI int pxcGetDeviceInfo(unsigned deviceIndex, CDevInfo *devInfo);

// Finds noisy pixels and it can optionally masked them. Designed for devices in environment WITHOUT radiation background.
// Currently Tpx3 only
// [in] deviceIndex - index of the device (indexing starting from 0)
// [in] limitNoisy - limit to mask noisy pixels. It goes from 1 to 1022, where 1 is the most strongest masking and 1022 is the weakest.
// [in] limitSatur - limit to mask saturated pixels. It goes from 0 to 100, where 0 is the most strongest masking and 100 is the weakest.
// [in] doMaskNoisyPixels - check to mask noisy pixels within the process. Default is false alias do not mask.
// [in] matrixSize - size of the matrix.
// [in/out] noisyPixelsMatrix - mask/positions of noisy pixels.
PXCAPI int pxcFindNoisyPixels(unsigned deviceIndex, double limitNoisy = 50, double limitSatur = 50, bool doMaskNoisyPixels = false, 
                                unsigned* noisyPixelsMatrix = nullptr, unsigned matrixSize = 65536);
};

#endif /* !PXCAPI_H */

