# Compiler

## Overview
 - [Usage](#usage)
 - [Commandline Arguments](#commandline-arguments)

---

### Usage

```bash
qinp [options] <input_file>
```

---

### Commandline Arguments

 - -h, --help
 
   Prints the help message.

 - -v, --verbose

   Prints verbose output.

 - -i, --import \[path\]

   Specifies an import directory.
  
 - -o, --output \[path\]

   Specifies the output path of the generated program.

 - -k, --keep

   Keeps the generated assembly file.

 - -r, --run

   Runs the generated program.

 - -p, --platform \[platform\]
 
   Specifies the target platform. (linux, windows, macos)  
   Only linux and windows are supported for now.

 - -a, --runarg \[arg\]
  
    Specifies a single argument to pass to the generated program.  
    Only used when --run is specified.

 - -x, --extern \[name\]
  
    Specifies a library/object file to link against.

 - -e, --export-symbol-info

    Writes the symbols (including unused ones) of the parsed program code
    and additional info to the specified file.

 - -c, --export-comments
      
    Writes the comments of the parsed program code to the specified file.