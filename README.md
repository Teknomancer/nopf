# nopf #

nopf (Numbers, OPerators, Functions) is an interactive command-line expression evaluator.

It include some handy x86/amd64 register dumping, bit-shifting, page-to-byte conversions and other programmer centric features.

## Compiling sources ##

### Linux/Solaris/OS X build instructions ###
Build using the provided makefile.

### Linux Dependencies ###
Requires libreadline (e.g., on Ubuntu sudo apt-get install libreadline-dev)

### Windows build instructions ###
Create a Visual Studio project and add the source file under src/ folder

### Windows dependencies ###
* Appropriate Windows SDK
* C++ command-line support while installing Visual Studio.

### Configurations ###
* Debug and release builds can be built using makefile switch BUILD_TYPE=debug or BUILD_TYPE=release added to the make command line.
* On Windows use the Visual Studio configuration to toggle debug or release builds.

## Binaries ##
* Only a binary for Windows is provided in the Downloads section (works on 64-bit or 32-bit Windows 10).

## More information ##
Visit the nopf homepage http://sites.google.com/site/appnopf/
 