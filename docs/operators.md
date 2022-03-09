# Operators

## Overview
 - [Operator Table](#operator-table)
 - [Operator Precedence](#precedence)
 - [Assignment](#assignment)
 - [In-/Decrement](#in-decrement)
 - [Arithmetic](#arithmetic)
 - [Logical](#logical)
 - [Comparison](#comparison)
 - [Member Access](#member-access)
 - [Function Call](#function-call)
 - [C-Style Cast](#c-style-cast)
 - [size-of](#size-of)

---

### Operator Table

| Assignment | In-/Decrement | Arithmetic | Logical  | Comparison | Member Access | Other
| ---------- | ------------- | ---------- | -------  | ---------- | ------------- | -----
| a = b      | ++a           | +a         | !a       | a == b     | a[b]          | a(...)
| a += b     | --a           | -a         | a && b   | a != b     | *a            | (a)b
| a -= b     | a++           | a + b      | a \|\| b | a < b      | &a            | sizeof(a)
| a *= b     | a--           | a - b      |          | a > b      | a.b           |
| a /= b     |               | a * b      |          | a <= b     | a->b          |
| a %= b     |               | a / b      |          | a >= b     |               |
| a &= b     |               | a % b      |          |            |               |
| a \|= b    |               | ~a         |          |            |               |
| a ^= b     |               | a & b      |          |            |               |
| a <<= b    |               | a \| b     |          |            |               |
| a >>= b    |               | a ^ b      |          |            |               |
|            |               | a << b     |          |            |               |
|            |               | a >> b     |          |            |               |

---

### Operator Precedence

Operators with a lower precedence are evaluated first.

| Precedence | Operator                                                                       | Associativity
| ---------- | ------------------------------------------------------------------------------ | -------------
| 0          | `a.b` `a->b`                                                                   | left-to-right
| 1          | `a++` `a--` `a()` `a[]`                                                        | left-to-right
| 2          | `++a` `--a` `+a` `-a` `!a` `~a` `(a)b` `*a` `&a`                               | right-to-left
| 3          | `a*b` `a/b` `a%b`                                                              | left-to-right
| 4          | `a+b` `a-b`                                                                    | left-to-right
| 5          | `a<<b` `a>>b`                                                                  | left-to-right
| 6          | `a<b` `a<=b` `a>b` `a>=b`                                                      | left-to-right
| 7          | `a==b` `a!=b`                                                                  | left-to-right
| 8          | `a&b`                                                                          | left-to-right
| 9          | `a^b`                                                                          | left-to-right
| 10         | `a\|b`                                                                         | left-to-right
| 11         | `a&&b`                                                                         | left-to-right
| 12         | `a\|\|b`                                                                       | left-to-right
| 13         | `a=b` `a+=b` `a-=b` `a*=b` `a/=b` `a%=b` `a<<=b` `a>>=b` `a&=b` `a^=b` `a\|=b` | right-to-left

---

### Assignment

The assignment operators are used to assign a value (second operand) to an lvalue (first operand).
While the direct assignment operator `=` is used to assign a value to a variable, the other assignment operators modify the value of the first operand.
If the datatypes of the operands don't match, the value of the second operand is converted to the datatype of the first operand.

---

### In-/Decrement

The in-/decrement operators are used to increment or decrement a variable and can only be used on lvalues.

The prefix in-/decrement operators in-/decrement the operand and return the new value as an lvalue.

The suffix in-/decrement operators in-/decrement the operand and return the old value as an rvalue.

---

### Arithmetic

The arithmetic operators are used to perform arithmetic operations on operands.
Before the arithmetic operators are applied, the operands are converted to the same type (following the [type precedence](./expressions.md#type-precedence)).

---

### Logical

Logical operators are used to perform logical operations on operands.
They always result in a boolean value.
Similar to arithmetic operators, the operands are converted to the same type (following the [type precedence](./expressions.md#type-precedence)).

---

### Comparison

Comparison operators are used to compare two operands.
They always result in a boolean value.
Similar to arithmetic operators, the operands are converted to the same type (following the [type precedence](./expressions.md#type-precedence)).

---

### Member Access

The subscript and dereference operators can be used on lvalues and rvalues and return an lvalue.

The address-of operator can only be used on lvalues and returns the address of the lvalue.

The `.` operator can be used on lvalues and xvalues. It is used to access the members of a pack. The returned value is an lvalue.
The `->` operator can be used on lvalues and xvalues. It is used to access the members of a pointer to a pack. The returned value is an lvalue.

---

### Function Call

Function calls are used to call a function with a set of parameters.
The parameters are passed to the function in the order they are specified in the function definition.
The parameters in the function call are evaluated right-to-left.
The parameters may be implicitly converted to the types the function expects.
If the function is overloaded, and multiple overloads are applicable with implicit conversions, the function call is ambiguous and the compiler will report an error. To avoid this, use explicit conversions.

---

### C-Style Cast

The C-style cast operator can be used to convert a value to another type.
The value is converted to the target type and the result is returned as an rvalue.
It can convert between any pointer types.

---

### Size-Of

The `sizeof` operator returns the size of the resulting value of an expression as a literal of type `u64`.
The expression inside the `sizeof` operator is not evaluated.
When using a function call, the size of the return value is used.
For function addresses, the size of the function pointer is returned (usually 8 bytes)