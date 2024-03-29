
# Stdlib - stdlib/random.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Details](#details)


## Globals
 - [var\<u64\> std.random.__seed](#ref_c3f15f919ff5c47263ff53a63125bb67)

## Functions
 - [fn\<void*\> std.random.fill(void* buffer, u64 size)](#ref_49c7956de099810e259ea53538d5c1ea)
 - [fn\<u64\> std.random.randU64() nodiscard](#ref_67351cd0f694e95fbdad681bb84e0ec3)
 - [fn\<T\> std.random.random() nodiscard](#ref_e95962300b3949d16ca970d8fa0928ff)
 - [fn\<T\> std.random.range(T min, T max) nodiscard](#ref_4148f76e4e1212e891bb0b86f7de9572)
 - [fn\<\> std.random.seed(u64 seed)](#ref_6640e06ad3641d2c0d88029c0a2c0fbc)

## Details
#### <a id="ref_c3f15f919ff5c47263ff53a63125bb67"/>var\<u64\> std.random.__seed
> [Declaration](/stdlib/random.qnp?plain=1#L38)
```qinp
The seed used by the random number generator
```
#### <a id="ref_49c7956de099810e259ea53538d5c1ea"/>fn\<void*\> std.random.fill(void* buffer, u64 size)
> [Declaration](/stdlib/random.qnp?plain=1#L31) | [Definition](/stdlib/random.qnp?plain=1#L53)
```qinp
Fill a buffer with random data
@param buffer The buffer to fill
@param size The size of the buffer
@returns The buffer
```
#### <a id="ref_67351cd0f694e95fbdad681bb84e0ec3"/>fn\<u64\> std.random.randU64() nodiscard
> [Declaration](/stdlib/random.qnp?plain=1#L14) | [Definition](/stdlib/random.qnp?plain=1#L43)
```qinp
Random number generator
@returns A random integer between U64_MIN and U64_MAX
```
#### <a id="ref_e95962300b3949d16ca970d8fa0928ff"/>fn\<T\> std.random.random() nodiscard
> [Declaration](/stdlib/random.qnp?plain=1#L19) | [Definition](/stdlib/random.qnp?plain=1#L46)
```qinp
Random number generator
T must be a integer type
@returns A random integer between T_MIN and T_MAX
```
#### <a id="ref_4148f76e4e1212e891bb0b86f7de9572"/>fn\<T\> std.random.range(T min, T max) nodiscard
> [Declaration](/stdlib/random.qnp?plain=1#L25) | [Definition](/stdlib/random.qnp?plain=1#L50)
```qinp
Generates a random number in a specified range
@param min The minimum value
@param max The maximum value
@returns A random integer between min and max (inclusive)
```
#### <a id="ref_6640e06ad3641d2c0d88029c0a2c0fbc"/>fn\<\> std.random.seed(u64 seed)
> [Declaration](/stdlib/random.qnp?plain=1#L10) | [Definition](/stdlib/random.qnp?plain=1#L40)
```qinp
Seeds the random number generator
@param seed The seed to use
```

