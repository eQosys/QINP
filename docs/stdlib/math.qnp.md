
# Stdlib - stdlib/math.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [blueprint fn<T> std.abs(T a) nodiscard](#ref_c232567e806df6eb594430beab221b68)
 - [blueprint fn<T> std.max(T a, T b) nodiscard](#ref_5fcaecb3d2e3b2fa3965aa9d8f533fe7)
 - [blueprint fn<T> std.max(T a, ...) nodiscard](#ref_48e373660360e3b56e1668a51b40d5aa)
 - [blueprint fn<T> std.min(T a, T b) nodiscard](#ref_4a8c3db1b4b0744e836dc6f6cc742223)
 - [blueprint fn<T> std.min(T a, ...) nodiscard](#ref_3f7317dd0fa1e4c390c5606ac93e9f92)
 - [blueprint fn<T> std.pow(T base, T exponent) nodiscard](#ref_0049f2bca2601cbebc1dbb64322e3934)
 - [blueprint fn<T> std.roundDownTo(T val, T multiple) nodiscard](#ref_370e72cf75f5c8db01f39dbf34f8e03a)
 - [blueprint fn<T> std.roundUpTo(T val, T multiple) nodiscard](#ref_df649bbb298b3df4a3aace95ca0e71db)
 - [blueprint fn<T> std.sign(T a) nodiscard](#ref_1569c60d301932493c7987d7c77fcb11)

## Details
#### <a id="ref_c232567e806df6eb594430beab221b68"/>blueprint fn<T> std.abs(T a) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L32) | [Definition](/stdlib/math.qnp?plain=1#L71)
```qinp
\\ Returns the absolute value of a value
\\ @param a The value
\\ @return The absolute value of the value
```
#### <a id="ref_5fcaecb3d2e3b2fa3965aa9d8f533fe7"/>blueprint fn<T> std.max(T a, T b) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L21) | [Definition](/stdlib/math.qnp?plain=1#L65)
```qinp
\\ Returns the larger of two values
\\ @param a The first value
\\ @param b The second value
\\ @return The larger of the two values
```
#### <a id="ref_48e373660360e3b56e1668a51b40d5aa"/>blueprint fn<T> std.max(T a, ...) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L27) | [Definition](/stdlib/math.qnp?plain=1#L68)
```qinp
\\ Returns the largest of two or more values
\\ @param a The first value
\\ @param ... The other values
\\ @return The largest of the values
```
#### <a id="ref_4a8c3db1b4b0744e836dc6f6cc742223"/>blueprint fn<T> std.min(T a, T b) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L9) | [Definition](/stdlib/math.qnp?plain=1#L59)
```qinp
\\ Returns the smaller of two values
\\ @param a The first value
\\ @param b The second value
\\ @return The smaller of the two values
```
#### <a id="ref_3f7317dd0fa1e4c390c5606ac93e9f92"/>blueprint fn<T> std.min(T a, ...) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L15) | [Definition](/stdlib/math.qnp?plain=1#L62)
```qinp
\\ Returns the smallest of two or more values
\\ @param a The first value
\\ @param ... The other values
\\ @return The smallest of the values
```
#### <a id="ref_0049f2bca2601cbebc1dbb64322e3934"/>blueprint fn<T> std.pow(T base, T exponent) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L43) | [Definition](/stdlib/math.qnp?plain=1#L77)
```qinp
\\ Returns the value of base raised to the power of exponent.
\\ @param base The base
\\ @param exponent The exponent (must be greater than or equal to 0)
\\ @return The value of base raised to the power of exponent
```
#### <a id="ref_370e72cf75f5c8db01f39dbf34f8e03a"/>blueprint fn<T> std.roundDownTo(T val, T multiple) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L49) | [Definition](/stdlib/math.qnp?plain=1#L83)
```qinp
\\ Rounds a value down to the nearest multiple of another value
\\ @param val The value to round down
\\ @param multiple The multiple to round down to
\\ @return The value rounded down to the nearest multiple of another value, or the value itself if it is already a multiple of the other value
```
#### <a id="ref_df649bbb298b3df4a3aace95ca0e71db"/>blueprint fn<T> std.roundUpTo(T val, T multiple) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L55) | [Definition](/stdlib/math.qnp?plain=1#L86)
```qinp
\\ Rounds a value up to the nearest multiple of another value
\\ @param val The value to round up
\\ @param multiple The multiple to round up to
\\ @return The value rounded up to the nearest multiple of another value, or the value itself if it is already a multiple of the other value
```
#### <a id="ref_1569c60d301932493c7987d7c77fcb11"/>blueprint fn<T> std.sign(T a) nodiscard
> [Declaration](/stdlib/math.qnp?plain=1#L37) | [Definition](/stdlib/math.qnp?plain=1#L74)
```qinp
\\ Returns the sign of a value
\\ @param a The value
\\ @return The sign of the value
```

