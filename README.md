# nopf

nopf (**N**umbers, **Op**erators, **F**unctions) is an interactive, command-line expression evaluator/calculator.

## Features:
* Performs separate integer and floating-point math for every operation.
* Interactive mode with colored output.
* Auto-completion on \*nix platforms that support `libreadline` in interactive mode.
* Support for user-defined constants.
* Support for input and output in hex, oct, decimal and binary.
* Bit shifts, alignment and other bitwise operations.
* Pretty printing of some x86/amd64 registers with description of bits.
* Unit of measure conversions for common units like pages to bytes, gigabits to bits etc.
* Basic statistics like sum, avg, lcd, gcd.

## Examples:
```
> 1 + 2 * (10 - 4)
Bool:         true (N)
Dec :           13 (U32)                       13 (U64)  13 (N)
Hex :   0x0000000d (U32)       0x000000000000000d (U64)  0xd (N)
Oct :    000000015 (U32)        00000000000000015 (U64)  015 (N)
Bin : 1101 (4)

> avg(10, 12, 14, 0x18 + 0x120)
Bool:         true (N)
Dec :           87 (U32)                       87 (U64)  87 (N)
Hex :   0x00000057 (U32)       0x0000000000000057 (U64)  0x57 (N)
Oct :    000000127 (U32)        00000000000000127 (U64)  0127 (N)
Bin : 101 0111 (7)

> myvar = 60
Stored variable: 'myvar'

> min2sec(myvar)
Bool:         true (N)
Dec :         3600 (U32)                     3600 (U64)  3600 (N)
Hex :   0x00000e10 (U32)       0x0000000000000e10 (U64)  0xe10 (N)
Oct :    000007020 (U32)        00000000000007020 (U64)  07020 (N)
Bin : 1110 0001 0000 (12)

> efer 0xd01
efer:
  0000 0000 0000 0000 0000 1101 0000 0001
                       │││ ││ │         │
                       │││ ││ │         └─── SCE ( 0) *
                       │││ ││ └───────────── LME ( 8) *
                       │││ │└─────────────── LMA (10) *
                       │││ └──────────────── NXE (11) *
                       ││└────────────────── SVME (12)
                       │└─────────────────── LMSLE (13)
                       └──────────────────── FFXSR (14)
```
## Downloads
Download the Windows binary from [here](https://sites.google.com/site/appnopf/downloads).

For all other platforms, you will need to compile the sources.

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

Note: The `all` switch in the above commands does a full-rebuild everytime (clean + build). For incremental builds, you may skip `all`.

#### Dependencies
* Appropriate Windows SDK
* Build Tools for Visual Studio 2019, see [Building on command-line](https://docs.microsoft.com/en-us/cpp/build/building-on-the-command-line?view=vs-2019) and [NMake reference](https://docs.microsoft.com/en-us/cpp/build/reference/nmake-reference?view=vs-2019)

#### Build configurations
* Debug and release builds can be built by adding `BUILD_TYPE=debug` or `BUILD_TYPE=release` to `nmake.exe` on the command line.

## More information
Visit the nopf homepage at https://sites.google.com/site/appnopf/
