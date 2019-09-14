# nopf #

nopf (Numbers, OPerators, Functions) is an interactive command-line expression evaluator.

It include some handy x86/amd64 register dumping, bit-shifting, page-to-byte conversions and other programmer centric features.

## Compiling the source code ##

### Linux/Solaris/OS X ###
Run `make` on the directory with the makefile.

### Linux Dependencies ###
* `libreadline` (e.g., on Debian/Ubuntu apt-based distros `sudo apt-get install libreadline-dev`)

### Windows ###
* To setup the environment before starting a build, execute:
  `%comspec% /k "<path-to-Visual-Studio>\VC\Auxiliary\Build\vcvars64.bat"`
* To build nopf, execute:
  `nmake /F Win_Makefile all`

### Windows dependencies ###
* Appropriate Windows SDK
* Build Tools for Visual Studio 2019:
  See: https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019
  See: https://docs.microsoft.com/en-us/cpp/build/reference/nmake-reference?view=vs-2019

### Configurations ###
* Debug and release builds can be built using makefile switch `BUILD_TYPE=debug` or `BUILD_TYPE=release` passed to make on the command line.
* On Windows use the Visual Studio debug/release project configuration

## Binaries ##
* Only a binary for Windows is provided in the Downloads section (works on 64-bit or 32-bit Windows 10). For other OSes compile it from the source code.

## More information ##
Visit the nopf homepage http://sites.google.com/site/appnopf/
 