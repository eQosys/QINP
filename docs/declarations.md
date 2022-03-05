# Declarations

Declarations add new names to QINP programs.
The QINP language follows the ODR (One Definition Rule).

## Overview
 - [Variables](#variables)
 - [Functions](#functions)

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