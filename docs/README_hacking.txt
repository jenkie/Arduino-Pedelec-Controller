*** Determine code size (linux only) ***
nm --demangle --print-size --size-sort --reverse-sort -t d pcontroller.elf |less

*** Interactive serial interface on linux ***
socat - /dev/ttyUSB0,b115200,echo=0
