# Basic Concepts

This page views the basic concepts of the QINP language.

## Overview
 - [Source Code](#source-code)
 - [Comments](#comments)
 - [Indentation](#indentation)
 - [Identifiers](#identifiers)

---

### Source Code
The associated file extension for QINP source code is `.qnp`.
Multiple files can be compiled into a single executable using the [import](./keywords.md#import) statement.

There is no entry point in the QINP language. Expressions are evaluated in the order they are written at global scope. (This also applies to imported code).

---

### Comments

Comments are ignored by the compiler and therefore do not affect the generated assembly/binary code.

At the moment there are no multi-line comments.
Single-line comments are preceded by a double backslash ('\\\\').

#Example
```qinp
u32 a = 3 * 5	\\ This is a comment
```

---

### Indentation

The indentation of statements/expressions defines the grouping and nesting.
Tabs and spaces can be used, but can't be mixed.
How many tabs/spaces are used for each level of indentation is defined by the first time an indentation is encountered.

The indentation is file specific. Different files can have different indentations.

---

### Identifiers

Identifiers are used to name variables, functions, packs, etc.

Valid characters are `a-z, A-Z, 0-9, _`, but the first character must be a letter or an underscore.

Identifiers are case sensitive.