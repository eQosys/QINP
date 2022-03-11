# Declarations

Declarations add new names to QINP programs.
The QINP language follows the ODR (One Definition Rule).

## Overview
 - [Variables](#variables)
 - [Functions](#functions)
 - [Packs](#packs)
 - [Static](#static)

---

### Variables
Every variable declaration is a definition, variables have a type and a name.

Variables can be initialized with an expression.

Arrays can be declared with a size specified. The size must be a literal integer.

Multidimensional arrays are supported.
Arrays can not be initialized.

#### Usage

> Simple Declaration/Definition
>
> > `datatype` `name`

> Simple Declaration/Definition with initialization
>
> > `datatype` `name` = `expression`

> Array Declaration/Definition
>
> > `datatype` `name`[`size`]

#### Examples

> Declaration/Definition
> ```qinp
> u32* foo
> ```

> Declaration/Definition with initialization
> ```qinp
> u32 bar = 100
> ```

> Array Declaration/Definition
> ```qinp
> u16 baz[10]
> u64 qux[10][8]
> ```

---

### Functions

A function declaration associates a name with a return type and a list of zero or more parameters.
A function definition additionally associates a list of statements.

#### Examples

> Declaration
> ```qinp
> u8 foo(u32 a, u16* b)...
> ```

> Definition
> ```qinp
> u64 square(u64 x):
>    return x * x
> ```

---

### Packs

A pack is a collection of variables (called members). They are similar to structs in C++.
They can be declared and defined.
A pack that has only been declared can not be used directly to instantiate a new object, but pointers to an declared-only pack can be used.
Before accessing any member of a pack, the pack must be defined.
Packs can hold pointers to objects of the same pack type.
The size of a pack is constant and only known after the pack's definition. The [sizeof operator](./operators.md#sizeof) can be used on defined packs, but not on declared-only ones.

#### Examples

> Declaration
> ```qinp
> pack User...
> ```

> Definition
> ```qinp
> pack User:
>    u32 age
>    u8 name[32]
>    User* friends[10]
> ```

---

### Static

The `static` keyword can be used to declare a variable in a function as static, which means that its state is preserved between function calls (like a global variable), without being visible outside the function. When defining a static variable, the initializer is only run once (with the first call of the function).

#### Example
```qinp
void foo():
	static u32 x
	print(++x)
```