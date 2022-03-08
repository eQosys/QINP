# Declarations

Declarations add new names to QINP programs.
The QINP language follows the ODR (One Definition Rule).

## Overview
 - [Variables](#variables)
 - [Functions](#functions)
 - [Packs](#packs)

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

A pack is a collection of members. They are similar to structs in C++.
They can get declared and defined.
A pack that has only been declared can not be used directly to instantiate a new object, but pointers to it can.
Before accessing a member of a pack, the pack must be defined.
Packs can hold pointers to objects of the same pack type.

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