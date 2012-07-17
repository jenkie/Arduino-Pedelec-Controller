If you like more control over the build process
than the Java based Arduino UI provides,
the CMAKE based build system is for you.

It invokes avr-gcc and avrdude directly.

Note: Linux only for now. Windows support is untested.

1. Setup: Create a build directory

   mkdir build
   cd build
   cmake ../

2. Compile

   cd build
   make

3. Upload

   cd build
   make pcontroller-upload
