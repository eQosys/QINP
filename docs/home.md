# QINP Reference

## Overview
 - [Getting Started](#getting-started)
 - [Development](#development)
 - [Language](#language)
 - [Compiler](#compiler)
 - [Standard Library](#standard-library)

---

### Getting Started

#### Linux

Prerequisites:
 - CMake
 - Make
 - C++17 Compiler
 - NASM

Building:
 - run `./build.sh Release` in the project root directory to build QINP. The executable can be found in `./bin/Release/`

#### Windows

Prerequisites:
 - Visual Studio ('Desktop development with C++' workflow)
 - NASM

Note: NASM and 'vcvars64.bat' must be added to PATH.

Building:
 - Use Visual Studio's CMake integration to build QINP.

---

### Development
The QINP language is a work in progress. It is not yet stable and may be subject to breaking changes.

Next steps (Not in order):
 - [ ] Self hosted
 - [ ] Dynamic memory management
 - [ ] General optimization
 - [ ] Inline functions
 - [ ] Object oriented programming
 - [ ] Function pointers
 - [ ] Operator overloading
 - [ ] Blueprints + Varargs
 - [ ] Floating point values/operations
 - [ ] Signal handling
 - [ ] Full Windows support
 - [ ] Literal expressions
 - [ ] Exceptions

---

### Language
 - [Basic concepts](./basic-concepts.md)
 - [Keywords](./keywords.md)
 - [Builtin Types](./builtin-types.md)
 - [Expressions](./expressions.md)
 - [Declarations](./declarations.md)
 - [Overloading](./overloading.md)
 - [Control Flow](./control-flow.md)
 - [Packs](./declarations.md#packs)
 - [Blueprints - noimpl]()
 - [Exceptions - noimpl]()

---

### Compiler
 - [Compiler](./compiler.md)

---

### Standard Library

The Standard Library is a collection of functions and packs that are available to all QINP programs and are essential to the language.
For an overview of the content of the Standard Library, see the [Standard Library Reference](./stdlib/stdlib.md).