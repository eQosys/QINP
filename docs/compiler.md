# Compiler

## Overview
 - [Usage](#usage)
 - [Commandline Arguments](#commandline-arguments)

---

### Usage

```bash
qinp [options] [input_files]
```

---

### Commandline Arguments

 - -h, --help
 
   Prints the help message.

 - -v, --verbose

   Prints verbose output.

 - -i, --import-dir \[path\]

   Specifies an import directory.
  
 - -o, --output-file \[path\]

   Specifies the output path of the generated program.

 - -k, --keep-intermediate

   Keeps the generated assembly file.

 - -r, --run

   Runs the generated program.

 - -A, --architecture \[arch\]

   Specifies the target architecture. (x86, qipu)  
   Only x86 is supported for now.

 - -P, --platform \[platform\]
 
   Specifies the target platform. (linux, windows, macos)  
   Only linux and windows are supported for now.

 - -a, --runarg \[arg\]
  
    Specifies a single argument to pass to the generated program.  
    Only used when --run is specified.

 - -x, --extern-library \[path\]
  
    Specifies a library/object file to link against.

 - -e, --export-symbol-info \[path\]

    Writes the symbols (including unused ones) of the parsed program code
    and additional info to the specified file.

 - -c, --export-comments \[path\]
      
    Writes the comments of the parsed program code to the specified file.