# nopf

nopf (Numbers, OPerators, Functions) is an interactive, command-line expression evaluator.

It include some handy x86/amd64 register dumping, bit-shifting, page-to-byte conversions and other programmer centric features.

## Compiling on GNU/Linux, Solaris, MacOS
Execute `make` on the directory with Makefile.

#### Dependencies
`libreadline` (e.g., on Debian/Ubuntu apt-based distros `sudo apt-get install libreadline-dev`)

#### Build configurations
* Debug and release builds can be built using makefile switch `BUILD_TYPE=debug` or `BUILD_TYPE=release` passed to make on the command line.

## Compiling on Windows
Setup environment variables before starting a build session using:  
`%comspec% /k "<path-to-Visual-Studio>\VC\Auxiliary\Build\vcvars64.bat"`

After the environment is suitable, execute:  
`nmake /F Win_Makefile all`

#### Dependencies
* Appropriate Windows SDK
* Build Tools for Visual Studio 2019, see [Building on command-line](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019) and [NMake reference](https://docs.microsoft.com/en-us/cpp/build/reference/nmake-reference?view=vs-2019)

#### Build configurations
* Debug and release builds switches are not yet finished in `Win_Makefile`.

## Binaries
* Only a binary for Windows is provided in the source for now.

## More information
Visit the nopf homepage at http://sites.google.com/site/appnopf/
