# QINP Reference

## Overview
 - [Getting Started](#getting-started)
 - [Examples](#examples)
 - [Tests](#tests)
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
 - Python

Building:
 - run `./build.sh Release` in the project root directory to build QINP. The executable can be found in `./bin/Release/`

#### Windows

Prerequisites:
 - Visual Studio ('Desktop development with C++' workflow)
 - NASM
 - Python

Note: NASM and 'vcvars64.bat' must be added to PATH.

Building:
 - Use Visual Studio's CMake integration to build QINP.

---

### Examples

```qinp
import "std.qnp"
std.print("Hello, world!")
```

The [examples](../examples/) can be a good starting point to get to know the QINP language.
The [tests](../tests/) may also be interesting to some users.

---

### Tests

The [./test.py](../test.py) script can be used to test the QINP language features.

To run all tests from the [./tests/](../tests/) directory, run `test.py run all`

For more information about what the test script can do run `test.py help`

---

### Development
The QINP language is a work in progress. It is not yet stable and may be subject to breaking changes.

Next steps (Not in order):
 - [ ] Self hosted
 - [x] Dynamic memory management
 - [ ] General optimization
 - [ ] Inline functions
 - [ ] Object oriented programming
 - [ ] Function pointers
 - [ ] Operator overloading
 - [x] Blueprints
 - [ ] Variadic blueprints
 - [ ] Floating point values/operations
 - [ ] Signal handling
 - [ ] Full Windows support
 - [x] Literal expressions
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