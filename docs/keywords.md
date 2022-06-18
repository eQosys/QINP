# Keywords

This page lists the keywords in the QINP language.
Keywords are special identifiers in the QINP language and cannot be used for any other purpose. The [builtin types](./builtin-types.md) are also treated as keywords.

## Overview
 - [_\_\_file\_\__](#file)
 - [_\_\_line\_\__](#line)
 - [alias](#alias)
 - [asm](#inline-assembly)
 - [assembly](#inline-assembly)
 - [_blueprint_](./blueprints.md)
 - [_break_](./control-flow.md#break)
 - [const](#const)
 - [_continue_](./control-flow.md#continue)
 - [defer](#defer)
 - [define](#macros)
 - [_do_](./control-flow.md#do-while-loop)
 - [_elif_](./control-flow.md#if-elif-else)
 - [_else_](./control-flow.md#if-elif-else)
 - [_enum_](./declarations.md#enums)
 - [extern](#extern)
 - [_if_](./control-flow.md#if-elif-else)
 - [import](#import)
 - [null](#null)
 - [_pack_](./declarations.md#packs)
 - [pass](#pass)
 - [return](#return)
 - [_union_](./declarations.md#unions)
 - [_while_](./control-flow.md#while-loop)
 - [_sizeof_](./operators.md#size-of)
 - [space](#spaces)
 - [_static_](./declarations.md#static)

---

### \_\_file\_\_

The `__file__` keyword is replaced with the canonical path of the file containing the keyword.

---

### \_\_line\_\_

The `__line__` keyword is replaced with the line number of the keyword.

---

### Alias

The `alias` keyword can be used give an existing symbol a new name.

Aliases and [macros](#define) are not interchangeable.

#### Usage

```qinp
alias [name] = [symbol]
```

#### Example

```qinp
alias print = std.print \\ print is now a synonym for std.print
```

---

### Inline Assembly
Inline assembly can written through the `asm` and `assembly` keywords. The former is usually used to write single line assembly code, while the latter is mostly used to write longer blocks of inline assembly, but both can be used in the same way.

Variables can also be used in inline assembly. Global variables are replaced with their mangled name, local variables are replaced by their offset to the base pointer (including a +/- sign).

#### Usage

> Single-line assembly:
> ```qinp
> asm: [assembly string]
> assembly: [assembly string]
> ```

> Multiline assembly:
> ```qinp
> asm:
>   [assembly string1]
>   [assembly string2]
>   ...
> assembly:
>   [assembly string1]
>   [assembly string2]
>   ...
> ```

> Variables:
> ```qinp
> $([name])
> ```

#### Examples

```qinp
asm: "mov rax, [$(global_var)]"
assembly: "add rax, [rbp $(local_var)]"
```

```qinp
asm:
	"mov rax, 1"
	"add rax, 3"
assembly:
	"mov rax, 1"
	"add rax, 3"
```

---

### Const

TODO

---

### Defer

The `defer` keyword is used to defer the compilation of the remaining code in the file the keyword is used in.
It can be used multiple times in a file.
The keyword can only be placed in global scope.
It's primary use is to separate declarations from implementations. (Comparable to C/C++'s Header/Source separation.)

Note: Packs whose members should be reachable from another file should be _defined_ before any `defer`s are used.

#### Usage
```qinp
defer
```

---

### Macros

The `define` keyword is used to create a named macro and associate it with a list of tokens.
The name must be an identifier. The tokens can be of any type except a newline.
Every occurence of the macro's name in the code is replaced by it's associated tokens.

Function-like macros have a 'parameter' list, which is a list of identifiers. Every occurence of the parameter's names in the macro definition is replaced by the corresponding argument.

#### Usage

> Simple macro:
> ```qinp
> define [name] [token list]
> ```
> Function-like macro
> ```qinp
> define
> ```

#### Examples

> Simple macro
> ```qinp
> define foo "bar"
> ```

> Function-like macro
> ```qinp
> define log(msg) .std.print("Log %:%: %", __line__, __file__, msg)
> \\        ^ The opening parenthesis must be directly after the name of the macro.
> \\          When a whitespace is placed between the name and the opening parenthesis,
> \\          the macro definition is treated as a simple macro instead of a function-like macro.
> ```

---

### Extern

The `extern` keyword can be used to declare functions that are part of an external library (Mainly C).
Functions marked with the `extern` keyword can used the same way as any other function.
Currently only builtin types can be used as return types and parameter types. (Pointers to packs are valid)

#### Usage

```qinp
extern [function declaration] = [export-name]
```

#### Example

```qinp
extern void __ExitProcess(i32 status) = "ExitProcess" \\ ExitProcess is part of the Windows API
```

---

### Import

QINP code files can be imported using the `import` keyword.
A single file is imported once and all other imports of the same file are ignored.

The compiler tries to resolve imports in the following order:
 1. Relative to the current file
 2. Directories specified via compiler option [`-i`](./compiler.md#commandline-arguments) (checked in the order provided)

An import may have zero or more specifiers. Their primary usage are conditional imports.

Conditional imports can be used to import a file only if the specified platform matches the compiler platform.
Possible platform values are:
 - `linux`
 - `windows`
 - `macos`

Only the first given platform is checked.

The `defer` specifier can be used to import a file after all other imports.
It can be used to mimic the behavior of C/C++'s header and source file inclusion.

#### Usage

> Standard import:
> ```qinp
> import [file_string]
> ```

> Conditional import:
> ```qinp
> import.[specifier1].... [file_string]
> ```

#### Example

> Standard import:
> ```qinp
> import "std.qnp"
> ```

> Conditional import:
> ```qinp
> import.linux "platform/linux/std.qnp"
> ```

---

### Null

The `null` keyword is used to represent 0 of any builtin type.
It can be implicitely converted to any builtin type.

#### Example

```qinp
void* p = null
u8* pu = null
i32 i = null
bool b = null
```

---

### Pass

The `pass` keyword is a no-op statement.
It's primary purpose is to be used as a placeholder for empty bodies (e.g. function body).

#### Example

```qinp
void foo():
	pass
```

---

### Return

The `return` statement is used to return (a value) from a function.

When the function it is used in has a return type other than `void`, the statement must be followed by an expression specifying the value to return.

In functions with a return type of `void`, the `return` statement is optional, otherwise the last statement in the function body must be a `return` statement.

#### Usage

```qinp
return [expression*]
```

#### Examples

```qinp
u64 square(u64 x):
	return x * x
```

```qinp
void say_hello():
	std.print("Hello, world!")
	return		\\ This is optional
```

### Spaces

Spaces are used to group functions/variables/etc.
Symbols with the same name may exists in different spaces.
When resolving a symbol reference. The most local symbol with a matching name is used.
The preceding `.` operator is used to access a symbol from the global scope.
Spaces can be nested.

#### Usage

> Space definition:
> ```qinp
> space [name]:
> 	[body]
> ```

> Accessing a space member:
> ```qinp
> [space-name].[member-name]
> .[space-name].[member-name]
> ```

#### Examples

> Space definition:
> ```qinp
> u64 x = 0
> space foo:
> 	u64 x = 1
> space bar:
> 	u64 x = 2
> 
> 	space foo:
> 		u64 x = 3
> ```

> Accessing a space member:
> ```qinp
> x			\\ Always resolves to the local x
> .x			\\ Always accesses the global x
> foo.x		\\ In global scope or foo space: .foo.x, in bar space: .bar.foo.x
> bar.x		\\ In this example everywhere .bar.x
> .foo.x
> .bar.x
> ```