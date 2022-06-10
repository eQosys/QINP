# Compiler

## Overview
 - [Commandline Arguments](#commandline-arguments)

---

### Commandline Arguments

 - -h, --help
 
   Prints the help message.

 - -v, --verbose

   Prints verbose output.

 - -i, --import=\[path\]

   Specifies an import directory.
  
 - -o, --output=\[path\]

   Specifies the output path of the generated program.

 - -k, --keep

   Keeps the generated assembly file.

 - -r, --run

   Runs the generated program.

 - -p, --platform
 
   Specifies the target platform. (linux, windows, macos)
   Only linux and windows are supported for now.

 - -a, --runarg
  
    Specifies a single argument to pass to the generated program.
    Only used when --run is specified.

 - -x, --extern
  
    Specifies a library/object file to link against.

 - -s, --print-symbols

    Prints the symbols (including unused ones) of the parsed program code.

 - -e, --export-symbol-info

    Writes the symbols (including unused ones) of the parsed program code
    and additional info to the specified file.