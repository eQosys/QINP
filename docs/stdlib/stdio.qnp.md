
# Stdlib - stdlib/stdio.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [fn\<i32\> std.__printf(u8 const** pStr, T val)](#ref_92a657806b74ff9696a48ed7468aa67b)
 - [fn\<u8\> std.getchar() nodiscard](#ref_2429c551516f61cdca2e363ab9b74350)
 - [fn\<i32\> std.print(T val)](#ref_681d0735318e7582191a6dac62c8b927)
 - [fn\<i32\> std.print(T val, ...)](#ref_c1ea5723cf53aeaa4fdfd566d032c1b3)
 - [fn\<i32\> std.print(u8 const* str, T val)](#ref_2ff8303d971b0840664dc9daa08eeabd)
 - [fn\<i32\> std.print(u8 const* str, T val, ...)](#ref_e857908ca1aa19fddf5af17b5bdccc54)
 - [fn\<i32\> std.print(bool val)](#ref_6edaa46df87737c2fd35cd07f677c122)
 - [fn\<i32\> std.print(u8 char)](#ref_2a64ebd9be54c8488f7cf98a5c2b5837)
 - [fn\<i32\> std.print(u8 const* str)](#ref_29740113bb79b0cb7d1c131548ff0e1f)
 - [fn\<i32\> std.println()](#ref_37010d86dac151fe6c59b96e986d8278)
 - [fn\<i32\> std.println(T val)](#ref_9e2171085cb5b7b5490df6abe30bfbe1)
 - [fn\<i32\> std.println(T val, ...)](#ref_b058c70d99d62c2a1aacf095538136b5)
 - [fn\<i32\> std.printn(u8 const* str, u64 num) ...](#ref_136b6a7ba51a95ace40f4bab7a28482a)
 - [fn\<i32\> std.scann(u8* dest, u64 num) ...](#ref_14749def6dac39e09280b4ed5ffa05ca)

## Details
#### <a id="ref_92a657806b74ff9696a48ed7468aa67b"/>fn\<i32\> std.__printf(u8 const** pStr, T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L57) | [Definition](/stdlib/stdio.qnp?plain=1#L128)
```qinp
\\ Internal function to print a formatted string to stdout
\\ @param pStr A pointer to the string to print
\\ @param val The value to place in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_2429c551516f61cdca2e363ab9b74350"/>fn\<u8\> std.getchar() nodiscard
> [Declaration](/stdlib/stdio.qnp?plain=1#L76) | [Definition](/stdlib/stdio.qnp?plain=1#L160)
```qinp
\\ Reads a character from stdin
\\ @return The character read
```
#### <a id="ref_681d0735318e7582191a6dac62c8b927"/>fn\<i32\> std.print(T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L32) | [Definition](/stdlib/stdio.qnp?plain=1#L101)
```qinp
\\ Print a base-10 value with any other integral type to stdout
\\ @param num The integer to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_c1ea5723cf53aeaa4fdfd566d032c1b3"/>fn\<i32\> std.print(T val, ...)
> [Declaration](/stdlib/stdio.qnp?plain=1#L38) | [Definition](/stdlib/stdio.qnp?plain=1#L105)
```qinp
\\ Print two or more values of different types consecutively to stdout
\\ @param val The first value to print
\\ @param ... The remaining values to print
\\ @return Positive on success, negative on error.
```
#### <a id="ref_2ff8303d971b0840664dc9daa08eeabd"/>fn\<i32\> std.print(u8 const* str, T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L44) | [Definition](/stdlib/stdio.qnp?plain=1#L110)
```qinp
\\ Format a string and print it to stdout
\\ @param str The string to print
\\ @param val The first argument placed in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_e857908ca1aa19fddf5af17b5bdccc54"/>fn\<i32\> std.print(u8 const* str, T val, ...)
> [Declaration](/stdlib/stdio.qnp?plain=1#L51) | [Definition](/stdlib/stdio.qnp?plain=1#L119)
```qinp
\\ Format a string and print it to stdout
\\ @param str The string to print
\\ @param val The first argument placed in the string
\\ @param ... The remaining arguments placed in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_6edaa46df87737c2fd35cd07f677c122"/>fn\<i32\> std.print(bool val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L27) | [Definition](/stdlib/stdio.qnp?plain=1#L98)
```qinp
\\ Print a boolean value to stdout
\\ @param val The boolean to print (printed as "true" or "false")
\\ @return Positive on success, negative on error.
```
#### <a id="ref_2a64ebd9be54c8488f7cf98a5c2b5837"/>fn\<i32\> std.print(u8 char)
> [Declaration](/stdlib/stdio.qnp?plain=1#L22) | [Definition](/stdlib/stdio.qnp?plain=1#L95)
```qinp
\\ Print a single character to stdout
\\ @param char The character to print
\\ @return Positive on success, negative on error.
```
#### <a id="ref_29740113bb79b0cb7d1c131548ff0e1f"/>fn\<i32\> std.print(u8 const* str)
> [Declaration](/stdlib/stdio.qnp?plain=1#L17) | [Definition](/stdlib/stdio.qnp?plain=1#L88)
```qinp
\\ Print a null-terminated string to stdout
\\ @param str The null-terminated string to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_37010d86dac151fe6c59b96e986d8278"/>fn\<i32\> std.println()
> [Declaration](/stdlib/stdio.qnp?plain=1#L61) | [Definition](/stdlib/stdio.qnp?plain=1#L146)
```qinp
\\ Prints a newline to stdout
\\ @return Positive on success, negative on error.
```
#### <a id="ref_9e2171085cb5b7b5490df6abe30bfbe1"/>fn\<i32\> std.println(T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L66) | [Definition](/stdlib/stdio.qnp?plain=1#L150)
```qinp
\\ Same as print but appends a newline
\\ @param val The value to print (passed to appropriate print function)
\\ @return Positive on success, negative on error.
```
#### <a id="ref_b058c70d99d62c2a1aacf095538136b5"/>fn\<i32\> std.println(T val, ...)
> [Declaration](/stdlib/stdio.qnp?plain=1#L72) | [Definition](/stdlib/stdio.qnp?plain=1#L155)
```qinp
\\ Same as print but appends a newline
\\ @param val The value to print (passed to appropriate print function)
\\ @param ... The remaining values to print (passed to appropriate print function)
\\ @return Positive on success, negative on error.
```
#### <a id="ref_136b6a7ba51a95ace40f4bab7a28482a"/>fn\<i32\> std.printn(u8 const* str, u64 num) ...
> [Declaration](/stdlib/stdio.qnp?plain=1#L12) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L16)
```qinp
\\ Print the first num chars of a string to stdout
\\ @param str The string to print
\\ @param len The length of the string to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_14749def6dac39e09280b4ed5ffa05ca"/>fn\<i32\> std.scann(u8* dest, u64 num) ...
> [Declaration](/stdlib/stdio.qnp?plain=1#L82) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L23)
```qinp
\\ Reads a specified number of characters from stdin and appends a null character
\\ @param dest The destination buffer (Minimum buffer size: [num] + 1)
\\ @param num The number of characters to read
\\ @return Number of characters read, negative on error
```

