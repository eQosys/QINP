# Expressions

This page describes the syntax of QINP expressions.
An expression is a sequence of operators and their corresponding operands.

Expressions have an associated type, which they 'return' when evaluated, a value category (rvalue, lvalue) and have a precedence, which determines the order in which they are evaluated compared to other/sub-expressions.

## Overview

 - [Operators](#operators)
 - [Conversions](#conversions)
 - [Literals](#literals)

---

### Operators

[Operators](./operators.md) are the fundamental building blocks of expressions.
They have a precedence order, which determines the order in which they are evaluated.

Expressions can be grouped using parentheses, which ensures that the expression in the parentheses is evaluated before the result is being used by the surrounding operators/expressions.

---

### Conversions

Conversions are used to convert between different types.

They may occure in two forms:
#### Implicit
> Implicit conversions are performed automatically when required.
> Not all types can be implicitly converted to each other.

#### Explicit
> Explicit conversions are performed through the [cast](./operators.md#c-style-cast) operator.
> They can be used to convert between most types that are not implicitly convertible.

#### Type Precedence
> When a conversion of two operands to the same type is required, the type with the higher precedence is used.
>
> The precedence of types is the following:
> - bool
> - u8
> - u16
> - u32
> - u64
>
> The last type in the list has the highest precedence.
>
> Example: `u8` and `u32` are converted to `u32` because `u32` has a higher precedence than `u8`.

---

### Literals

Literals are are tokens in the source code that represent a value that can be determined at compile time.

There are different types of literals:
 - Integer literals
 - Boolean literals
 - String literals