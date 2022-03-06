# Builtin Types

This page lists the builtin types in the QINP language.

## Overview
 - [bool](#boolean)
 - [i8, i16, i32, i64](#signed-integers)
 - [u8, u16, u32, u64](#unsigned-integers)
 - [void](#void)

---

### Boolean

Variables of type `bool` may hold one of two values as listed below.
The size of a `bool` is `1` byte.

#### Values
 - `true`
 - `false`

---

### Signed Integers
All 'i*' types are signed integers. The number in the name stands for the number of bits used to represent the type.

Type | Size (in bytes) | Min  | Max
---- | --------------- | ---  | ------
i8   | 1               | 2^7  | 2^7-1
i16  | 2               | 2^15 | 2^15-1
i32  | 4               | 2^31 | 2^31-1
i64  | 8               | 2^63 | 2^63-1

---

### Unsigned Integers
All `u*` types are unsigned integers. The number in the name stands for the number of bits used to represent the value.

Type | Size (in bytes) | Min | Max
---- | --------------- | --- | ------
u8   | 1               | 0   | 2^8-1
u16  | 2               | 0   | 2^16-1
u32  | 4               | 0   | 2^32-1
u64  | 8               | 0   | 2^64-1

---

### Void
The `void` type cannot be used to declare a variable.
When used as the return type of a function the function will return nothing.
The `void` type has no size, but pointers to `void` are allowed.