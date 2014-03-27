#!/usr/bin/python3
"""Try to configure different config.h settings"""
"""and see if everything still compiles"""
"""Licensed under the GPL v3, part of the pedelec controller"""
import os
import subprocess
from datetime import datetime

BASE_DIR = 'Arduino_Pedelec_Controller'
BUILD_PREFIX = 'compile-test-'
CPU_COUNT = 8                      # Number of CPUs for parallel make

if not os.path.isdir(BASE_DIR):
    raise Exception('Please call from base directory: docs/compile_test.py')

start_time = datetime.now()

SIMPLE_CONFIGS = [
                  'SUPPORT_SWITCH_ON_POTI_PIN',
#                  'SUPPORT_DISPLAY_BACKLIGHT',
                  'SUPPORT_BMP085',
                  'SUPPORT_DSPC01',
                  'SUPPORT_POTI',
                  'SUPPORT_SOFT_POTI',
                  'SUPPORT_POTI_SWITCHES',
                  'SUPPORT_THROTTLE',
                  'SUPPORT_PAS',
                  'SUPPORT_XCELL_RT',
                  'SUPPORT_HRMI',
                  'SUPPORT_BRAKE',
                  'SUPPORT_PROFILE_SWITCH_MENU',
                  'SUPPORT_FIRST_AID_MENU',
                  'SUPPORT_LIGHTS_SWITCH',
#                  'SUPPORT_LIGHTS_ENABLE_ON_STARTUP',
                  'SUPPORT_MOTOR_GUESS',
                  'SUPPORT_BATTERY_CHARGE_DETECTION',
                 ]

DISPLAY_TYPES = [
                'NONE',
                'NOKIA_5PIN',
                'NOKIA_4PIN',
                '16X2_LCD_4BIT',
                'KINGMETER',
                'BMS'
                ]

SERIAL_MODES = [
                'NONE',
                'DEBUG',
                'ANDROID',
                'MMC',
                'LOGVIEW',
                ]

CONTROL_MODES = [
                'NORMAL',
                'LIMIT_WH_PER_KM',
                'TORQUE'
                ]

HW_REVISIONS = [1, 2, 3, 4, 5]

# Cleanup
print('Cleaning build directories')
subprocess.call('rm -rf ' + BUILD_PREFIX + '*', shell=True)
print('')

def write_config_h(filename):
    with open(filename, 'w') as f:
        f.write('//place all your personal configurations here and keep this file when updating!\n')
        f.write('#ifndef CONFIG_H\n')
        f.write('#define CONFIG_H\n')
        f.write('\n')
        f.write('#if ARDUINO < 100\n')
        f.write('#include <WProgram.h>\n')
        f.write('#else\n')
        f.write('#include <Arduino.h>\n')
        f.write('#endif\n')
        f.write('#include "switches_action.h"\n')
        f.write('\n')
        f.write('#define HARDWARE_REV 4      //place your hardware revision (1-5) here: x means hardware-revision 1.x\n')
        f.write('\n')
        f.write('#define DISPLAY_TYPE_NONE (1<<0)             //no display at all\n')
        f.write('#define DISPLAY_TYPE_NOKIA_5PIN (1<<1)       //Nokia 5110 5 pin mode\n')
        f.write('#define DISPLAY_TYPE_NOKIA_4PIN (1<<2)       //Nokia 5110 4 pin mode (SCE pin tied to GND)\n')
        f.write('#define DISPLAY_TYPE_NOKIA (DISPLAY_TYPE_NOKIA_5PIN|DISPLAY_TYPE_NOKIA_4PIN)\n')
        f.write('#define DISPLAY_TYPE_16X2_LCD_4BIT (1<<3)    //16x2 LCD 4bit-mode\n')
        f.write('#define DISPLAY_TYPE_KINGMETER (1<<4)            //King-Meter J-LCD or SW-LCD\n')
        f.write('#define DISPLAY_TYPE_BMS (1<<5)              //BMS Battery S-LCD\n')
        f.write('#define DISPLAY_TYPE DISPLAY_TYPE_NOKIA_4PIN //Set your display type here. CHANGES ONLY HERE!<-----------------------------\n')
        f.write('\n')
        f.write('#define NOKIA_LCD_CONTRAST 190                   //set display contrast here. values around 190 should do the job\n')
        f.write('\n')
        f.write('#define SERIAL_MODE_NONE (1<<0)              //dont send serial data at all\n')
        f.write('#define SERIAL_MODE_DEBUG (1<<1)             //send debug data over Serial Monitor\n')
        f.write('#define SERIAL_MODE_ANDROID (1<<2)           //send Arduino Pedelec HMI compatible data over serial/bluetooth\n')
        f.write('#define SERIAL_MODE_MMC (1<<3)               //send MMC-App compatible data over serial/bluetooth (for future use, not implemented yet)\n')
        f.write('#define SERIAL_MODE_LOGVIEW (1<<4)           //send logview-compatible data over serial (for future use, not implemented yet)\n')
        f.write('#define SERIAL_MODE SERIAL_MODE_DEBUG        //Set your serial mode here. CHANGES ONLY HERE!<-----------------------------\n')
        f.write('\n')
        f.write('\n')
        f.write('// Customizable buttons for use with the on-the-go-menu.\n')
        f.write('// The menu is activated by the ACTION_ENTER_MENU action (see below).\n')
        f.write('//\n')
        f.write('// Choose from: SWITCH_THROTTLE, SWITCH_DISPLAY1 and SWITCH_DISPLAY2\n')
        f.write('//\n')
        f.write('const switch_name MENU_BUTTON_UP = SWITCH_THROTTLE;\n')
        f.write('const switch_name MENU_BUTTON_DOWN = SWITCH_DISPLAY1;\n')
        f.write('\n')
        f.write('// Switch actions: Customizable actions for short and long press\n')
        f.write('//\n')
        f.write('// Choose from: ACTION_NONE, ACTION_SET_SOFT_POTI, ACTION_SHUTDOWN_SYSTEM\n')
        f.write('//              ACTION_ENABLE_BACKLIGHT_LONG, ACTION_TOGGLE_BLUETOOTH,\n')
        f.write('//              ACTION_ENTER_MENU, ACTION_PROFILE_1, ACTION_PROFILE_2, ACTION_PROFILE\n')
        f.write('//              ACTION_TOGGLE_LIGHTS, ACTION_INCREASE_POTI, ACTION_DECREASE_POTI\n')
        f.write('//\n')
        f.write('// The file "switches_action.h" contains a list with descriptions.\n')
        f.write('//\n')
        f.write('const sw_action SW_THROTTLE_SHORT_PRESS = ACTION_SET_SOFT_POTI;\n')
        f.write('const sw_action SW_THROTTLE_LONG_PRESS  = ACTION_SHUTDOWN_SYSTEM;\n')
        f.write('\n')
        f.write('// #define SUPPORT_SWITCH_ON_POTI_PIN              //uncomment if you have an additional switch on the poti pin\n')
        f.write('const sw_action SW_POTI_SHORT_PRESS = ACTION_NONE;\n')
        f.write('const sw_action SW_POTI_LONG_PRESS = ACTION_NONE;\n')
        f.write('\n')
        f.write('const sw_action SW_DISPLAY1_SHORT_PRESS = ACTION_ENABLE_BACKLIGHT_LONG;\n')
        f.write('const sw_action SW_DISPLAY1_LONG_PRESS  = ACTION_ENTER_MENU;\n')
        f.write('\n')
        f.write('const sw_action SW_DISPLAY2_SHORT_PRESS = ACTION_NONE;\n')
        f.write('const sw_action SW_DISPLAY2_LONG_PRESS  = ACTION_ENTER_MENU;\n')
        f.write('\n')
        f.write('// #define SUPPORT_DISPLAY_BACKLIGHT // uncomment for LCD display backlight support\n')
        f.write('                                     // The Nokia 5110 display needs a 120 Ohm resistor on the backlight pin\n')
        f.write('#ifdef SUPPORT_DISPLAY_BACKLIGHT\n')
        f.write('const int display_backlight_pin = 12;   // LCD backlight. Use a free pin here, f.e. instead of display switch #2 (12, default).\n')
        f.write('#endif\n')
        f.write('\n')
        f.write('// #define SUPPORT_BMP085   //uncomment if BMP085 available (barometric pressure + temperature sensor)\n')
        f.write('// #define SUPPORT_DSPC01   //uncomment if DSPC01 available (barometric altitude + temperature sensor), connect to I2C-BUS\n')
        f.write('#define SUPPORT_POTI        //uncomment if Poti connected\n')
        f.write('// #define SUPPORT_SOFT_POTI //uncomment if Poti is emulated: The switch_disp button will store the current throttle value as poti value\n')
        f.write('\n')
        f.write('// #define SUPPORT_POTI_SWITCHES     //uncomment to increase/decrease the poti via switch action ACTION_INCREASE_POTI / ACTION_DECREASE_POTI\n')
        f.write('const int poti_level_step_size_in_watts = 50;        //number of watts to increase / decrease poti value by switch press\n')
        f.write('\n')
        f.write('#define SUPPORT_THROTTLE    //uncomment if Throttle connected\n')
        f.write('#define SUPPORT_PAS         //uncomment if PAS-sensor connected\n')
        f.write('// #define SUPPORT_XCELL_RT    //uncomment if X-CELL RT connected. FC1.4: pas_factor_min=0.2, pas_factor_max=0.5. FC1.5: pas_factor_min=0.5, pas_factor_max=1.5. pas_magnets=8\n')
        f.write('// #define SUPPORT_HRMI         //uncomment if polar heart-rate monitor interface connected to i2c port\n')
        f.write('#define SUPPORT_BRAKE        //uncomment if brake switch connected\n')
        f.write('// #define INVERT_BRAKE         //uncomment if brake signal is low when not braking\n')
        f.write('\n')
        f.write('#define SUPPORT_PROFILE_SWITCH_MENU           //uncomment to disable support for profile switching in the menu\n')
        f.write('#define SUPPORT_FIRST_AID_MENU                //uncomment if you want a on-the-go workaround menu ("Pannenhilfe")\n')
        f.write('\n')
        f.write('// Software controlled lights switch. Needs FC 1.3 or newer. Only possible if X-CELL RT is not in use as they share the A3 pin\n')
        f.write('// #define SUPPORT_LIGHTS_SWITCH                   //uncomment if you want software switchable lights on pin A3 (lights_pin)\n')
        f.write('// #define SUPPORT_LIGHTS_ENABLE_ON_STARTUP   //uncomment if you want the lights turned on when the system turns on\n')
        f.write('// #define SUPPORT_LIGHTS_SWITCH_MENU       //uncomment if you want a "Toggle lights" menu entry\n')
        f.write('\n')
        f.write('#define CONTROL_MODE_NORMAL 0            //Normal mode: poti and throttle control motor power\n')
        f.write('#define CONTROL_MODE_LIMIT_WH_PER_KM 1   //Limit wh/km consumption: poti controls wh/km, throttle controls power to override poti\n')
        f.write('#define CONTROL_MODE_TORQUE 2            //power = x*power of the biker, see also description of power_poti_max!\n')
        f.write('#define CONTROL_MODE CONTROL_MODE_NORMAL //Set your control mode here\n')
        f.write('\n')
        f.write('//#define SUPPORT_MOTOR_GUESS   //enable guess of motor drive depending on current speed. Usefull for motor controllers with speed-throttle to optimize response behaviour\n')
        f.write('#define SUPPORT_BATTERY_CHARGE_DETECTION //support detection if the battery was charged -> reset wh / trip km / mah counters if detected.\n')
        f.write('\n')
        f.write('//Config Options-----------------------------------------------------------------------------------------------------\n')
        f.write('const int pas_tolerance=1;               //0... increase to make pas sensor slower but more tolerant against speed changes\n')
        f.write('const int throttle_offset=196;           //Offset voltage of throttle control when in "0" position (0..1023 = 0..5V)\n')
        f.write('const int throttle_max=832;              //Offset voltage of throttle control when in "MAX" position (0..1023 = 0..5V)\n')
        f.write('const int poti_offset=0;                 //Offset voltage of poti when in "0" position (0..1023 = 0..5V)\n')
        f.write('const int poti_max=1023;                 //Offset voltage of poti when in "MAX" position (0..1023 = 0..5V)\n')
        f.write('const int motor_offset=50;               //Offset for throttle output where Motor starts to spin (0..255 = 0..5V)\n')
        f.write('const int motor_max=200;                 //Maximum input value for motor driver (0..255 = 0..5V)\n')
        f.write('const int spd_idle=55;                   //idle speed of motor in km/h - may be much higher than real idle speed (depending on controller)\n')
        f.write('const boolean startingaidenable = true;  //enable starting aid?\n')
        f.write('const int startingaid_speed = 6;         //starting aid up to this speed. 6km/h is the limit for legal operation of a Pedelec by EU-wide laws\n')
        f.write('const float vmax=42.0;                   //Battery voltage when fully charged\n')
        f.write('const float vcutoff=33.0;                //cutoff voltage in V;\n')
        f.write('const float vemergency_shutdown = 28.0;  //emergency power off situation to save the battery from undervoltage\n')
        f.write('const float wheel_circumference = 2.202; //wheel circumference in m\n')
        f.write('const int spd_max1=22;                   //speed cutoff start in Km/h\n')
        f.write('const int spd_max2=25;                   //speed cutoff stop (0W) in Km/h\n')
        f.write('const int power_max=500;                 //Maximum power in W (throttle mode)\n')
        f.write('const int power_poti_max=500;            //Maximum power in W (poti mode) or maximum percentage of human power drawn by motor (torque mode)\n')
        f.write('const int thermal_limit=150;             //Maximum continuous thermal load motor can withstand\n')
        f.write('const int thermal_safe_speed=12;         //Speed above which motor is thermally safe at maximum current, see EPACSim\n')
        f.write('const int whkm_max=30;                   //Maximum wh/km consumption in CONTROL_MODE_LIMIT_WH_PER_KM (controls poti-range)\n')
        f.write('const unsigned int idle_shutdown_secs = 30 * 60;           // Idle shutdown in seconds. Max is ~1080 minutes or 18 hours\n')
        f.write('const unsigned int menu_idle_timeout_secs = 60;            // Menu inactivity timeout in seconds.\n')
        f.write('const double capacity = 166.0;           //battery capacity in watthours for range calculation\n')
        f.write('const double pas_factor_min=1.2;         //Use pas_factor from hardware-test here with some tolerances. Both values have to be eihter larger or smaller than 1.0 and not 0.0!\n')
        f.write('const double pas_factor_max=3;           //Use pas_factor from hardware-test here with some tolerances. Both values have to be eihter larger or smaller than 1.0 and not 0.0!\n')
        f.write('const int pas_magnets=5;                 //number of magnets in your PAS sensor. When using a Thun X-Cell RT set this to 8\n')
        f.write('const double cfg_pid_p=0.0;              //pid p-value, default: 0.0\n')
        f.write('const double cfg_pid_i=2.0;              //pid i-value, default: 2.0\n')
        f.write('const double cfg_pid_p_throttle=0.05;    //pid p-value for throttle mode\n')
        f.write('const double cfg_pid_i_throttle=2.5;     //pid i-value for throttle mode\n')
        f.write('const byte pulse_min=150;                //lowest value of desired pulse range in bpm\n')
        f.write('const byte pulse_range=20;               //width of desired pulse range in bpm\n')
        f.write('\n')
        f.write('//Config Options for profile 2-----------------------------------------------------------------------------------------------------\n')
        f.write('const int startingaid_speed_2 = 6;\n')
        f.write('const int spd_max1_2=22;                   //speed cutoff start in Km/h\n')
        f.write('const int spd_max2_2=25;                   //speed cutoff stop (0W) in Km/h\n')
        f.write('const int power_max_2=500;                 //Maximum power in W (throttle mode)\n')
        f.write('const int power_poti_max_2=500;            //Maximum power in W (poti mode) or maximum percentage of human power drawn by motor (torque mode)\n')
        f.write('const double capacity_2 = 166.0;           //battery capacity in watthours for range calculation\n')
        f.write('\n')
        f.write('// voltage and current calibration\n')
        f.write('const float voltage_amplitude = 0.0587;       // set this value according to your own voltage-calibration. Default: 0.0587\n')
        f.write('const float voltage_offset = 0.0;             // set this value according to your own voltage-calibration. Default: 0.0\n')
        f.write('const float current_offset = 0.0;             // for Rev 1.3 ONLY! set this value according to your own current-calibration. Default: 0.0\n')
        f.write('                                              // for Rev 1.1 - 1.2 the offset is corrected by software!\n')
        f.write('const float current_amplitude_R11 = 0.0296;   // for Rev 1.1 - 1.2 set this value according to your own current-calibration. Default: 0.0296\n')
        f.write('const float current_amplitude_R13 = 0.0741;   // for Rev 1.3 set this value according to your own current-calibration. Default: 0.0741\n')
        f.write('\n')
        f.write('const char* const msg_welcome = "Welcome";\n')
        f.write('const char* const msg_shutdown = "Live long and prosper.";\n')
        f.write('const char* const msg_battery_charged = "Batt. charged! Resetting counters";\n')
        f.write('\n')
        f.write('#endif\n')

def prepare_config_h(append_str, filter_support=False):
    config_h = BASE_DIR + '/config.h'
    config_new = config_h + '.new'

    print('    Resetting config.h')
    write_config_h(config_h)
    print('');

    # TODO: Implement own filtering of config.h
    if filter_support:
        subprocess.call('grep -h -v "define SUPPORT_" ' + config_h + ' > ' + config_new, shell=True)
    else:
        subprocess.call('cp -f ' + config_h + ' ' + config_new, shell=True)

    # TODO: shell escaping
    subprocess.call('echo "' + append_str + '" >> ' + config_new, shell=True)
    os.remove(config_h)
    os.rename(config_new, config_h)

def prepare_cmake(build_name):
    test_name = BUILD_PREFIX + build_name
    os.mkdir(test_name)
    os.chdir(test_name)

    subprocess.call('cmake ../ -DDOCUMENTATION=OFF', shell=True)

def run_make(build_name):
    ret = subprocess.call('make -j' + str(CPU_COUNT), shell=True)

    # Keep config.h used for this build
    os.rename('../' + BASE_DIR + '/config.h', 'config.h')

    if ret != 0:
        raise Exception('Build failed for config option: %s' % build_name)

def test_simple_configs():
    for config_option in SIMPLE_CONFIGS:
        print('Testing config: %s' % config_option)

        prepare_config_h('#define ' + config_option, filter_support=True)

        prepare_cmake(config_option)
        run_make(config_option)

        # Cleanup
        os.chdir('..')
        print('');

def test_display_types():
    for display_type in DISPLAY_TYPES:
        full_display_name = 'DISPLAY_TYPE_' + display_type
        print('Testing display: %s' % full_display_name)

        prepare_config_h('#undef DISPLAY_TYPE\n#define DISPLAY_TYPE ' + full_display_name)
        prepare_cmake(full_display_name)
        run_make(full_display_name)

        # Cleanup
        os.chdir('..')
        print('');

# Test all serial modes
def test_serial_modes():
    for serial_mode in SERIAL_MODES:
        full_mode_name = 'SERIAL_MODE_' + serial_mode
        print('Testing serial mode: %s' % full_mode_name)

        prepare_config_h('#undef SERIAL_MODE\n#define SERIAL_MODE ' + full_mode_name)
        prepare_cmake(full_mode_name)
        run_make(full_mode_name)

        # Cleanup
        os.chdir('..')
        print('');

# Test all control modes
# TODO: Add XCELL_RT for TORQUE mode
def test_control_modes():
    for control_mode in CONTROL_MODES:
        full_mode_name = 'CONTROL_MODE_' + control_mode
        print('Testing control mode: %s' % full_mode_name)

        prepare_config_h('#undef CONTROL_MODE\n#define CONTROLE_MODE ' + full_mode_name)
        prepare_cmake(full_mode_name)
        run_make(full_mode_name)

        # Cleanup
        os.chdir('..')
        print('');

# Test hardware revisions
def test_hw_revisions():
    for hw_revision in HW_REVISIONS:
        test_name = 'HW_REV_' + str(hw_revision)
        print('Testing hardware revision: %s' % hw_revision)

        prepare_config_h('#undef HARDWARE_REV\n#define HARDWARE_REV ' + str(hw_revision))
        prepare_cmake(test_name)
        run_make(test_name)

        # Cleanup
        os.chdir('..')
        print('');

test_simple_configs()
test_display_types()
test_serial_modes()
test_control_modes()
test_hw_revisions()

# TODO: Compile clever "real life" feature combinations
# TODO: Add ability to run single 'test' -> convert to python unit test
# TODO: Test 'max' config option -> enable as much sane features as possible

print('Execution time: ' + str(datetime.now()-start_time))
