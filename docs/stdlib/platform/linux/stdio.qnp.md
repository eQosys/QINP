
# Stdlib - stdlib/platform/linux/stdio.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<i32\> std.__printn_unbuf(u8 const* str, u64 num)](#ref_8b9233f9e565d77f5e5b61ab03eaad1a)
 - [fn\<i32\> std.scann(u8* dest, u64 num)](#ref_b15555d75a6fdd47e44ec56c04992654)

## Macros
 - std.__STDERR
 - std.__STDIN
 - std.__STDOUT

## Details
#### <a id="ref_8b9233f9e565d77f5e5b61ab03eaad1a"/>fn\<i32\> std.__printn_unbuf(u8 const* str, u64 num)
> [Declaration](/stdlib/stdio.qnp?plain=1#L38) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L19)
```qinp
Print the first num chars of a string to stdout (unbuffered)
@param str The string to print
@param len The length of the string to print
@return The number of characters printed. Negative on error.
```
#### <a id="ref_b15555d75a6fdd47e44ec56c04992654"/>fn\<i32\> std.scann(u8* dest, u64 num)
> [Declaration](/stdlib/stdio.qnp?plain=1#L190) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L26)
```qinp
Reads a specified number of characters from stdin and appends a null character
@param dest The destination buffer (Minimum buffer size: [num] + 1)
@param num The number of characters to read
@return Number of characters read, negative on error
```

