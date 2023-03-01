
# Stdlib - stdlib/math.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [fn\<T\> std.abs(T a) nodiscard](#ref_1f40f5731275a8bbb759f96e632f9248)
 - [fn\<T\> std.max(T a, T b) nodiscard](#ref_4647c68a0c982c79783a341b421606c5)
 - [fn\<T\> std.max(T a, ...) nodiscard](#ref_e71a12366d543be42c0e95d4e4a8c616)
 - [fn\<T\> std.min(T a, T b) nodiscard](#ref_02a9415bf3a9df7fee6ac0ee50091465)
 - [fn\<T\> std.min(T a, ...) nodiscard](#ref_4a3642fe1a523b82c4f5171760985618)
 - [fn\<T\> std.pow(T base, T exponent) nodiscard](#ref_b32efe24f2f65bfb89920ee8d75c1e63)
 - [fn\<T\> std.roundDownTo(T val, T multiple) nodiscard](#ref_1e47b7d85bcca029a5bd7d954bd014ab)
 - [fn\<T\> std.roundUpTo(T val, T multiple) nodiscard](#ref_b12e54729746cb70dd8179c7d3bc8293)
 - [fn\<T\> std.sign(T a) nodiscard](#ref_0b13ebada57ca6d1aff2bacad39e3bcb)

## Details
#### <a id="ref_1f40f5731275a8bbb759f96e632f9248"/>fn\<T\> std.abs(T a) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L32) | [Definition](/stdlib/math.qnp?plain=1#L71)
```qinp
\\ Returns the absolute value of a value
\\ @param a The value
\\ @return The absolute value of the value
```
#### <a id="ref_4647c68a0c982c79783a341b421606c5"/>fn\<T\> std.max(T a, T b) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L21) | [Definition](/stdlib/math.qnp?plain=1#L65)
```qinp
\\ Returns the larger of two values
\\ @param a The first value
\\ @param b The second value
\\ @return The larger of the two values
```
#### <a id="ref_e71a12366d543be42c0e95d4e4a8c616"/>fn\<T\> std.max(T a, ...) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L27) | [Definition](/stdlib/math.qnp?plain=1#L68)
```qinp
\\ Returns the largest of two or more values
\\ @param a The first value
\\ @param ... The other values
\\ @return The largest of the values
```
#### <a id="ref_02a9415bf3a9df7fee6ac0ee50091465"/>fn\<T\> std.min(T a, T b) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L9) | [Definition](/stdlib/math.qnp?plain=1#L59)
```qinp
\\ Returns the smaller of two values
\\ @param a The first value
\\ @param b The second value
\\ @return The smaller of the two values
```
#### <a id="ref_4a3642fe1a523b82c4f5171760985618"/>fn\<T\> std.min(T a, ...) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L15) | [Definition](/stdlib/math.qnp?plain=1#L62)
```qinp
\\ Returns the smallest of two or more values
\\ @param a The first value
\\ @param ... The other values
\\ @return The smallest of the values
```
#### <a id="ref_b32efe24f2f65bfb89920ee8d75c1e63"/>fn\<T\> std.pow(T base, T exponent) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L43) | [Definition](/stdlib/math.qnp?plain=1#L77)
```qinp
\\ Returns the value of base raised to the power of exponent.
\\ @param base The base
\\ @param exponent The exponent (must be greater than or equal to 0)
\\ @return The value of base raised to the power of exponent
```
#### <a id="ref_1e47b7d85bcca029a5bd7d954bd014ab"/>fn\<T\> std.roundDownTo(T val, T multiple) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L49) | [Definition](/stdlib/math.qnp?plain=1#L83)
```qinp
\\ Rounds a value down to the nearest multiple of another value
\\ @param val The value to round down
\\ @param multiple The multiple to round down to
\\ @return The value rounded down to the nearest multiple of another value, or the value itself if it is already a multiple of the other value
```
#### <a id="ref_b12e54729746cb70dd8179c7d3bc8293"/>fn\<T\> std.roundUpTo(T val, T multiple) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L55) | [Definition](/stdlib/math.qnp?plain=1#L86)
```qinp
\\ Rounds a value up to the nearest multiple of another value
\\ @param val The value to round up
\\ @param multiple The multiple to round up to
\\ @return The value rounded up to the nearest multiple of another value, or the value itself if it is already a multiple of the other value
```
#### <a id="ref_0b13ebada57ca6d1aff2bacad39e3bcb"/>fn\<T\> std.sign(T a) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L37) | [Definition](/stdlib/math.qnp?plain=1#L74)
```qinp
\\ Returns the sign of a value
\\ @param a The value
\\ @return The sign of the value
```

