# Keywords

This page lists the keywords in the QINP language.
Keywords are special identifiers in the QINP language and cannot be used for any other purpose. The [builtin types](./builtin-types.md) are also treated as keywords.

## Overview
 - [asm](#inline-assembly)
 - [assembly](#inline-assembly)
 - [_break_](./control-flow.md#break)
 - [_continue_](./control-flow.md#continue)
 - [define](#define)
 - [_do_](./control-flow.md#do-while-loop)
 - [_elif_](./control-flow.md#if-elif-else)
 - [_else_](./control-flow.md#if-elif-else)
 - [_enum_](./declarations.md#enums)
 - [_if_](./control-flow.md#if-elif-else)
 - [import](#import)
 - [_pack_](./declarations.md#packs)
 - [pass](#pass)
 - [return](#return)
 - [_while_](./control-flow.md#while-loop)
 - [_sizeof_](./operators.md#size-of)
 - [spaces](#spaces)
 - [_static_](./declarations.md#static)
 - [_\_\_file\_\__](#file)
 - [_\_\_line\_\__](#line)

---

### Inline Assembly
Inline Assembly can marked through the `asm` and `assembly` keywords as single-line or multiline code respectively.

Variables can also be used in inline assembly. Global variables are replaced with their mangled name, local variables are replaced by their offset to the base pointer (including a +/- sign).

#### Usage

> Single-line assembly:
> ```qinp
> asm: [assembly string]
> ```

> Multiline assembly:
> ```qinp
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
asm: "add rax, [rbp $(local_var)]"
```

```qinp
assembly:
	"mov rax, 1"
	"add rax, 3"
```

Both examples are equivalent.

---

### Define

The `define` keyword is used to associate a name with a token.
The name must be an identifier. The token may be any token other than a newline
Every usage of the name in the program is replaced with the token.
The token used may also be the identifier of another defined name.

#### Usage
```qinp
define [name] [token]
```

#### Example
```qinp
define foo "bar"
```

---

### Import

QINP code files can be imported using the `import` keyword.
A single file is imported once and all other imports of the same file are ignored.

The filepaths are resolved with the compiler import-directory options.
Resolving import paths relative to the importing file is planned.

Conditional imports can be used to import a file only if the specified platform matches the compiler platform.
Possible platforms are:
 - `windows`
 - `linux`
 - `macos`

#### Usage

> Standard import:
> ```qinp
> import [file_string]
> ```

> Conditional import:
> ```qinp
> import.[platform] [file_string]
> ```

#### Example

> Standard import:
> ```qinp
> import "std.qnp"
> ```

> Conditional import:
> ```qinp
> import.linux "linux/std.qnp"
> ```

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
	print("Hello, world!")
	return		\\ This is optional
```

### Spaces

Spaces are used to group functions/variables/etc.
Symbols with the same name may exists in different spaces.
When resolving a symbol reference. The most local symbol with a matching name is used.
The preceding `::` operator is used to access a symbo from the global scope.
Spaces can be nested.

#### Usage

> Space definition:
> ```qinp
> space [name]:
> 	[body]
> ```

> Accessing a space member:
> ```qinp
> [space-name]::[member-name]
> ::[space-name]::[member-name]
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
> ::x			\\ Always accesses the global x
> foo::x		\\ In global scope or foo space: ::foo::x, in bar space: ::bar::foo::x
> bar::x		\\ In this example everywhere ::bar::x
> ::foo::x
> ::bar::x
> ```

### \_\_file\_\_

The `__file__` keyword is replaced with the canonical path of the file containing the keyword.

### \_\_line\_\_

The `__line__` keyword is replaced with the line number of the keyword.