#!/usr/bin/python3
"""Try to configure different config.h settings"""
"""and see if everything still compiles"""
"""Licensed under the GPL v3, part of the pedelec controller"""
"""Written by Thomas Jarosch, (c) 2014"""
import os
import subprocess
import unittest
import multiprocessing
from datetime import datetime

# Hint: You can run a single test case f.e. with:
#       tools/compile_test.py CompileTest.test_max_config

BASE_DIR = 'Arduino_Pedelec_Controller'
BUILD_PREFIX = 'compile_'
CONFIG_H = BASE_DIR + '/config.h'
CPU_COUNT = multiprocessing.cpu_count()                      # Number of CPUs for parallel make

ALL_FEATURES = [
                  'SUPPORT_SWITCH_ON_POTI_PIN',
                  'SUPPORT_DISPLAY_BACKLIGHT',
                  'SUPPORT_BMP085',
                  'SUPPORT_DSPC01',
                  'SUPPORT_RTC',
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
                  'SUPPORT_LIGHTS_ENABLE_ON_STARTUP',
                  'SUPPORT_LIGHTS_SWITCH_MENU',
                  'SUPPORT_MOTOR_GUESS',
                  'SUPPORT_BATTERY_CHARGE_DETECTION',
                  'SUPPORT_GEAR_SHIFT',
                 ]

# List of features that's enabled by default
DEFAULT_FEATURES = [
                  'SUPPORT_POTI',
                  'SUPPORT_THROTTLE',
                  'SUPPORT_PAS',
                  'SUPPORT_BRAKE',
                  'SUPPORT_PROFILE_SWITCH_MENU',
                  'SUPPORT_FIRST_AID_MENU',
                  'SUPPORT_BATTERY_CHARGE_DETECTION',
                 ]

def write_config_h(filename=CONFIG_H,
                   hardware_rev=4,
                   display_type='NOKIA_4PIN',
                   serial_mode='DEBUG',
                   features=DEFAULT_FEATURES,
                   control_mode='NORMAL'):
    with open(filename, 'w') as f:
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
        f.write('#define HARDWARE_REV ' + str(hardware_rev) + '      //place your hardware revision (1-5) here: x means hardware-revision 1.x\n')
        f.write('\n')
        f.write('#define DISPLAY_TYPE_NONE (1<<0)             //no display at all\n')
        f.write('#define DISPLAY_TYPE_NOKIA_5PIN (1<<1)       //Nokia 5110 5 pin mode\n')
        f.write('#define DISPLAY_TYPE_NOKIA_4PIN (1<<2)       //Nokia 5110 4 pin mode (SCE pin tied to GND)\n')
        f.write('#define DISPLAY_TYPE_NOKIA (DISPLAY_TYPE_NOKIA_5PIN|DISPLAY_TYPE_NOKIA_4PIN)\n')
        f.write('#define DISPLAY_TYPE_16X2_LCD_4BIT (1<<3)    //16x2 LCD 4bit-mode\n')
        f.write('#define DISPLAY_TYPE_KINGMETER (1<<4)            //King-Meter J-LCD or SW-LCD\n')
        f.write('#define DISPLAY_TYPE_BMS (1<<5)              //BMS Battery S-LCD\n')
        f.write('#define DISPLAY_TYPE_16X2_SERIAL (1<<6)    //16x2 LCD via serial connection (New Haven display)\n')
        f.write('#define DISPLAY_TYPE_16X2 (DISPLAY_TYPE_16X2_LCD_4BIT|DISPLAY_TYPE_16X2_SERIAL)\n')
        f.write('\n')
        f.write('#define DISPLAY_TYPE DISPLAY_TYPE_' + display_type + '    //Set your display type here. CHANGES ONLY HERE!<-----------------------------\n')
        f.write('\n')
        f.write('#define NOKIA_LCD_CONTRAST 190                   //set display contrast here. values around 190 should do the job\n')
        f.write('const int serial_display_16x2_pin = 12;\n')
        f.write('\n')
        f.write('#define SERIAL_MODE_NONE (1<<0)              //dont send serial data at all\n')
        f.write('#define SERIAL_MODE_DEBUG (1<<1)             //send debug data over Serial Monitor\n')
        f.write('#define SERIAL_MODE_ANDROID (1<<2)           //send Arduino Pedelec HMI compatible data over serial/bluetooth\n')
        f.write('#define SERIAL_MODE_MMC (1<<3)               //send MMC-App compatible data over serial/bluetooth (for future use, not implemented yet)\n')
        f.write('#define SERIAL_MODE_LOGVIEW (1<<4)           //send logview-compatible data over serial (for future use, not implemented yet)\n')
        f.write('#define SERIAL_MODE SERIAL_MODE_' + serial_mode + '        //Set your serial mode here. CHANGES ONLY HERE!<-----------------------------\n')
        f.write('\n')

        # Output all enabled features
        f.write('\n')
        f.write('\n')
        f.write('// FEATURES begin\n')
        for feature in ALL_FEATURES:
            if feature not in features:
                continue
            f.write('#define ' + feature + '\n')
        f.write('// FEATURES end\n')
        f.write('\n')

        f.write('\n')
        f.write('const switch_name MENU_BUTTON_UP = SWITCH_THROTTLE;\n')
        f.write('const switch_name MENU_BUTTON_DOWN = SWITCH_DISPLAY1;\n')
        f.write('\n')
        f.write('const sw_action SW_THROTTLE_SHORT_PRESS = ACTION_SET_SOFT_POTI;\n')
        f.write('const sw_action SW_THROTTLE_LONG_PRESS  = ACTION_SHUTDOWN_SYSTEM;\n')
        f.write('\n')
        f.write('const sw_action SW_POTI_SHORT_PRESS = ACTION_NONE;\n')
        f.write('const sw_action SW_POTI_LONG_PRESS = ACTION_NONE;\n')
        f.write('\n')
        f.write('const sw_action SW_DISPLAY1_SHORT_PRESS = ACTION_ENABLE_BACKLIGHT_LONG;\n')
        f.write('const sw_action SW_DISPLAY1_LONG_PRESS  = ACTION_ENTER_MENU;\n')
        f.write('\n')
        f.write('const sw_action SW_DISPLAY2_SHORT_PRESS = ACTION_NONE;\n')
        f.write('const sw_action SW_DISPLAY2_LONG_PRESS  = ACTION_ENTER_MENU;\n')
        f.write('\n')
        f.write('#ifdef SUPPORT_DISPLAY_BACKLIGHT\n')
        f.write('const int display_backlight_pin = 12;   // LCD backlight. Use a free pin here, f.e. instead of display switch #2 (12, default).\n')
        f.write('#endif\n')
        f.write('\n')
        f.write('const int poti_level_step_size_in_watts = 50;        //number of watts to increase / decrease poti value by switch press\n')
        f.write('\n')
        f.write('#ifdef SUPPORT_GEAR_SHIFT\n')
        f.write('const byte gear_shift_pin_low_gear = 5;      //pin that connect to the low gear signal ("red" cable)\n')
        f.write('const byte gear_shift_pin_high_gear = 7;     //pin that connects to the high gear signal ("green" cable)\n')
        f.write('#endif\n')
        f.write('\n')
        f.write('#define CONTROL_MODE_NORMAL 0            //Normal mode: poti and throttle control motor power\n')
        f.write('#define CONTROL_MODE_LIMIT_WH_PER_KM 1   //Limit wh/km consumption: poti controls wh/km, throttle controls power to override poti\n')
        f.write('#define CONTROL_MODE_TORQUE 2            //power = x*power of the biker, see also description of power_poti_max!\n')
        f.write('#define CONTROL_MODE CONTROL_MODE_' + control_mode + ' //Set your control mode here\n')
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
        f.write('const char msg_welcome[] PROGMEM = "Welcome";\n')
        f.write('const char msg_shutdown[] PROGMEM = "Live long and prosper.";\n')
        f.write('\n')
        f.write('const char msg_battery_charged[] PROGMEM = "Batt. charged! Resetting counters";\n')
        f.write('const char msg_idle_shutdown[] PROGMEM = "Idle shutdown. Good night.";\n')
        f.write('const char msg_emergency_shutdown[] PROGMEM = "Battery undervoltage detected. Emergency shutdown.";\n')
        f.write('\n')
        f.write('const char msg_tempomat_reset[] PROGMEM = "Tempomat reset";\n')
        f.write('const char msg_unknown_action[] PROGMEM = "Unknown action!";\n')
        f.write('const char msg_activated[] PROGMEM = "Activated";\n')
        f.write('const char msg_deactivated[] PROGMEM = "Deactivated";\n')
        f.write('\n')
        f.write('#ifdef SUPPORT_GEAR_SHIFT\n')
        f.write('const char msg_gear_shift_low_gear[] PROGMEM = "Low gear active";\n')
        f.write('const char msg_gear_shift_high_gear[] PROGMEM = "High gear active";\n')
        f.write('const char msg_gear_shift_auto_selection[] PROGMEM = "Auto gear shift";\n')
        f.write('#endif\n')
        f.write('\n')
        f.write('#endif\n')


class CompileTest(unittest.TestCase):
    def tearDownClass():
        # Restore original config.h
        subprocess.call('git checkout ' + CONFIG_H, shell=True)

    def tearDown(self):
        if not os.path.isdir(BASE_DIR):
            os.chdir('..')
        if not os.path.isdir(BASE_DIR):
            raise Exception('Wrong directory, something is messed up: ' + os.getcwd())

    def prepare_cmake(self, build_name):
        test_name = BUILD_PREFIX + build_name
        os.mkdir(test_name)
        os.chdir(test_name)

        subprocess.call('cmake ../ -DDOCUMENTATION=OFF', shell=True)

    def run_make(self, build_name):
        ret = subprocess.call('make -j' + str(CPU_COUNT), shell=True)

        # Keep config.h used for this build
        os.rename('../' + BASE_DIR + '/config.h', 'config.h')

        if ret != 0:
            raise Exception('Build failed for config option: %s' % build_name)

    def build_firmware(self, name_hint=None, *args, **kwargs):
        # Compute firmware directory from current test case name
        full_id = self.id()
        test_name = full_id[full_id.rfind('.')+1:]
        if name_hint:
            test_name = test_name + '_' + name_hint

        print('Building firmware: ' + test_name)

        # pass on all arguments to write_config_h
        write_config_h(*args, **kwargs)

        try:
            self.prepare_cmake(test_name)
            self.run_make(test_name)
        finally:
            # Cleanup
            os.chdir('..')
            print('')

    def test_single_feature(self):
        for feature in ALL_FEATURES:
            self.build_firmware(feature, features=[feature])

    def test_display_types(self):
        for disp_type in ['NONE', 'NOKIA_5PIN', 'NOKIA_4PIN', '16X2_LCD_4BIT', '16X2_SERIAL', 'KINGMETER', 'BMS']:
            self.build_firmware(disp_type, display_type=disp_type)

    def test_16x2_serial_with_dynamic_backlight(self):
        my_features = DEFAULT_FEATURES
        my_features.append('SUPPORT_DISPLAY_BACKLIGHT')

        self.build_firmware(hardware_rev = 5,
                    display_type='16X2_SERIAL',
                    features=my_features)

    def test_serial_modes(self):
        for serial_mode in ['NONE', 'DEBUG', 'ANDROID', 'MMC', 'LOGVIEW']:
            self.build_firmware(serial_mode, serial_mode=serial_mode)

    def test_control_modes(self):
        for control_mode in ['NORMAL', 'LIMIT_WH_PER_KM', 'TORQUE']:
            # Enable XCELL_RT support in TORQUE mode
            my_features = DEFAULT_FEATURES
            if control_mode == 'TORQUE':
                my_features.append('SUPPORT_XCELL_RT')

            self.build_firmware(control_mode, control_mode=control_mode, features=my_features)

    def test_hw_revisions(self):
        for hw_revision in [1, 2, 3, 4, 5]:
            self.build_firmware(str(hw_revision), hardware_rev = hw_revision)

    def test_max_config(self):
        self.build_firmware(hardware_rev = 5,
                    display_type='NOKIA_4PIN',
                    serial_mode='DEBUG',
                    control_mode='TORQUE',
                    features=[
                            'SUPPORT_DISPLAY_BACKLIGHT',
                            'SUPPORT_BMP085',
                            'SUPPORT_RTC',
                            'SUPPORT_SOFT_POTI',
                            'SUPPORT_POTI_SWITCHES',
                            'SUPPORT_THROTTLE',
                            'SUPPORT_PAS',
                            'SUPPORT_XCELL_RT',
                            'SUPPORT_HRMI',
                            'SUPPORT_BRAKE',
                            'SUPPORT_PROFILE_SWITCH_MENU',
                            'SUPPORT_FIRST_AID_MENU',
                            'SUPPORT_MOTOR_GUESS',
                            'SUPPORT_BATTERY_CHARGE_DETECTION',
                            'SUPPORT_GEAR_SHIFT'
                        ]
                    )

    def test_minimal_config(self):
        self.build_firmware(hardware_rev = 5,
                    display_type='NONE',
                    serial_mode='NONE',
                    features=[]
                    )

if __name__ == '__main__':
    if not os.path.isdir(BASE_DIR):
        raise Exception('Please call from base directory: tools/compile_test.py')

    # Cleanup
    print('Cleaning build directories')
    subprocess.call('rm -rf ' + BUILD_PREFIX + '*', shell=True)
    print('')

    unittest.main()
