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

def prepare_config_h(append_str, filter_support=False):
    config_h = BASE_DIR + '/config.h'
    config_new = config_h + '.new'

    print('    Resetting config.h')
    subprocess.call('git checkout ' + config_h, shell=True)
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
