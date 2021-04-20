/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.170.0
        Device            :  dsPIC33EP512GM706
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.61
        MPLAB 	          :  MPLAB X v5.45
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include <xc.h>
#include <stdbool.h>
#include <stdlib.h>

#define FCY 4000000UL // clock frequency
#include <libpic30.h>

#include "mcc_generated_files/mcc.h"
//#include "mcc_generated_files/system.h"
#include "mcc_generated_files/fatfs/fatfs_demo.h"
#include "mcc_generated_files/pwm_module_features.h"
#include "I2C_Handler.h"
#include "Initialize.h"

// Address Definitions
#define MUX_0 0x70
#define MUX_1 0x71

// number of ToF sensors
#define NUM_TOF 6

// pairing of mux numbers and port numbers
typedef struct {
    uint8_t mux;
    uint8_t port;
} muxPort_t;

// store (mux, port) pairs in an array
muxPort_t ToF[NUM_TOF] = {
    {0x00, 0x00},   // Left Center  --> MID LED
    {0x00, 0x01},   // Left         --> LEFT LED
    {0x00, 0x02},   // Left Left    --> LEFT LED
    {0x01, 0x00},   // Right Center --> MID LED
    {0x01, 0x01},   // Right        --> RIGHT LED
    {0x01, 0x02}    // Right Right  --> RIGHT LED
};

// LEFT LED == 0, MID LED == 1, RIGHT LED == 2
int LEDs[NUM_TOF] = {
    1, 0, 0, 1, 2, 2
};

/* * * * * * * * * * * RGB LED Color Definitions * * * * * * * * * * */
// These are utilizing the same I/O pins as the on-board LEDs
/*
 * (0x01) LED1 <==> L/R RED
 * (0x02) LED2 <==> L/R GRN
 * (0x04) LED3 <==> L/R BLU
 * (0x08) LED4 <==> R ENABLE (ENABLEs are active LOW)
 * (0x10) LED5 <==> L ENABLE
 * (0x20) LED6 <==> MID RED
 * (0x40) LED7 <==> MID GRN
 * (0x80) LED8 <==> MID BLU (MID LED has no ENABLE, is tied to GND)
*/
#define LR_RED  0x01
#define LR_GRN  0x02
#define LR_BLU  0x04
#define LR_YLW  0x03
#define LR_PRP  0x05
#define LR_ON   0x00 // both L and R on
#define R_ON    0x00
#define R_OFF   0x10
#define L_ON    0x00
#define L_OFF   0x08
#define MID_RED 0x20
#define MID_GRN 0x40
#define MID_BLU 0x80
#define MID_YLW 0x60
#define MID_PRP 0xA0

typedef enum { LED_L, LED_C, LED_R } LED_posn;


/* * * * * * * * * * * ToF Sensor Definitions * * * * * * * * * * */
# define MAX_DISTANCE 1500 // 1500mm = 1.5m, the maximum range of interest

#define VL53L0X_I2CADDR 0x29
// Record the current time to check an upcoming timeout against
#define startTimeout() (timeout_start_ms = millis())
// Check if timeout is enabled (set to nonzero value) and has expired
#define checkTimeoutExpired() (io_timeout > 0 && ((uint16_t)millis() - timeout_start_ms) > io_timeout)
// Decode VCSEL (vertical cavity surface emitting laser) pulse period in PCLKs
// from register value
// based on VL53L0X_decode_vcsel_period()
#define decodeVcselPeriod(reg_val)      (((reg_val) + 1) << 1)
// Encode VCSEL pulse period register value from period in PCLKs
// based on VL53L0X_encode_vcsel_period()
#define encodeVcselPeriod(period_pclks) (((period_pclks) >> 1) - 1)
// Calculate macro period in *nanoseconds* from VCSEL period in PCLKs
// based on VL53L0X_calc_macro_period_ps()
// PLL_period_ps = 1655; macro_period_vclks = 2304
#define calcMacroPeriod(vcsel_period_pclks) ((((uint32_t)2304 * (vcsel_period_pclks) * 1655) + 500) / 1000)

enum regAddr {
    SYSRANGE_START                              = 0x00,

    SYSTEM_THRESH_HIGH                          = 0x0C,
    SYSTEM_THRESH_LOW                           = 0x0E,

    SYSTEM_SEQUENCE_CONFIG                      = 0x01,
    SYSTEM_RANGE_CONFIG                         = 0x09,
    SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04,

    SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A,

    GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84,

    SYSTEM_INTERRUPT_CLEAR                      = 0x0B,

    RESULT_INTERRUPT_STATUS                     = 0x13,
    RESULT_RANGE_STATUS                         = 0x14,

    RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC,
    RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0,
    RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0,
    RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4,
    RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6,

    ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28,

    I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A,

    MSRC_CONFIG_CONTROL                         = 0x60,

    PRE_RANGE_CONFIG_MIN_SNR                    = 0x27,
    PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56,
    PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57,
    PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64,

    FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67,
    FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47,
    FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48,
    FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,

    PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61,
    PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62,

    PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50,
    PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51,
    PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52,

    SYSTEM_HISTOGRAM_BIN                        = 0x81,
    HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33,
    HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55,

    FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70,
    FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71,
    FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72,
    CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20,

    MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46,

    SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF,
    IDENTIFICATION_MODEL_ID                     = 0xC0,
    IDENTIFICATION_REVISION_ID                  = 0xC2,

    OSC_CALIBRATE_VAL                           = 0xF8,

    GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32,
    GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0,
    GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1,
    GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2,
    GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3,
    GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4,
    GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5,

    GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6,
    DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E,
    DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F,
    POWER_MANAGEMENT_GO1_POWER_FORCE            = 0x80,

    VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV           = 0x89,

    ALGO_PHASECAL_LIM                           = 0x30,
    ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30,
};

typedef enum { VcselPeriodPreRange, VcselPeriodFinalRange } vcselPeriodType;

uint8_t last_status; // status of last I2C transmission

typedef struct {
    bool tcc, msrc, dss, pre_range, final_range;
    // TCC: Target CentreCheck
    // MSRC: Minimum Signal Rate Check
    // DSS: Dynamic Spad Selection
} SequenceStepEnables;

typedef struct {
    uint16_t pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks;

    uint16_t msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks;
    uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
} SequenceStepTimeouts;

uint8_t address;
uint16_t io_timeout;
bool did_timeout;
uint16_t timeout_start_ms;
uint8_t stop_variable; // read by init and used when starting measurement; is StopVariable field of VL53L0X_DevData_t structure in API
uint32_t measurement_timing_budget_us;


/* * * * * * * * * * * * I2C Bus Read/Write Functions * * * * * * * * * * * */
I2C1_MESSAGE_STATUS I2C_Status = I2C1_MESSAGE_COMPLETE; // I2C Bus Status

void selectPort(muxPort_t sensorPort);
void selectPort2(uint8_t mux, uint8_t port); // select mux port
uint8_t getMuxCtrlReg(uint8_t mux);          // read interrupt values from mux
uint8_t getInterrupts(uint8_t mux);
void writeRegister(uint8_t dev, uint8_t reg, uint8_t data);
void writeRegister_16b(uint8_t dev, uint8_t reg, uint16_t data);
void writeRegister_32b(uint8_t dev, uint8_t reg, uint32_t data);
uint8_t readReg(uint8_t dev, uint8_t reg);
void readRegister(uint8_t dev, uint8_t reg, uint8_t* data);
uint16_t readRegister_16b(uint8_t dev, uint8_t reg);


/* * * * * * * * * * * * * * ToF Helper Functions * * * * * * * * * * * * * */
void initSingleToF(int ToF_num, uint16_t *dists);
void initAllToF(uint16_t *dists);
void initAllToF2(uint16_t *dists);
void getSingleToF(int ToF_num, uint16_t *dists);
void getAllToF(uint16_t *dists);
void getAllToF2(uint16_t *dists);
uint8_t getNearestObstacleIndex(uint16_t *dists);
uint8_t getNearestObstacleIndex2(uint16_t *dists);


/* * * * * * * * * * * Accelerometer Definitions * * * * * * * * * * */
//<Nick>
#define H3LIS200DL_I2CADDR 0x19
#define H3LIS200DL_WHO_AM_I 0x0F
#define H3LIS200DL_CTRL_REG1 0x20
#define H3LIS200DL_CTRL_REG2 0x21
#define H3LIS200DL_CTRL_REG3 0x22
#define H3LIS200DL_CTRL_REG4 0x23
#define H3LIS200DL_CTRL_REG5 0x24
#define H3LIS200DL_HP_FILTER_RESET 0x25
#define H3LIS200DL_REFERENCE 0x26
#define H3LIS200DL_STATUS_REG 0x27
#define H3LIS200DL_OUT_X_H 0x29               // X Data
#define H3LIS200DL_OUT_X_L 0x28
#define H3LIS200DL_OUT_Y_H 0x2B               // Y Data
#define H3LIS200DL_OUT_Y_L 0x2A
#define H3LIS200DL_OUT_Z_H 0x2D               // Z Data
#define H3LIS200DL_OUT_Z_L 0x2C
#define H3LIS200DL_INT1_CFG 0x30            // Interrupt 1 (Pin 11, used)
#define H3LIS200DL_INT1_SRC 0x31
#define H3LIS200DL_INT1_THS 0x32
#define H3LIS200DL_INT1_DURATION 0x33
#define H3LIS200DL_INT2_CFG 0x34            // Interrupt 2
#define H3LIS200DL_INT2_SRC 0x35
#define H3LIS200DL_INT2_THS 0x36
#define H3LIS200DL_INT2_DURATION 0x37
                                // POWER MODES
#define H3LIS200DL_PWR_DWN 0x00     // Power Down Mode
#define H3LIS200DL_NRML 0x01        // Normal Mode
#define H3LIS200DL_LP_0_5HZ 0x02    // Low Power 0.5Hz
#define H3LIS200DL_LP_1HZ 0x03      // Low Power 1.0Hz
#define H3LIS200DL_LP_2HZ 0x04      // Low Power 2.0Hz
#define H3LIS200DL_LP_5HZ 0x05      // Low Power 5.0Hz
#define H3LIS200DL_LP_10HZ 0x06     // Low Power 10.Hz
                                // OUTPUT DATA RATES
#define H3LIS200DL_DR_50HZ 0x00     // 50Hz
#define H3LIS200DL_DR_100HZ 0x01    // 100Hz
#define H3LIS200DL_DR_400HZ 0x02    // 400Hz
#define H3LIS200DL_DR_1000HZ 0x03   // 1000Hz

#define H3LIS200DL_EN_X 0x01    // Enable X Data
#define H3LIS200DL_EN_Y 0x02    // Enable Y Data
#define H3LIS200DL_EN_Z 0x04    // Enable Z Data
#define H3LIS200DL_EN_XYZ 0x07  // Enable X, Y, and Z Data


/* * * * * * * * * * * Accelerometer Function Definitions * * * * * * * * * * */
typedef enum {USE_I2C, USE_SPI} comm_mode;
typedef enum {POWER_DOWN, NORMAL, LOW_POWER_0_5HZ, LOW_POWER_1HZ,
                LOW_POWER_2HZ, LOW_POWER_5HZ, LOW_POWER_10HZ} power_mode;
typedef enum {DR_50HZ, DR_100HZ, DR_400HZ, DR_1000HZ} data_rate;
typedef enum {HPC_8, HPC_16, HPC_32, HPC_64} high_pass_cutoff_freq_cfg;
typedef enum {PUSH_PULL, OPEN_DRAIN} pp_od;
typedef enum {INT_SRC, INT1_2_SRC, DRDY, BOOT} int_sig_src;
typedef enum {LOW_RANGE, MED_RANGE, NO_RANGE, HIGH_RANGE} fs_range;
typedef enum {X_AXIS, Y_AXIS, Z_AXIS} int_axis;
typedef enum {TRIG_ON_HIGH, TRIG_ON_LOW} trig_on_level;

void H3LIS200DL_begin();
void H3LIS200DL_axesEnable(bool enable);
void H3LIS200DL_setPowerMode(power_mode pmode);
void H3LIS200DL_setODR(data_rate drate);
void H3LIS200DL_readAxes(int16_t* x, int16_t* y, int16_t* z);
uint8_t H3LIS200DL_readReg(uint8_t reg_address);
int16_t H3LIS200DL_convertToG(int16_t maxScale, int16_t reading);
void H3LIS200DL_setHighPassCoeff(high_pass_cutoff_freq_cfg hpcoeff);
void H3LIS200DL_enableHPF(bool enable);
void H3LIS200DL_HPFOnIntPin(bool enable, uint8_t pin);
void H3LIS200DL_intActiveHigh(bool enable);
void H3LIS200DL_intPinMode(pp_od _pinMode);
void H3LIS200DL_latchInterrupt(bool enable, uint8_t intSource);
void H3LIS200DL_intSrcConfig(int_sig_src src, uint8_t pin);
void H3LIS200DL_setFullScale(fs_range range);
bool H3LIS200DL_newXData();
bool H3LIS200DL_newYData();
bool H3LIS200DL_newZData();
void H3LIS200DL_enableInterrupt(int_axis axis, trig_on_level trigLevel,uint8_t interrupt, bool enable);
void H3LIS200DL_setIntDuration(uint8_t duration, uint8_t intSource);
void H3LIS200DL_setIntThreshold(uint8_t threshold, uint8_t intSource);
int16_t H3LIS200DL_Read_x(int16_t x);
int16_t H3LIS200DL_Read_y(int16_t y);
int16_t H3LIS200DL_Read_z(int16_t z);
//</Nick>
bool getAccelPoints(void);


/* * * * * * * * * * * SD Card Functions * * * * * * * * * * */
void writeTemplateToSD(void);
void writeAccelToSD(void);


/* * * * * * * * * * * Time-of-Flight Sensor Functions * * * * * * * * * * */
uint8_t VL53L0X_init(void);
bool VL53L0X_config(void);
bool VL53L0X_setSignalRateLimit(float limit_Mcps);
bool VL53L0X_getSpadInfo(uint8_t * count, bool * type_is_aperture);
uint32_t VL53L0X_getMeasurementTimingBudget(void);
void VL53L0X_getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts);
uint8_t VL53L0X_getVcselPulsePeriod(vcselPeriodType type);
bool VL53L0X_setMeasurementTimingBudget(uint32_t budget_us);
void VL53L0X_getSequenceStepEnables(SequenceStepEnables * enables);
uint16_t VL53L0X_encodeTimeout(uint16_t timeout_mclks);
bool VL53L0X_performSingleRefCalibration(uint8_t vhv_init_byte);
uint32_t VL53L0X_timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks);
uint32_t VL53L0X_timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks);
uint16_t VL53L0X_decodeTimeout(uint16_t reg_val);
void VL53L0X_startContinuous(uint32_t period_ms);
void VL53L0X_stopContinuous(void);
uint16_t VL53L0X_readRangeContinuousMillimeters(void);
uint16_t VL53L0X_readRangeSingleMillimeters(void);
inline void VL53L0X_setTimeout(uint16_t timeout) { io_timeout = timeout; }
inline uint16_t VL53L0X_getTimeout(void) { return io_timeout; }
bool VL53L0X_timeoutOccurred(void);


/* * * * * * * * * * * * * * LED Display Functions * * * * * * * * * * * * * */
void showBinary(uint8_t n);
void showStartup(void);
void showStartupRGB(void);
void showDistanceRGB(uint16_t dist, LED_posn LED);
void showInitRGB(int index);
void showConcussion(void);
void showCount(void);
void showError(void);


/* * * * * * * * * * * * * Accelerometer Variables * * * * * * * * * * * * */
// <Nick>
int16_t x_1, y_1, z_1;
int16_t x_2, y_2, z_2;
int16_t x_3, y_3, z_3;
int16_t x_4, y_4, z_4;
int16_t x_5, y_5, z_5;
int16_t thresh = 50; // impact threshold (in Gs))
int16_t max; // current maximum axis reading

// timestamps of readings
unsigned long timer1, timer2, timer3, timer4, timer5;

// c-strings for writing data to SD card
char data1[255];
char data2[255];
char data3[255];
char data4[255];
char data5[255];
// </Nick>

// set TRUE to automatically clear the interrupt of each ToF sensor upon reading
bool auto_int_clr = false;

/*
                         Main application
 */
int main(void) {
    /* * * * * * * * * * * * * * * ToF * * * * * * * * * * * * * * */
    address = VL53L0X_I2CADDR;
    io_timeout = 0;
    did_timeout = false;
    // distances from each ToF sensor
    int ToF_to_test = 2; // 0, 1, 2, 3, 4, 5
    uint8_t dist_8; // 8-bit integer used to display the 8 LSBs of distance
    uint16_t distances[NUM_TOF] = { 0xFFFF, 0xFFFF, 0xFFFF,
                                    0xFFFF, 0xFFFF, 0xFFFF};

    
    /* * * * * * * * * * * * * * * SETUP * * * * * * * * * * * * * * */
    SYSTEM_Initialize();    // MCC: I2C1, TMR1, SPI1 initialization
    Start_Initialization(); // for pins/LEDs
    I2C1_MESSAGE_STATUS I2C_Status = I2C1_MESSAGE_COMPLETE;
    
    //showStartup();    // run on-board LED startup pattern
    showStartupRGB();   // run RGB LED startup pattern
    
    //writeTemplateToSD();
    
    /* * * * * * * * * * * * * * ToF Setup * * * * * * * * * * * * * */
//    initSingleToF(ToF_to_test, distances); // initialize a single ToF sensor
    initAllToF(distances); // initialize all ToF sensors
    uint8_t index; // hold the index of the sensor detecting the nearest object

    while (1) {
        
        /* * * * * * * * * * * * * * Accelerometer * * * * * * * * * * * * * */
        if(getAccelPoints()) { // get accel data, returns true if max >= thresh
            writeAccelToSD();
        }
        
        /*************** I2C Time-of-Flight **************/
        getAllToF(distances);      // get the distances from all ToF sensors
        
        // get the distance from the ToF_to_test sensor
//        getSingleToF(ToF_to_test, distances);

        /************** SHOW READING **************/
//        dist_8 = distances[ToF_to_test] & 0xFF;
        // show reading LSBs via on-board LEDs
//        showBinary(dist_8); 
//        msTimerDelay(10);
        // show RGB corresponding to reading of ToF_to_test
//        showDistanceRGB(distances[ToF_to_test], LEDs[ToF_to_test]);
        
        // find the sensor detecting the closest obstacle
        index = getNearestObstacleIndex(distances);
        // show RGB corresponding to reading of said sensor
        if(index == 0xFF) {
            showBinary(0x00);
        }
        else {
            showDistanceRGB(distances[index], LEDs[index]);
        }

    }
    return 0; 
}

// displays the byte value in binary on the LEDs
void showBinary(uint8_t n) {
    LED1 = n;
    LED2 = n >> 1;
    LED3 = n >> 2;
    LED4 = n >> 3;
    LED5 = n >> 4;
    LED6 = n >> 5;
    LED7 = n >> 6;
    LED8 = n >> 7;
}

// a fun visual to do at boot
void showStartup(void) {
    uint16_t delay = 25;
    
    uint8_t display = 0x01;
    while(display != 0x80) {     // run one LED "up"
        display = display << 1;
        showBinary(display);
        msTimerDelay(delay);
    }
    while(display > 0x00) {     // run one LED back "down"
        display = display >> 1;
        showBinary(display);
        msTimerDelay(delay);
    }
    
    display = 0x00;             // reset to 0
    while(display < 0xFF) {     // "fill" LEDs
        display = display << 1;
        showBinary(++display);
        msTimerDelay(delay);
    }
    while(display > 0x00) {     // "empty" LEDs
        display = display >> 1;
        showBinary(display);
        msTimerDelay(delay);
    }
    
}

void showStartupRGB(void) {
    uint16_t delay = 200;
    
    showBinary(LR_RED | LR_ON | MID_RED);
    msTimerDelay(delay);
    
    showBinary(LR_PRP | LR_ON | MID_PRP);
    msTimerDelay(delay);

    showBinary(LR_BLU | LR_ON | MID_BLU);
    msTimerDelay(delay);
    
    showBinary(LR_BLU | LR_GRN | LR_ON | MID_BLU | MID_GRN);
    msTimerDelay(delay);

    showBinary(LR_GRN | LR_ON | MID_GRN);
    msTimerDelay(delay);
}

void showDistanceRGB(uint16_t dist, LED_posn LED) {
    if(1900 < dist) {
        showBinary(0x00);
    }
    else if(LED == LED_C) {
        if(dist <= 500) {
            showBinary(MID_RED);
        } else if(500 < dist && dist <= 1000) {
            showBinary(MID_GRN);
        } else if(1000 < dist && dist <= 1500) {
            showBinary(MID_BLU);
        }
    }
    else if(LED == LED_L) {
        if(dist <= 500) {
            showBinary(LR_RED | R_OFF);
        } else if(500 < dist && dist <= 1000) {
            showBinary(LR_GRN | R_OFF);
        } else if(1000 < dist && dist <= 1500) {
            showBinary(LR_BLU | R_OFF);
        }
    }
    else if(LED == LED_R) {
        if(dist <= 500) {
            showBinary(LR_RED | L_OFF);
        } else if(500 < dist && dist <= 1000) {
            showBinary(LR_GRN | L_OFF);
        } else if(1000 < dist && dist <= 1500) {
            showBinary(LR_BLU | L_OFF);
        }
    }
}

void showInitRGB(int index) {
    int clr = index % 3; // determines what color to turn the LED
        // 0 -> RED
        // 1 -> YELLOW
        // 2 -> WHITE
    
    if(index < (NUM_TOF/2)) { // left side sensors
        if(clr == 0) {
            showBinary(LR_RED | R_OFF); // red
        } else if(clr == 1) {
            showBinary(LR_RED | LR_GRN | R_OFF); // yellow
        } else if(clr == 2) {
            showBinary(LR_RED | LR_GRN | LR_BLU | R_OFF); // white
        }
    }
    else if(( (NUM_TOF/2) < index ) && ( index < NUM_TOF )) { // right side sensors
        if(clr == 0) {
            showBinary(LR_RED | L_OFF); // red
        } else if(clr == 1) {
            showBinary(LR_RED | LR_GRN | L_OFF); // yellow
        } else if(clr == 2) {
            showBinary(LR_RED | LR_GRN | LR_BLU | L_OFF); // white
        }
    }
}

void showConcussion(void) {
    uint16_t delay = 1000;
    
    // show all red
    showBinary(LR_RED | LR_ON | MID_RED);
    msTimerDelay(delay);
    
    // show all purple
    showBinary(LR_PRP | LR_ON | MID_PRP);
    msTimerDelay(delay);
    
    // show all red
    showBinary(LR_RED | LR_ON | MID_RED);
    msTimerDelay(delay);
    
    // show all purple
    showBinary(LR_PRP | LR_ON | MID_PRP);
    msTimerDelay(delay);
    
    // show all red
    showBinary(LR_RED | LR_ON | MID_RED);
    msTimerDelay(delay);
}

// a visual for errors
void showError(void) {
    uint16_t delay = 50;
    
    uint8_t i = 0x00;
    while(i < 3) { // flash alternating LEDs 3 times
        showBinary(0x55);
        msTimerDelay(delay);
        showBinary(0xAA);
        msTimerDelay(delay);
        i++;
    }
}

/* * * * * * * * * * * * I2C Bus Read/Write Functions * * * * * * * * * * * */

void selectPort(muxPort_t sensorPort) {
    if(sensorPort.mux == 0x00) {
        writeRegister(MUX_1, 0x00, 0x00); // disable mux 1
    }
    else if(sensorPort.mux == 0x01) {
        writeRegister(MUX_0, 0x00, 0x00); // disable mux 0
    }
    // base mux address is 0x70
    writeRegister((MUX_0 + sensorPort.mux), 0x00, (0x04 + sensorPort.port));
}

void selectPort2(uint8_t mux, uint8_t port) {
    // base mux address is 0x70
    if(mux == 0x00) {
        writeRegister(MUX_1, 0x00, 0x00);
        writeRegister(MUX_0, 0x00, (0x04 + port));
    }
    else if(mux == 0x01) {
        writeRegister(MUX_0, 0x00, 0x00);
        writeRegister(MUX_1, 0x00, (0x04 + port));
    }
    
}

// read the interrupts from mux
uint8_t getMuxCtrlReg(uint8_t mux) {
    // read the control register of mux
    return readReg((MUX_0 + mux), 0x00);
}

// writes to the device a byte of data to the register
void writeRegister(uint8_t dev, uint8_t reg, uint8_t data) {
    uint8_t config[] = {reg, data};
    
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterWrite(config, sizeof(config), dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    // showBinary(I2C_Status);
    msTimerDelay(5);
}

// writes to the device a byte of data to the register
void writeRegister_16b(uint8_t dev, uint8_t reg, uint16_t data) {
    // send MSB then LSB
    uint8_t config[] = {reg, data >> 8, data & 0x00FF};
    
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterWrite(config, sizeof(config), dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    msTimerDelay(5);
}

void writeRegister_32b(uint8_t dev, uint8_t reg, uint32_t data) {
    // send MSB, ..., then LSB
    uint8_t config[] = {reg, data >> 24, data >> 16 & 0x00FF, data >> 8 & 0x00FF, data & 0x00FF};
    
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterWrite(config, sizeof(config), dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    msTimerDelay(5);
}

// reads from the device a byte of data from the given register
void readRegister(uint8_t dev, uint8_t reg, uint8_t* data) {
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterWrite(&reg, 1, dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterRead(data, 1, dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    msTimerDelay(5);
}

uint16_t readRegister_16b(uint8_t dev, uint8_t reg) {
    uint8_t data[2];
    
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterWrite(&reg, 1, dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterRead(data, 2, dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    
    msTimerDelay(5);
    
    return ((data[0] << 8) + data[1]); // MSB first (VL53L0X pg. 15)
    //return (data[0] >> 8 + data[1]); // LSB first
    // TODO: switch endianness if need be
}

uint8_t data;
uint8_t readReg(uint8_t dev, uint8_t reg) {
    
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterWrite(&reg, 1, dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterRead(&data, 1, dev, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    msTimerDelay(5);
    
    return data;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * ToF Helper Functions * * * * * * * * * * * * * */

void initSingleToF(int ToF_num, uint16_t *dists) {
    selectPort(ToF[ToF_num]);
    msTimerDelay(1);
    VL53L0X_init();
    dists[ToF_num] = VL53L0X_readRangeSingleMillimeters();
}

void initAllToF(uint16_t *dists) {
    int i = 0;
    for(i = 0; i < 6; i++) {
        selectPort(ToF[i]);
        msTimerDelay(1);
        VL53L0X_init();
        dists[i] = VL53L0X_readRangeSingleMillimeters();
        if(!auto_int_clr) {
            writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CLEAR, 0x01); // clear interrupt
        }
        showInitRGB(i); // show RGB LED change on each side as sensors init
    }
    
    // turn all on white for 1 second to show initialization is done
    showBinary(LR_RED | LR_GRN | LR_BLU | LR_ON | MID_RED | MID_GRN | MID_BLU);
    msTimerDelay(1000);
}

void initAllToF2(uint16_t *dists) {
    int i = 1;
//    selectPort(ToF[i]);
//    msTimerDelay(1);
//    VL53L0X_init();
//    dists[i] = VL53L0X_readRangeSingleMillimeters();
//    if(!auto_int_clr) {
//        writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CLEAR, 0x01); // clear interrupt
//    }

    i = 3;
    selectPort(ToF[i]);
    msTimerDelay(1);
    VL53L0X_init();
    dists[i] = VL53L0X_readRangeSingleMillimeters();
    if(!auto_int_clr) {
        writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CLEAR, 0x01); // clear interrupt
    }

    i = 4;
    selectPort(ToF[i]);
    msTimerDelay(1);
    VL53L0X_init();
    dists[i] = VL53L0X_readRangeSingleMillimeters();
    if(!auto_int_clr) {
        writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CLEAR, 0x01); // clear interrupt
    }

    i = 5;
    selectPort(ToF[i]);
    msTimerDelay(1);
    VL53L0X_init();
    dists[i] = VL53L0X_readRangeSingleMillimeters();
    if(!auto_int_clr) {
        writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CLEAR, 0x01); // clear interrupt
    }
}

void getSingleToF(int ToF_num, uint16_t *dists) {
    selectPort(ToF[ToF_num]); // Select Multiplexer 0, Port 0
    msTimerDelay(1);
    dists[ToF_num] = VL53L0X_readRangeSingleMillimeters();
    msTimerDelay(1);
}

void getAllToF(uint16_t *dists) {
    int i = 0;
    for(i = 0; i < 6; i++) {
        selectPort(ToF[i]);
        msTimerDelay(1);
        dists[i] = VL53L0X_readRangeSingleMillimeters();
        msTimerDelay(1);
    }
}

void getAllToF2(uint16_t *dists) {
    int i = 1;
//    selectPort(ToF[i]); // Select Multiplexer 0, Port 1
//    msTimerDelay(1);
//    VL53L0X_startContinuous(0);
//    dists[i] = VL53L0X_readRangeSingleMillimeters();
//    msTimerDelay(1);
        
    i = 3;
    selectPort(ToF[i]); // Select Multiplexer 1, Port 0
    msTimerDelay(1);
    dists[i] = VL53L0X_readRangeSingleMillimeters();
    msTimerDelay(1);
    
    i = 4;
    selectPort(ToF[i]); // Select Multiplexer 1, Port 1
    msTimerDelay(1);
    dists[i] = VL53L0X_readRangeSingleMillimeters();
    msTimerDelay(1);
    
    i = 5;
    selectPort(ToF[i]); // Select Multiplexer 1, Port 2
    msTimerDelay(1);
    dists[i] = VL53L0X_readRangeSingleMillimeters();
    msTimerDelay(1);
}

// given the array of distances, return the index of the closest object
uint8_t getNearestObstacleIndex(uint16_t *dists) {
    uint8_t index = 0xFF;
    uint16_t min = 0xFFFF;        // 65535
    
    int i = 0;
    for(i = 0; i < NUM_TOF; i++) {
        if(dists[i] < min) {    // if this is the minimum value
            min = dists[i];     // min is the minimum value
            index = i;          // update the value of index
        }
    }
    
    return index;               // 255 indicates all sensors returned 65535
}

uint8_t getNearestObstacleIndex2(uint16_t *dists) {
    uint8_t index = 0xFF;
    uint16_t min = 0xFFFF;        // 65535
    
    int i = 1;
    if(dists[i] < min) {    // if this is the minimum value
        min = dists[i];     // min is the minimum value
        index = i;          // update the value of index
    }

    i = 3;
    if(dists[i] < min) {    // if this is the minimum value
        min = dists[i];     // min is the minimum value
        index = i;          // update the value of index
    }

    i = 4;
    if(dists[i] < min) {    // if this is the minimum value
        min = dists[i];     // min is the minimum value
        index = i;          // update the value of index
    }

    i = 5;
    if(dists[i] < min) {    // if this is the minimum value
        min = dists[i];     // min is the minimum value
        index = i;          // update the value of index
    }

    return index;               // 255 indicates all sensors returned 65535
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * * ToF Sensor Functions * * * * * * * * * * * * * */
uint8_t VL53L0X_init(void) {
    uint8_t success = 0;
                
    if(VL53L0X_config()) { // configure ToF
        success = 1;
    }
    
    VL53L0X_setTimeout(200); // was 500
    // Start continuous back-to-back mode (take readings as
    // fast as possible).  To use continuous timed mode
    // instead, provide a desired inter-measurement period in
    // ms (e.g. sensor.startContinuous(100))
    
    VL53L0X_setMeasurementTimingBudget(200000); // was 200000
    //VL53L0X_startContinuous(0); // was 0
    VL53L0X_stopContinuous();
    
    return success;
}

bool VL53L0X_config(void) {
    // VL53L0X_DataInit() begin
    uint8_t io_2v8 = 0;
    
    // sensor uses 1V8 mode for I/O by default; switch to 2V8 mode if necessary
    if (io_2v8) {
        writeRegister(VL53L0X_I2CADDR, VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV,
        readReg(VL53L0X_I2CADDR, VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV) | 0x01); // set bit 0
    }

    // "Set I2C standard mode"
    writeRegister(VL53L0X_I2CADDR, 0x88, 0x00);

    writeRegister(VL53L0X_I2CADDR, 0x80, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x00, 0x00);

    stop_variable = readReg(VL53L0X_I2CADDR, 0x91);

    writeRegister(VL53L0X_I2CADDR, 0x00, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x80, 0x00);

    // disable SIGNAL_RATE_MSRC (bit 1) and SIGNAL_RATE_PRE_RANGE (bit 4) limit checks
    writeRegister(VL53L0X_I2CADDR, MSRC_CONFIG_CONTROL, readReg(VL53L0X_I2CADDR, MSRC_CONFIG_CONTROL) | 0x12);

    // set final range signal rate limit to 0.25 MCPS (million counts per second)
    VL53L0X_setSignalRateLimit(0.25);

    writeRegister(VL53L0X_I2CADDR, SYSTEM_SEQUENCE_CONFIG, 0xFF);

    // VL53L0X_DataInit() end

    // VL53L0X_StaticInit() begin

    uint8_t spad_count;
    bool spad_type_is_aperture;
    if (!VL53L0X_getSpadInfo(&spad_count, &spad_type_is_aperture)) { return false; }

    // The SPAD map (RefGoodSpadMap) is read by VL53L0X_get_info_from_device() in
    // the API, but the same data seems to be more easily readable from
    // GLOBAL_CONFIG_SPAD_ENABLES_REF_0 through _6, so read it from there
    uint8_t ref_spad_map[6];

    VL53L0X_Read(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);

    // -- VL53L0X_set_reference_spads() begin (assume NVM values are valid)

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00);
    writeRegister(VL53L0X_I2CADDR, DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C);
    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4);

    uint8_t first_spad_to_enable = spad_type_is_aperture ? 12 : 0; // 12 is the first aperture spad
    uint8_t spads_enabled = 0;

    uint8_t i;
    for (i = 0; i < 48; i++) {
      if (i < first_spad_to_enable || spads_enabled == spad_count) {
        // This bit is lower than the first one that should be enabled, or
        // (reference_spad_count) bits have already been enabled, so zero this bit
        ref_spad_map[i / 8] &= ~(1 << (i % 8));
      }
      else if ((ref_spad_map[i / 8] >> (i % 8)) & 0x1) {
        spads_enabled++;
      }
    }

    // TODO: ?
    //writeMulti(GLOBAL_CONFIG_SPAD_ENABLES_REF_0, ref_spad_map, 6);
    uint8_t ad[] = {GLOBAL_CONFIG_SPAD_ENABLES_REF_0}; // this is a dumb workaround
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterWrite(ad, 1, VL53L0X_I2CADDR, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    I2C1_MasterWrite(ref_spad_map, sizeof(ref_spad_map), VL53L0X_I2CADDR, &I2C_Status);
    while(I2C_Status != I2C1_MESSAGE_COMPLETE);
    // -- VL53L0X_set_reference_spads() end

    // DefaultTuningSettings from vl53l0x_tuning.h
    // -- VL53L0X_load_tuning_settings() begin
    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x00, 0x00);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x09, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x10, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x11, 0x00);

    writeRegister(VL53L0X_I2CADDR, 0x24, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x25, 0xFF);
    writeRegister(VL53L0X_I2CADDR, 0x75, 0x00);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x4E, 0x2C);
    writeRegister(VL53L0X_I2CADDR, 0x48, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x30, 0x20);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x30, 0x09);
    writeRegister(VL53L0X_I2CADDR, 0x54, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x31, 0x04);
    writeRegister(VL53L0X_I2CADDR, 0x32, 0x03);
    writeRegister(VL53L0X_I2CADDR, 0x40, 0x83);
    writeRegister(VL53L0X_I2CADDR, 0x46, 0x25);
    writeRegister(VL53L0X_I2CADDR, 0x60, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x27, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x50, 0x06);
    writeRegister(VL53L0X_I2CADDR, 0x51, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x52, 0x96);
    writeRegister(VL53L0X_I2CADDR, 0x56, 0x08);
    writeRegister(VL53L0X_I2CADDR, 0x57, 0x30);
    writeRegister(VL53L0X_I2CADDR, 0x61, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x62, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x64, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x65, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x66, 0xA0);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x22, 0x32);
    writeRegister(VL53L0X_I2CADDR, 0x47, 0x14);
    writeRegister(VL53L0X_I2CADDR, 0x49, 0xFF);
    writeRegister(VL53L0X_I2CADDR, 0x4A, 0x00);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x7A, 0x0A);
    writeRegister(VL53L0X_I2CADDR, 0x7B, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x78, 0x21);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x23, 0x34);
    writeRegister(VL53L0X_I2CADDR, 0x42, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x44, 0xFF);
    writeRegister(VL53L0X_I2CADDR, 0x45, 0x26);
    writeRegister(VL53L0X_I2CADDR, 0x46, 0x05);
    writeRegister(VL53L0X_I2CADDR, 0x40, 0x40);
    writeRegister(VL53L0X_I2CADDR, 0x0E, 0x06);
    writeRegister(VL53L0X_I2CADDR, 0x20, 0x1A);
    writeRegister(VL53L0X_I2CADDR, 0x43, 0x40);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x34, 0x03);
    writeRegister(VL53L0X_I2CADDR, 0x35, 0x44);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x31, 0x04);
    writeRegister(VL53L0X_I2CADDR, 0x4B, 0x09);
    writeRegister(VL53L0X_I2CADDR, 0x4C, 0x05);
    writeRegister(VL53L0X_I2CADDR, 0x4D, 0x04);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x44, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x45, 0x20);
    writeRegister(VL53L0X_I2CADDR, 0x47, 0x08);
    writeRegister(VL53L0X_I2CADDR, 0x48, 0x28);
    writeRegister(VL53L0X_I2CADDR, 0x67, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x70, 0x04);
    writeRegister(VL53L0X_I2CADDR, 0x71, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x72, 0xFE);
    writeRegister(VL53L0X_I2CADDR, 0x76, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x77, 0x00);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x0D, 0x01);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x80, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x01, 0xF8);

    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x8E, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0x00, 0x01);
    writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
    writeRegister(VL53L0X_I2CADDR, 0x80, 0x00);
    // -- VL53L0X_load_tuning_settings() end

    // "Set interrupt config to new sample ready"
    // -- VL53L0X_SetGpioConfig() begin
    writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04);
    writeRegister(VL53L0X_I2CADDR, GPIO_HV_MUX_ACTIVE_HIGH, readReg(VL53L0X_I2CADDR, GPIO_HV_MUX_ACTIVE_HIGH) & ~0x10); // active low
    if(auto_int_clr) {
        writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CLEAR, 0x01); // clear interrupt
    }
    // -- VL53L0X_SetGpioConfig() end

    measurement_timing_budget_us = VL53L0X_getMeasurementTimingBudget();

    // "Disable MSRC and TCC by default"
    // MSRC = Minimum Signal Rate Check
    // TCC = Target CentreCheck
    // -- VL53L0X_SetSequenceStepEnable() begin

    writeRegister(VL53L0X_I2CADDR, SYSTEM_SEQUENCE_CONFIG, 0xE8);

    // -- VL53L0X_SetSequenceStepEnable() end

    // "Recalculate timing budget"
    VL53L0X_setMeasurementTimingBudget(measurement_timing_budget_us);

    // VL53L0X_StaticInit() end

    // VL53L0X_PerformRefCalibration() begin (VL53L0X_perform_ref_calibration())

    // -- VL53L0X_perform_vhv_calibration() begin

    writeRegister(VL53L0X_I2CADDR, SYSTEM_SEQUENCE_CONFIG, 0x01);
    if (!VL53L0X_performSingleRefCalibration(0x40)) { return false; }

    // -- VL53L0X_perform_vhv_calibration() end

    // -- VL53L0X_perform_phase_calibration() begin

    writeRegister(VL53L0X_I2CADDR, SYSTEM_SEQUENCE_CONFIG, 0x02);
    if (!VL53L0X_performSingleRefCalibration(0x00)) { return false; }

    // -- VL53L0X_perform_phase_calibration() end

    // "restore the previous Sequence Config"
    writeRegister(VL53L0X_I2CADDR, SYSTEM_SEQUENCE_CONFIG, 0xE8);

    // VL53L0X_PerformRefCalibration() end

    return true;
}

bool VL53L0X_setSignalRateLimit(float limit_Mcps) {
    if (limit_Mcps < 0 || limit_Mcps > 511.99) { return false; }

    // Q9.7 fixed point format (9 integer bits, 7 fractional bits)
    writeRegister_16b(VL53L0X_I2CADDR, FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, limit_Mcps * (1 << 7));
    return true;
}

// Get reference SPAD (single photon avalanche diode) count and type
// based on VL53L0X_get_info_from_device(),
// but only gets reference SPAD count and type
bool VL53L0X_getSpadInfo(uint8_t * count, bool * type_is_aperture) {
  uint8_t tmp;

  writeRegister(VL53L0X_I2CADDR, 0x80, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0x00, 0x00);

  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x06);
  writeRegister(VL53L0X_I2CADDR, 0x83, readReg(VL53L0X_I2CADDR, 0x83) | 0x04);
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x07);
  writeRegister(VL53L0X_I2CADDR, 0x81, 0x01);

  writeRegister(VL53L0X_I2CADDR, 0x80, 0x01);

  writeRegister(VL53L0X_I2CADDR, 0x94, 0x6b);
  writeRegister(VL53L0X_I2CADDR, 0x83, 0x00);
  startTimeout();
  // HELP
  while (readReg(VL53L0X_I2CADDR, 0x83) == 0x00) {
    if (checkTimeoutExpired()) { return false; }
  }
  writeRegister(VL53L0X_I2CADDR, 0x83, 0x01);
  readRegister(VL53L0X_I2CADDR, 0x92, &tmp);

  *count = tmp & 0x7f;
  *type_is_aperture = (tmp >> 7) & 0x01;

  writeRegister(VL53L0X_I2CADDR, 0x81, 0x00);
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x06);
  writeRegister(VL53L0X_I2CADDR, 0x83, readReg(VL53L0X_I2CADDR, 0x83) & ~0x04);  //(3) for this
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0x00, 0x01);

  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
  writeRegister(VL53L0X_I2CADDR, 0x80, 0x00);

  return true;
}

// Get the measurement timing budget in microseconds
// based on VL53L0X_get_measurement_timing_budget_micro_seconds()
// in us
uint32_t VL53L0X_getMeasurementTimingBudget(void) {
    SequenceStepEnables enables;
    SequenceStepTimeouts timeouts;

    uint16_t const StartOverhead     = 1910; // note that this is different than the value in set_
    uint16_t const EndOverhead        = 960;
    uint16_t const MsrcOverhead       = 660;
    uint16_t const TccOverhead        = 590;
    uint16_t const DssOverhead        = 690;
    uint16_t const PreRangeOverhead   = 660;
    uint16_t const FinalRangeOverhead = 550;

    // "Start and end overhead times always present"
    uint32_t budget_us = StartOverhead + EndOverhead;

    VL53L0X_getSequenceStepEnables(&enables);
    VL53L0X_getSequenceStepTimeouts(&enables, &timeouts);

    if (enables.tcc) {
      budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
    }

    if (enables.dss) {
      budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
    }
    else if (enables.msrc) {
      budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
    }

    if (enables.pre_range) {
      budget_us += (timeouts.pre_range_us + PreRangeOverhead);
    }

    if (enables.final_range) {
        budget_us += (timeouts.final_range_us + FinalRangeOverhead);
    }

    measurement_timing_budget_us = budget_us; // store for internal reuse
    return budget_us;
}

// Get sequence step enables
// based on VL53L0X_GetSequenceStepEnables()
void VL53L0X_getSequenceStepEnables(SequenceStepEnables * enables) {
  uint8_t sequence_config;
  readRegister(VL53L0X_I2CADDR, SYSTEM_SEQUENCE_CONFIG, &sequence_config);

  enables->tcc          = (sequence_config >> 4) & 0x1;
  enables->dss          = (sequence_config >> 3) & 0x1;
  enables->msrc         = (sequence_config >> 2) & 0x1;
  enables->pre_range    = (sequence_config >> 6) & 0x1;
  enables->final_range  = (sequence_config >> 7) & 0x1;
}

// Get sequence step timeouts
// based on get_sequence_step_timeout(),
// but gets all timeouts instead of just the requested one, and also stores
// intermediate values
void VL53L0X_getSequenceStepTimeouts(SequenceStepEnables const * enables, SequenceStepTimeouts * timeouts) {
  timeouts->pre_range_vcsel_period_pclks = VL53L0X_getVcselPulsePeriod(VcselPeriodPreRange);

  timeouts->msrc_dss_tcc_mclks = readReg(VL53L0X_I2CADDR, MSRC_CONFIG_TIMEOUT_MACROP) + 1;
  timeouts->msrc_dss_tcc_us =
    VL53L0X_timeoutMclksToMicroseconds(timeouts->msrc_dss_tcc_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  timeouts->pre_range_mclks =
    VL53L0X_decodeTimeout(readRegister_16b(VL53L0X_I2CADDR, PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI));
  timeouts->pre_range_us =
    VL53L0X_timeoutMclksToMicroseconds(timeouts->pre_range_mclks,
                               timeouts->pre_range_vcsel_period_pclks);

  timeouts->final_range_vcsel_period_pclks = VL53L0X_getVcselPulsePeriod(VcselPeriodFinalRange);

  timeouts->final_range_mclks =
    VL53L0X_decodeTimeout(readRegister_16b(VL53L0X_I2CADDR, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI));

  if (enables->pre_range) {
    timeouts->final_range_mclks -= timeouts->pre_range_mclks;
  }

  timeouts->final_range_us =
    VL53L0X_timeoutMclksToMicroseconds(timeouts->final_range_mclks,
                               timeouts->final_range_vcsel_period_pclks);
}

// Get the VCSEL pulse period in PCLKs for the given period type.
// based on VL53L0X_get_vcsel_pulse_period()
uint8_t VL53L0X_getVcselPulsePeriod(vcselPeriodType type) {
  if (type == VcselPeriodPreRange) {
    return decodeVcselPeriod(readReg(VL53L0X_I2CADDR, PRE_RANGE_CONFIG_VCSEL_PERIOD));
  }
  else if (type == VcselPeriodFinalRange) {
    return decodeVcselPeriod(readReg(VL53L0X_I2CADDR, FINAL_RANGE_CONFIG_VCSEL_PERIOD));
  }
  else { return 255; }
}

// Convert sequence step timeout from MCLKs to microseconds with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_us()
uint32_t VL53L0X_timeoutMclksToMicroseconds(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks) {
  uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

  return ((timeout_period_mclks * macro_period_ns) + (macro_period_ns / 2)) / 1000;
}

// Convert sequence step timeout from microseconds to MCLKs with given VCSEL period in PCLKs
// based on VL53L0X_calc_timeout_mclks()
uint32_t VL53L0X_timeoutMicrosecondsToMclks(uint32_t timeout_period_us, uint8_t vcsel_period_pclks) {
  uint32_t macro_period_ns = calcMacroPeriod(vcsel_period_pclks);

  return (((timeout_period_us * 1000) + (macro_period_ns / 2)) / macro_period_ns);
}

// Decode sequence step timeout in MCLKs from register value
// based on VL53L0X_decode_timeout()
// Note: the original function returned a uint32_t, but the return value is
// always stored in a uint16_t.
uint16_t VL53L0X_decodeTimeout(uint16_t reg_val) {
  // format: "(LSByte * 2^MSByte) + 1"
  return (uint16_t)((reg_val & 0x00FF) <<
         (uint16_t)((reg_val & 0xFF00) >> 8)) + 1;
}

bool VL53L0X_setMeasurementTimingBudget(uint32_t budget_us) {
  SequenceStepEnables enables;
  SequenceStepTimeouts timeouts;

  uint16_t const StartOverhead      = 1320; // note that this is different than the value in get_
  uint16_t const EndOverhead        = 960;
  uint16_t const MsrcOverhead       = 660;
  uint16_t const TccOverhead        = 590;
  uint16_t const DssOverhead        = 690;
  uint16_t const PreRangeOverhead   = 660;
  uint16_t const FinalRangeOverhead = 550;

  uint32_t const MinTimingBudget = 20000;

  if (budget_us < MinTimingBudget) { return false; }

  uint32_t used_budget_us = StartOverhead + EndOverhead;

  VL53L0X_getSequenceStepEnables(&enables);
  VL53L0X_getSequenceStepTimeouts(&enables, &timeouts);

  if (enables.tcc) {
    used_budget_us += (timeouts.msrc_dss_tcc_us + TccOverhead);
  }

  if (enables.dss) {
    used_budget_us += 2 * (timeouts.msrc_dss_tcc_us + DssOverhead);
  }
  else if (enables.msrc) {
    used_budget_us += (timeouts.msrc_dss_tcc_us + MsrcOverhead);
  }

  if (enables.pre_range) {
    used_budget_us += (timeouts.pre_range_us + PreRangeOverhead);
  }

  if (enables.final_range) {
    used_budget_us += FinalRangeOverhead;

    // "Note that the final range timeout is determined by the timing
    // budget and the sum of all other timeouts within the sequence.
    // If there is no room for the final range timeout, then an error
    // will be set. Otherwise the remaining time will be applied to
    // the final range."

    if (used_budget_us > budget_us) {
      // "Requested timeout too big."
      return false;
    }

    uint32_t final_range_timeout_us = budget_us - used_budget_us;

    // set_sequence_step_timeout() begin
    // (SequenceStepId == VL53L0X_SEQUENCESTEP_FINAL_RANGE)

    // "For the final range timeout, the pre-range timeout
    //  must be added. To do this both final and pre-range
    //  timeouts must be expressed in macro periods MClks
    //  because they have different vcsel periods."

    uint16_t final_range_timeout_mclks =
      VL53L0X_timeoutMicrosecondsToMclks(final_range_timeout_us,
                                 timeouts.final_range_vcsel_period_pclks);

    if (enables.pre_range) {
      final_range_timeout_mclks += timeouts.pre_range_mclks;
    }

    writeRegister_16b(VL53L0X_I2CADDR, FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI,
      VL53L0X_encodeTimeout(final_range_timeout_mclks));

    // set_sequence_step_timeout() end

    measurement_timing_budget_us = budget_us; // store for internal reuse
  }
  return true;
}

// Encode sequence step timeout register value from timeout in MCLKs
// based on VL53L0X_encode_timeout()
// Note: the original function took a uint16_t, but the argument passed to it
// is always a uint16_t.
uint16_t VL53L0X_encodeTimeout(uint16_t timeout_mclks) {
  // format: "(LSByte * 2^MSByte) + 1"

  uint32_t ls_byte = 0;
  uint16_t ms_byte = 0;

  if (timeout_mclks > 0) {
    ls_byte = timeout_mclks - 1;

    while ((ls_byte & 0xFFFFFF00) > 0) {
      ls_byte >>= 1;
      ms_byte++;
    }

    return (ms_byte << 8) | (ls_byte & 0xFF);
  }
  else { return 0; }
}

// based on VL53L0X_perform_single_ref_calibration()
bool VL53L0X_performSingleRefCalibration(uint8_t vhv_init_byte) {
    writeRegister(VL53L0X_I2CADDR, SYSRANGE_START, 0x01 | vhv_init_byte); // VL53L0X_REG_SYSRANGE_MODE_START_STOP

    startTimeout();
    while ((readReg(VL53L0X_I2CADDR, RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
        if (checkTimeoutExpired()) { return false; }
    }

    if(auto_int_clr) {
        writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CLEAR, 0x01); // clear interrupt
    }

    writeRegister(VL53L0X_I2CADDR, SYSRANGE_START, 0x00);

    return true;
}

// FOR TOF READS:

// Start continuous ranging measurements. If period_ms (optional) is 0 or not
// given, continuous back-to-back mode is used (the sensor takes measurements as
// often as possible); otherwise, continuous timed mode is used, with the given
// inter-measurement period in milliseconds determining how often the sensor
// takes a measurement.
// based on VL53L0X_StartMeasurement()
void VL53L0X_startContinuous(uint32_t period_ms) {
  writeRegister(VL53L0X_I2CADDR, 0x80, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0x00, 0x00);
  writeRegister(VL53L0X_I2CADDR, 0x91, stop_variable);
  writeRegister(VL53L0X_I2CADDR, 0x00, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
  writeRegister(VL53L0X_I2CADDR, 0x80, 0x00);

  if (period_ms != 0) {
    // continuous timed mode

    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() begin

    uint16_t osc_calibrate_val = readRegister_16b(VL53L0X_I2CADDR, OSC_CALIBRATE_VAL);

    if (osc_calibrate_val != 0) {
      period_ms *= osc_calibrate_val;
    }

    writeRegister_32b(VL53L0X_I2CADDR, SYSTEM_INTERMEASUREMENT_PERIOD, period_ms);

    // VL53L0X_SetInterMeasurementPeriodMilliSeconds() end

    writeRegister(VL53L0X_I2CADDR, SYSRANGE_START, 0x04); // VL53L0X_REG_SYSRANGE_MODE_TIMED
  }
  else {
    // continuous back-to-back mode
    writeRegister(VL53L0X_I2CADDR, SYSRANGE_START, 0x02); // VL53L0X_REG_SYSRANGE_MODE_BACKTOBACK
  }
}

// Stop continuous measurements
// based on VL53L0X_StopMeasurement()
void VL53L0X_stopContinuous(void) {
  writeRegister(VL53L0X_I2CADDR, SYSRANGE_START, 0x01); // VL53L0X_REG_SYSRANGE_MODE_SINGLESHOT

  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0x00, 0x00);
  writeRegister(VL53L0X_I2CADDR, 0x91, 0x00);
  writeRegister(VL53L0X_I2CADDR, 0x00, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
}

// Returns a range reading in millimeters when continuous mode is active
// (readRangeSingleMillimeters() also calls this function after starting a
// single-shot range measurement)
uint16_t VL53L0X_readRangeContinuousMillimeters(void) {
    startTimeout();
    while ((readReg(VL53L0X_I2CADDR, RESULT_INTERRUPT_STATUS) & 0x07) == 0) {
        if (checkTimeoutExpired()) {
            did_timeout = true;
            return 65535;
        }
    }

    // assumptions: Linearity Corrective Gain is 1000 (default);
    // fractional ranging is not enabled
    uint16_t range = readRegister_16b(VL53L0X_I2CADDR, RESULT_RANGE_STATUS + 10);

    if(auto_int_clr) {
        writeRegister(VL53L0X_I2CADDR, SYSTEM_INTERRUPT_CLEAR, 0x01); // clear interrupt
    }
  
    return range;
}

// Performs a single-shot range measurement and returns the reading in
// millimeters
// based on VL53L0X_PerformSingleRangingMeasurement()
uint16_t VL53L0X_readRangeSingleMillimeters(void) {
  writeRegister(VL53L0X_I2CADDR, 0x80, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0x00, 0x00);
  writeRegister(VL53L0X_I2CADDR, 0x91, stop_variable);
  writeRegister(VL53L0X_I2CADDR, 0x00, 0x01);
  writeRegister(VL53L0X_I2CADDR, 0xFF, 0x00);
  writeRegister(VL53L0X_I2CADDR, 0x80, 0x00);

  writeRegister(VL53L0X_I2CADDR, SYSRANGE_START, 0x01);

  // "Wait until start bit has been cleared"
  startTimeout();
  while (readReg(VL53L0X_I2CADDR, SYSRANGE_START) & 0x01) {
    if (checkTimeoutExpired()) {
      did_timeout = true;
      return 65535;
    }
  }

  return VL53L0X_readRangeContinuousMillimeters();
}

// Did a timeout occur in one of the read functions since the last call to
// timeoutOccurred()?
bool VL53L0X_timeoutOccurred() {
  bool tmp = did_timeout;
  did_timeout = false;
  return tmp;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* * * * * * * * * * * * * Accelerometer Functions * * * * * * * * * * * * */
//<Nick>
void H3LIS200DL_begin()
{
    H3LIS200DL_setPowerMode(NORMAL);
    H3LIS200DL_axesEnable(true);

    uint8_t data = 0;
//    showBinary(I2C_Status<<1);
//    msTimerDelay(1000);
//    showBinary(0xff);
//    msTimerDelay(1000);
    
    uint8_t i = 0x21;
    for (i = 0x21; i < 0x25; i++) {
        writeRegister(H3LIS200DL_I2CADDR, i, data);
    }
//    showBinary(I2C_Status<<1);
//    msTimerDelay(1000);
//    showBinary(0xff);
//    msTimerDelay(1000);
    uint8_t j = 0x30;
    for (j = 0x30; j < 0x37; j++) {
      writeRegister(H3LIS200DL_I2CADDR, j, data);    
    }
//    showBinary(I2C_Status<<1);
//    msTimerDelay(1000);
//    showBinary(0xff);
//    msTimerDelay(1000);
}

void H3LIS200DL_axesEnable(bool enable)
{
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG1, &data);
  if (enable)
  {
    data |= 0x07;
  }
  else
  {
    data &= ~0x07;
  }
  writeRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG1, data);
}

void H3LIS200DL_setPowerMode(power_mode pmode)
{
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG1, &data);
  
  // The power mode is the high three bits of CTRL_REG1. The mode 
  //  constants are the appropriate bit values left shifted by five, so we 
  //  need to right shift them to make them work. We also want to mask off the
  //  top three bits to zero, and leave the others untouched, so we *only*
  //  affect the power mode bits.
  data &= ~0xe0;    // Clear the top three bits
  data |= pmode<<5; // set the top three bits to our pmode value
  writeRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG1, data); // write the new value to CTRL_REG1
}

void H3LIS200DL_setODR(data_rate drate)
{
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG1,  &data);

  // The data rate is bits 4:3 of CTRL_REG1. The data rate constants are the
  //  appropriate bit values; we need to right shift them by 3 to align them
  //  with the appropriate bits in the register. We also want to mask off the
  //  top three and bottom three bits, as those are unrelated to data rate and
  //  we want to only change the data rate.
  data &=~0x18;     // Clear the two data rate bits
  data |= drate<<3; // Set the two data rate bits appropriately.
  writeRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG1, data); // write the new value to CTRL_REG1
}

void H3LIS200DL_readAxes(int16_t* x, int16_t* y, int16_t* z)
{
  uint8_t data[6]; // create a buffer for our incoming data
  
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_X_L, &data[0]);
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_X_H, &data[1]);
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_Y_L, &data[2]);
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_Y_H, &data[3]);
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_Z_L, &data[4]);
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_Z_H, &data[5]);
  // The data that comes out is 12-bit data, left justified, so the lower
  //  four bits of the data are always zero. We need to right shift by four,
  //  then typecase the upper data to an integer type so it does a signed
  //  right shift.
  *x = data[0] | data[1] << 8;
  *y = data[2] | data[3] << 8;
  *z = data[4] | data[5] << 8;
  *x = (*x>>4) ;
  *y = (*y>>4) ;
  *z = (*z>>4) ;
}

int16_t H3LIS200DL_Read_x(int16_t x)
{
  uint8_t data[6];
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_X_L, &data[0]);
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_X_H, &data[1]);
  x = data[0] | data[1] << 8;
  x = x >> 4;
  return(x);
}

int16_t H3LIS200DL_Read_y(int16_t y)
{
  uint8_t data[6];
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_Y_L, &data[2]);
  //I2C1_MasterRead(&data[2], 2, H3LIS200DL_I2CADDR, &I2C_Status);
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_Y_H, &data[3]);
  //I2C1_MasterRead(&data[3], 2, H3LIS200DL_I2CADDR, &I2C_Status);
  y = data[2] | data[3] << 8;
  y = y >> 4;
  return(y);
}

int16_t H3LIS200DL_Read_z(int16_t z)
{
  uint8_t data[6];
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_Z_L, &data[4]);
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_OUT_Z_H, &data[5]);
  z = data[4] | data[5] << 8;
  z = z >> 4;
  return(z);
}

int16_t H3LIS200DL_convertToG(int16_t maxScale, int16_t reading)
{
    maxScale = (float)maxScale;
    reading = (float)reading;
  float result = ((maxScale * reading)/2047);
  return ((int16_t)result);
}

void H3LIS200DL_setHighPassCoeff(high_pass_cutoff_freq_cfg hpcoeff)
{
  // The HPF coeff depends on the output data rate. The cutoff frequency is
  //  is approximately fs/(6*HPc) where HPc is 8, 16, 32 or 64, corresponding
  //  to the various constants available for this parameter.
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG2,  &data);
  data &= ~0xfc;  // Clear the two low bits of the CTRL_REG2
  data |= hpcoeff;
  writeRegister(H3LIS200DL_CTRL_REG2, data, 1);
}

void H3LIS200DL_enableHPF(bool enable)
{
  // Enable the high pass filter
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG2,  &data);
  if (enable)
  {
    data |= 1<<5;
  }
  else
  {
    data &= ~(1<<5);
  }
  writeRegister(H3LIS200DL_CTRL_REG2, data, 1);
}

void H3LIS200DL_HPFOnIntPin(bool enable, uint8_t pin)
{
  // Enable the hpf on signal to int pins 
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG2,  &data);
  if (enable)
  {
    if (pin == 1)
    {
      data |= 1<<3;
    }
    if (pin == 2)
    {
      data |= 1<<4;
    }
  }
  else
  {
    if (pin == 1)
    {
      data &= ~1<<3;
    }
    if (pin == 2)
    {
      data &= ~1<<4;
    }
  }
  writeRegister(H3LIS200DL_CTRL_REG2, data, 1);
}

void H3LIS200DL_intActiveHigh(bool enable)
{
  // Are the int pins active high or active low?
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG3,  &data);
  // Setting bit 7 makes int pins active low
  if (!enable)
  {
    data |= 1<<7;
  }
  else
  {
    data &= ~(1<<7);
  }
  writeRegister(H3LIS200DL_CTRL_REG3, data, 1);
}

void H3LIS200DL_intPinMode(pp_od _pinMode)
{
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG3,  &data);
  // Setting bit 6 makes int pins open drain.
  if (_pinMode == OPEN_DRAIN)
  {
    data |= 1<<6;
  }
  else
  {
    data &= ~(1<<6);
  }
  writeRegister(H3LIS200DL_CTRL_REG3, data, 1);
}

void H3LIS200DL_latchInterrupt(bool enable, uint8_t intSource)
{
  // Latch mode for interrupt. When enabled, you must read the INTx_SRC reg
  //  to clear the interrupt and make way for another.
  uint8_t data; 
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG3,  &data);
  // Enable latching by setting the appropriate bit.
  if (enable)
  {
    if (intSource == 1)
    {
      data |= 1<<2;
    }
    if (intSource == 2)
    {
      data |= 1<<5;
    }
  }
  else
  {
    if (intSource == 1)
    {
      data &= ~1<<2;
    }
    if (intSource == 2)
    {
      data &= ~1<<5;
    }
  }
  writeRegister(H3LIS200DL_CTRL_REG3, data, 1);
}

void H3LIS200DL_intSrcConfig(int_sig_src src, uint8_t pin)
{

  uint8_t data; 
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG3,  &data);
  // Enable latching by setting the appropriate bit.
  if (pin == 1)
  {
    data &= ~0xfc; // clear the low two bits of the register
    data |= src;
  }
  if (pin == 2)
  {
    data &= ~0xe7; // clear bits 4:3 of the register
    data |= src<<4;
  }
  writeRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG3, data);
}

void H3LIS200DL_setFullScale(fs_range range)
{
  uint8_t data; 
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_CTRL_REG4,  &data);
  data &= ~0xcf;
  data |= range<<4;
  writeRegister(H3LIS200DL_CTRL_REG4, data, 1);
}

bool H3LIS200DL_newXData()
{
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_STATUS_REG,  &data);
  if (data & 1<<0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool H3LIS200DL_newYData()
{
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_STATUS_REG,  &data);
  if (data & 1<<1)
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool H3LIS200DL_newZData()
{
  uint8_t data;
  readRegister(H3LIS200DL_I2CADDR, H3LIS200DL_STATUS_REG,  &data);
  if (data & 1<<2)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void H3LIS200DL_enableInterrupt(int_axis axis, trig_on_level trigLevel,
                     uint8_t interrupt, bool enable)
{
  uint8_t data, reg, mask; 
  mask = 0;
  if (interrupt == 1)
  {
    reg = H3LIS200DL_INT1_CFG;
  }
  else
  {
    reg = H3LIS200DL_INT2_CFG;
  }
  readRegister(H3LIS200DL_I2CADDR, reg,  &data);
  if (trigLevel == TRIG_ON_HIGH)
  {
    mask = 1<<1;
  }
  else
  {
    mask = 1;
  }
  if (axis == Z_AXIS) mask = mask<<4;
  if (axis == Y_AXIS) mask = mask<<2;
  if (enable)
  {
    data |= mask;
  }
  else
  {
    data &= ~mask;
  }
  writeRegister(H3LIS200DL_I2CADDR, reg, data);
}

void H3LIS200DL_setIntDuration(uint8_t duration, uint8_t intSource)
{
  if (intSource == 1)
  {
    writeRegister(H3LIS200DL_I2CADDR, H3LIS200DL_INT1_DURATION, duration);
  }
  else
  {
    writeRegister(H3LIS200DL_I2CADDR, H3LIS200DL_INT2_DURATION, duration);
  }
}

void H3LIS200DL_setIntThreshold(uint8_t threshold, uint8_t intSource)
{
  if (intSource == 1)
  {
    writeRegister(H3LIS200DL_I2CADDR, H3LIS200DL_INT1_THS, threshold);
  }
  else
  {
    writeRegister(H3LIS200DL_I2CADDR, H3LIS200DL_INT2_THS, threshold);
  }
}
//</Nick>

// returns true if one of the axes exceeds the set threshold
bool getAccelPoints(void) {
    uint8_t delay = 5;
    selectPort2(0x01, 0x03); // select Accelerometer port (Mux 1, Port 3)
    
    // <Nick>
    H3LIS200DL_begin();
    
    H3LIS200DL_readAxes(&x_1, &y_1, &z_1);
    timer1 = millis();
    msTimerDelay(delay);
    
    H3LIS200DL_readAxes(&x_2, &y_2, &z_2);
    timer2 = millis();
    msTimerDelay(delay);
    
    H3LIS200DL_readAxes(&x_3, &y_3, &z_3);
    timer3 = millis();
    msTimerDelay(delay);
    
    H3LIS200DL_readAxes(&x_4, &y_4, &z_4);
    timer4 = millis();
    msTimerDelay(delay);
    
    H3LIS200DL_readAxes(&x_5, &y_5, &z_5);
    timer5 = millis();
    msTimerDelay(delay);

    // find the maximum value of the 3 axes
    if(x_3 > y_3) {
        if(x_3 > z_3) {
            max = x_3;
        }
        else {
            max = z_3;
        }
    }
    else {
        if(y_3 > z_3) {
            max = y_3;
        }
        else {
            max = z_3;
        }
    }
    // </Nick>
    
    // if the maximum value is at or above the preset threshold, return true
    return (max >= thresh);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
// write a .CSV template to the SD card
// pass SD status and File Write status variables for debugging purposes
void writeTemplateToSD(void) {
    uint8_t SD_status;
    uint8_t FW_status;
    FATFS drive;        // Work area (filesystem object) for logical drive
    FIL file;           // File to write
    UINT actualLength;  // Actual length of
    char data0[] = "X, Y, Z, T\r\n";
    char data1[] = "x1, y1, z1, t1\r\n";
    char data2[] = "x2, y2, z2, t2\r\n";
    char filename[] = "DUMMY.CSV";

    msTimerDelay(5);
    if( SD_SPI_IsMediaPresent() == false) {
        return;
    }
    SD_status = f_mount(&drive,"0:", 1);
    if (SD_status == FR_OK) {   //mount
        if (f_open(&file, filename, FA_WRITE | FA_CREATE_NEW ) == FR_OK) { //Open or Create TEST.TXT file
            FW_status = f_write(&file, data0, sizeof(data0)-1, &actualLength );    //write the first line
            FW_status = f_write(&file, data1, sizeof(data1)-1, &actualLength );
            FW_status = f_write(&file, data2, sizeof(data2)-1, &actualLength );
            f_close(&file);
        }
        f_mount(0,"0:",0);  //unmount disk
        msTimerDelay(5);
    }
}

// write a .CSV containing accelerometer data to the SD card
// pass SD status and File Write status variables for debugging purposes
void writeAccelToSD(void) {
    // <Nick>
    uint8_t SD_status;
    uint8_t FW_status;
    FATFS drive;        // Work area (filesystem object) for logical drive
    FIL file;           // File to write
    UINT actualLength;  // Actual length of
    char data0[] = "X, Y, Z, t\r\n";
    char filename[] = "ACCEL.CSV";

    // write 5 data strings in .CSV format for X, Y, and Z axes
    // with millis() timestamps to plot
    sprintf(data1, "%f, %f, %f, %f \r\n", (double)x_1, (double)y_1, (double)z_1, (double)timer1);
    sprintf(data2, "%f, %f, %f, %f \r\n", (double)x_2, (double)y_2, (double)z_2, (double)timer2);
    sprintf(data3, "%f, %f, %f, %f \r\n", (double)x_3, (double)y_3, (double)z_3, (double)timer3);
    sprintf(data4, "%f, %f, %f, %f \r\n", (double)x_4, (double)y_4, (double)z_4, (double)timer4);
    sprintf(data5, "%f, %f, %f, %f \r\n", (double)x_5, (double)y_5, (double)z_5, (double)timer5);
            
    // write the data strings to a file
    if( SD_SPI_IsMediaPresent() == false) {
        return;
    }
    SD_status = f_mount(&drive,"0:", 1);
    if (SD_status == FR_OK) {   // mount disk
        //Open or Create <filename> file
        if (f_open(&file, filename, FA_WRITE | FA_CREATE_NEW ) == FR_OK) {
            // write column headers
            FW_status = f_write(&file, data0, sizeof(data0)-1, &actualLength );
            
            // write each line of data
            FW_status = f_write(&file, data1, sizeof(data1)-1, &actualLength );
            FW_status = f_write(&file, data2, sizeof(data2)-1, &actualLength );
            FW_status = f_write(&file, data3, sizeof(data3)-1, &actualLength );
            FW_status = f_write(&file, data4, sizeof(data4)-1, &actualLength );
            FW_status = f_write(&file, data5, sizeof(data5)-1, &actualLength );
                    
            f_close(&file); // close the file
        }
        f_mount(0,"0:",0);  // unmount disk
        msTimerDelay(5);
    }
    // </Nick>
    
    showConcussion(); // 5 second LED display: RED/PURPLE/RED/PURPLE/RED
}

/**
 End of File
*/