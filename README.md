# nopf #

nopf (Numbers, OPerators, Functions) is an interactive command-line expression evaluator.

It include some handy x86/amd64 register dumping, bit-shifting, page-to-byte conversions and other programmer centric features.

## Compiling the source code ##

### Linux/Solaris/OS X ###
Run make on the directory with the makefile.

### Linux Dependencies ###
* libreadline (e.g., on Ubuntu sudo apt-get install libreadline-dev)

### Windows ###
Create a Visual Studio project and add the source file under src/ folder

### Windows dependencies ###
* Appropriate Windows SDK
* C++ command-line support while installing Visual Studio.

### Configurations ###
* Debug and release builds can be built using makefile switch BUILD_TYPE=debug or BUILD_TYPE=release passed to make on the command line.
* On Windows use the Visual Studio debug/release project configuration

## Binaries ##
* Only a binary for Windows is provided in the Downloads section (works on 64-bit or 32-bit Windows 10). For other OSes compile it from the source code.

## More information ##
Visit the nopf homepage http://sites.google.com/site/appnopf/
 