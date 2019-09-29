# nopf

nopf (**N**umbers, **Op**erators, **F**unctions) is an interactive, command-line expression evaluator/calculator.

It has some handy x86/amd64 register dumping, bit-shifting, page-to-byte conversions and other programmer centric features.

## Compiling on GNU/Linux, Solaris, MacOS
Execute `make` on the directory with Makefile.

#### Dependencies
* `libreadline` (e.g., on Debian/Ubuntu apt-based distros `sudo apt-get install libreadline-dev`)

#### Build configurations
* Debug and release builds can be built by adding `BUILD_TYPE=debug` or `BUILD_TYPE=release` to `make` on the command line.

## Compiling on Windows
Setup the required environment by executing (for 32-bit host, use `vcvars32.bat` in the command below):  
`%comspec% /k "<path-to-Visual-Studio>\VC\Auxiliary\Build\vcvars64.bat"`

Once environment is setup, create a release build by executing:  
`nmake.exe /F Makefile.vc all`

Note: The `all` switch in the above commands does a full-rebuild everytime (clean + build). For incremental builds, skip `all`.

#### Dependencies
* Appropriate Windows SDK
* Build Tools for Visual Studio 2019, see [Building on command-line](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019) and [NMake reference](https://docs.microsoft.com/en-us/cpp/build/reference/nmake-reference?view=vs-2019)

#### Build configurations
* Debug and release builds can be built by adding `BUILD_TYPE=debug` or `BUILD_TYPE=release` to `nmake.exe` on the command line.

## Binaries
Download Windows binary [here](https://sites.google.com/site/appnopf/downloads).

## More information
Visit the nopf homepage at https://sites.google.com/site/appnopf/
