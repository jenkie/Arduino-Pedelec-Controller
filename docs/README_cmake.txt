If you like more control over the build process
than the Java based Arduino UI provides,
the CMAKE based build system is for you.

It invokes avr-gcc and avrdude directly.

Note: Linux only for now. Windows support is untested.

1. Setup: Create a build directory

   mkdir build
   cd build
   cmake ../

   Most likely you will have to give the Arduino install path:
   cmake -DARDUINO_INSTALL_PATH=/tmp ../

   If you have a FC before 2.x and the Arduino Nano uses
   the old bootloader (57600 baud), use this option:

   cmake cmake -DARDUINO_INSTALL_PATH=/tmp -DOLD_BOOTLOADER=ON ../

2. Compile

   cd build
   make

3. Upload

   cd build
   make upload-pcontroller SERIAL_PORT=/dev/ttyUSB0
