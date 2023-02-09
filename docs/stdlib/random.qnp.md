
# Stdlib - stdlib/random.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Details](#details)


## Globals
 - var<u64> std.random.__seed

## Functions
 - [fn<void*> std.random.fill(void* buffer, u64 size)](#ref_8e61b0c4d90bb34ddc5fb9e5c48405bb)
 - [fn<u64> std.random.randU64() nodiscard](#ref_0745863634a44a563d788c8d0af0b88c)
 - [blueprint fn<T> std.random.random() nodiscard](#ref_eb3936d333d1565fcb75b95f434f1db1)
 - [blueprint fn<T> std.random.range(T min, T max) nodiscard](#ref_8571a97c8097003c5f0816c2718a92c9)
 - [fn<> std.random.seed(u64 seed)](#ref_412555faec8930093b0f87cffffdfdaa)

## Details
#### <a id="ref_8e61b0c4d90bb34ddc5fb9e5c48405bb"/>fn<void*> std.random.fill(void* buffer, u64 size)
> [Declaration](/stdlib/random.qnp?plain=1#L29) | [Definition](/stdlib/random.qnp?plain=1#L51)
```qinp
\\ Fill a buffer with random data
\\ @param buffer The buffer to fill
\\ @param size The size of the buffer
\\ @returns The buffer
```
#### <a id="ref_0745863634a44a563d788c8d0af0b88c"/>fn<u64> std.random.randU64() nodiscard
> [Declaration](/stdlib/random.qnp?plain=1#L12) | [Definition](/stdlib/random.qnp?plain=1#L41)
```qinp
\\ Random number generator
\\ @returns A random integer between U64_MIN and U64_MAX
```
#### <a id="ref_eb3936d333d1565fcb75b95f434f1db1"/>blueprint fn<T> std.random.random() nodiscard
> [Declaration](/stdlib/random.qnp?plain=1#L17) | [Definition](/stdlib/random.qnp?plain=1#L44)
```qinp
\\ Random number generator
\\ T must be a integer type
\\ @returns A random integer between T_MIN and T_MAX
```
#### <a id="ref_8571a97c8097003c5f0816c2718a92c9"/>blueprint fn<T> std.random.range(T min, T max) nodiscard
> [Declaration](/stdlib/random.qnp?plain=1#L23) | [Definition](/stdlib/random.qnp?plain=1#L48)
```qinp
\\ Generates a random number in a specified range
\\ @param min The minimum value
\\ @param max The maximum value
\\ @returns A random integer between min and max (inclusive)
```
#### <a id="ref_412555faec8930093b0f87cffffdfdaa"/>fn<> std.random.seed(u64 seed)
> [Declaration](/stdlib/random.qnp?plain=1#L8) | [Definition](/stdlib/random.qnp?plain=1#L38)
```qinp
\\ Seeds the random number generator
\\ @param seed The seed to use
```

