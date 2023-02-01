
# Stdlib - stdlib/platform/linux/stdio.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn<i32> std.printn(u8 const* str, u64 num)](#ref_c86762adc7164b6ece39cb4872a7bd68)
 - [fn<i32> std.scann(u8* dest, u64 num)](#ref_2bdda4e7dcc08093e0fdd68f7839ace1)

## Macros
 - std.__STDERR
 - std.__STDIN
 - std.__STDOUT

## Details
#### <a id="ref_c86762adc7164b6ece39cb4872a7bd68"/>fn<i32> std.printn(u8 const* str, u64 num)
> [Declaration](/stdlib/stdio.qnp?plain=1#L11) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L15)
```qinp
\\ Print the first num chars of a string to stdout
\\ @param str The string to print
\\ @param len The length of the string to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_2bdda4e7dcc08093e0fdd68f7839ace1"/>fn<i32> std.scann(u8* dest, u64 num)
> [Declaration](/stdlib/stdio.qnp?plain=1#L76) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L18)
```qinp
\\ Reads a specified number of characters from stdin and appends a null character
\\ @param dest The destination buffer (Minimum buffer size: [num] + 1)
\\ @param num The number of characters to read
\\ @return Number of characters read, negative on error
```

