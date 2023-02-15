
# Stdlib - stdlib/math.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [fn<T> std.abs(T a) nodiscard](#ref_a888d6b77a69edc63ed04ce47bdea11d)
 - [fn<T> std.max(T a, T b) nodiscard](#ref_589cdc5e408248395aa27a997c2c2a35)
 - [fn<T> std.max(T a, ...) nodiscard](#ref_68a81e36ca36227b720bdec1bac1349d)
 - [fn<T> std.min(T a, T b) nodiscard](#ref_188d9420976ab5fd9fd621ca6893a3d7)
 - [fn<T> std.min(T a, ...) nodiscard](#ref_2d47073c13327b4a0c488ce97397fc11)
 - [fn<T> std.pow(T base, T exponent) nodiscard](#ref_51ee682c4f39f3ad4cad92d1b9849176)
 - [fn<T> std.roundDownTo(T val, T multiple) nodiscard](#ref_97e393840853c3d2f74624f38f522fcd)
 - [fn<T> std.roundUpTo(T val, T multiple) nodiscard](#ref_d89f62ccc98e5116d4c71fcbdfab8c30)
 - [fn<T> std.sign(T a) nodiscard](#ref_d88da5e79c70fbccd92ecddc96633f59)

## Details
#### <a id="ref_a888d6b77a69edc63ed04ce47bdea11d"/>fn<T> std.abs(T a) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L32) | [Definition](/stdlib/math.qnp?plain=1#L71)
```qinp
\\ Returns the absolute value of a value
\\ @param a The value
\\ @return The absolute value of the value
```
#### <a id="ref_589cdc5e408248395aa27a997c2c2a35"/>fn<T> std.max(T a, T b) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L21) | [Definition](/stdlib/math.qnp?plain=1#L65)
```qinp
\\ Returns the larger of two values
\\ @param a The first value
\\ @param b The second value
\\ @return The larger of the two values
```
#### <a id="ref_68a81e36ca36227b720bdec1bac1349d"/>fn<T> std.max(T a, ...) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L27) | [Definition](/stdlib/math.qnp?plain=1#L68)
```qinp
\\ Returns the largest of two or more values
\\ @param a The first value
\\ @param ... The other values
\\ @return The largest of the values
```
#### <a id="ref_188d9420976ab5fd9fd621ca6893a3d7"/>fn<T> std.min(T a, T b) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L9) | [Definition](/stdlib/math.qnp?plain=1#L59)
```qinp
\\ Returns the smaller of two values
\\ @param a The first value
\\ @param b The second value
\\ @return The smaller of the two values
```
#### <a id="ref_2d47073c13327b4a0c488ce97397fc11"/>fn<T> std.min(T a, ...) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L15) | [Definition](/stdlib/math.qnp?plain=1#L62)
```qinp
\\ Returns the smallest of two or more values
\\ @param a The first value
\\ @param ... The other values
\\ @return The smallest of the values
```
#### <a id="ref_51ee682c4f39f3ad4cad92d1b9849176"/>fn<T> std.pow(T base, T exponent) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L43) | [Definition](/stdlib/math.qnp?plain=1#L77)
```qinp
\\ Returns the value of base raised to the power of exponent.
\\ @param base The base
\\ @param exponent The exponent (must be greater than or equal to 0)
\\ @return The value of base raised to the power of exponent
```
#### <a id="ref_97e393840853c3d2f74624f38f522fcd"/>fn<T> std.roundDownTo(T val, T multiple) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L49) | [Definition](/stdlib/math.qnp?plain=1#L83)
```qinp
\\ Rounds a value down to the nearest multiple of another value
\\ @param val The value to round down
\\ @param multiple The multiple to round down to
\\ @return The value rounded down to the nearest multiple of another value, or the value itself if it is already a multiple of the other value
```
#### <a id="ref_d89f62ccc98e5116d4c71fcbdfab8c30"/>fn<T> std.roundUpTo(T val, T multiple) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L55) | [Definition](/stdlib/math.qnp?plain=1#L86)
```qinp
\\ Rounds a value up to the nearest multiple of another value
\\ @param val The value to round up
\\ @param multiple The multiple to round up to
\\ @return The value rounded up to the nearest multiple of another value, or the value itself if it is already a multiple of the other value
```
#### <a id="ref_d88da5e79c70fbccd92ecddc96633f59"/>fn<T> std.sign(T a) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L37) | [Definition](/stdlib/math.qnp?plain=1#L74)
```qinp
\\ Returns the sign of a value
\\ @param a The value
\\ @return The sign of the value
```

