# nopf #

nopf (Numbers, OPerators, Functions) is an interactive command-line expression evaluator.

It include some handy x86/amd64 register dumping, bit-shifting, page-to-byte conversions and other programmer centric features.

### How do I get set up? ###

* Linux/Solaris/OS X build instructions:
 Build using the provided makefile.
 
 On Windows create a Visual Studio project and add the source file and build. No extra dependencies needed.
 
* Configuration
 Debug and release builds can be built using makefile switch BUILD_TYPE=debug or BUILD_TYPE=release added to the make command line.
 
* Dependencies
 On Linux, Solaris, OS X depends on libreadline.
 Windows SDK and C++ command-line support while installing Visual Studio.
 
 Visit the nopf homepage (http://sites.google.com/site/appnopf/) for more information.
 