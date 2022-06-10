# Basic Concepts

This page views the basic concepts of the QINP language.

## Overview
 - [Source Code](#source-code)
 - [Comments](#comments)
 - [Indentation](#indentation)
 - [Identifiers](#identifiers)
 - [Line breaks](#line-breaks)

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

#### Example
```qinp
u32 a = 3 * 5	\\ This is a comment
```

---

### Indentation

The indentation of statements/expressions defines the grouping and nesting.
Both tabs and spaces can be used, but they can't be mixed.
The number of tabs/spaces used for one level of indentation is defined by the first time an indentation is encountered.

The indentation is file specific. Different files can have different indentations.

The colon is almost always used to indicate a new scope/deeper indentation level with the beginning of the following line. In these cases it is possible to write a statement directly after it, without the need for a newline.

#### Example
```qinp
u64 a = 3
u64 b = 5
if a < b: std.print("a is smaller than b")
	std.print("I am still part of the if clause!")
else: std.print("a is bigger than b")
std.print("I am not part of the else clause!") \\ Indentation matters!
```

---

### Identifiers

Identifiers are used to name variables, functions, packs, etc.

Valid characters for an identifier are `a-z, A-Z, 0-9, _`, but the first character must be a letter or an underscore.

Identifiers are case sensitive.

### Line breaks

Newlines and indentations are important to the compiler.
It is possible to split a line across multiple lines by using a backslash ('\\') at the end of a line.
The indentation of the next line after the backslash is ignored.

#### Example

```qinp
std.print(\
	"Hello, world!\n" \
)
```