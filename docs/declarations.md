# Declarations

Declarations add new names to QINP programs.
The QINP language follows the ODR (One Definition Rule).

## Overview
 - [Variables](#variables)
 - [Functions](#functions)

---

### Variables
Every variable declaration is a definition, they have a type and a name.

Variables can be initialized with an expression.

#### Usage

> Declaration/Definition
> ```qinp
> [datatype] [name]
> ```

> Declaration/Definition with initialization
> ```qinp
> [datatype] [name] = [expression]
> ```

#### Examples

> Declaration/Definition
> ```qinp
> u32* foo
> ```

> Declaration/Definition with initialization
> ```qinp
> u32 bar = 100
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