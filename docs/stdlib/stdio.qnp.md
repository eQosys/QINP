
# Stdlib - stdlib/stdio.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Enums](#enums)
 - [Details](#details)


## Functions
 - [fn\<i32\> std.__extractFmtMod(u8 const* format, std.__PrintFmtMod* pMod)](#ref_8d858898d0b08f9b97737f3c3eff04cd)
 - [fn\<i32\> std.__printf(u8 const** pFormat, T val)](#ref_a6781587f4aa25c30fa24af9483e6dc1)
 - [fn\<i32\> std.__printmod(T val, std.__PrintFmtMod const* pMod)](#ref_246c44f1a6c757d657171d337587bfc6)
 - [fn\<i32\> std.__printmod(bool val, std.__PrintFmtMod const* pMod)](#ref_5ef7e271131125b22a8aba81bae2e1b9)
 - [fn\<i32\> std.__printmod(u8 const* str, std.__PrintFmtMod const* pMod)](#ref_11895996fe419c7cd2c1eaa601d53d16)
 - [fn\<u8\> std.getchar() nodiscard](#ref_2429c551516f61cdca2e363ab9b74350)
 - [fn\<i32\> std.print(T val)](#ref_681d0735318e7582191a6dac62c8b927)
 - [fn\<i32\> std.print(u8 const* format, T val)](#ref_5e066c72a6c50411410df4851174b4e0)
 - [fn\<i32\> std.print(u8 const* format, T val, ...)](#ref_586b298024c5f840d713391c45c1f481)
 - [fn\<i32\> std.print(bool val)](#ref_6edaa46df87737c2fd35cd07f677c122)
 - [fn\<i32\> std.print(u8 char)](#ref_2a64ebd9be54c8488f7cf98a5c2b5837)
 - [fn\<i32\> std.print(u8 const* str)](#ref_29740113bb79b0cb7d1c131548ff0e1f)
 - [fn\<i32\> std.println()](#ref_37010d86dac151fe6c59b96e986d8278)
 - [fn\<i32\> std.println(T val)](#ref_9e2171085cb5b7b5490df6abe30bfbe1)
 - [fn\<i32\> std.println(u8 const* format, ...)](#ref_6bf6d9c9c0fb93b9a3744f05bb3e75c9)
 - [fn\<i32\> std.printn(u8 const* str, u64 num) ...](#ref_136b6a7ba51a95ace40f4bab7a28482a)
 - [fn\<i32\> std.scann(u8* dest, u64 num) ...](#ref_14749def6dac39e09280b4ed5ffa05ca)

## Packs/Unions
 - pack std.__PrintFmtMod

## Enums
 - std.__PrintFmtMod_Type

## Details
#### <a id="ref_8d858898d0b08f9b97737f3c3eff04cd"/>fn\<i32\> std.__extractFmtMod(u8 const* format, std.__PrintFmtMod* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L64) | [Definition](/stdlib/stdio.qnp?plain=1#L176)
```qinp
\\ Internal function to extract the format modifier from the format string
\\ @param format Format string
\\ @param pMod Pointer to a format modifier pack to store the info in
\\ @return Number of characters read (Length of the format modifier)
```
#### <a id="ref_a6781587f4aa25c30fa24af9483e6dc1"/>fn\<i32\> std.__printf(u8 const** pFormat, T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L58) | [Definition](/stdlib/stdio.qnp?plain=1#L152)
```qinp
\\ Internal function to print a formatted string to stdout
\\ @param pFormat A pointer to the format string
\\ @param val The value to place in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_246c44f1a6c757d657171d337587bfc6"/>fn\<i32\> std.__printmod(T val, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L70) | [Definition](/stdlib/stdio.qnp?plain=1#L202)
```qinp
\\ Internal function to print a value using a specified format modifier
\\ @param val The value to print
\\ @param mod The format modifier to use
\\ @return Positive on success, negative on error.
```
#### <a id="ref_5ef7e271131125b22a8aba81bae2e1b9"/>fn\<i32\> std.__printmod(bool val, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L82) | [Definition](/stdlib/stdio.qnp?plain=1#L220)
```qinp
\\ Internal function to print a value using a specified format modifier
\\ @param val The value to print
\\ @param mod The format modifier to use
\\ @return Positive on success, negative on error.
```
#### <a id="ref_11895996fe419c7cd2c1eaa601d53d16"/>fn\<i32\> std.__printmod(u8 const* str, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L76) | [Definition](/stdlib/stdio.qnp?plain=1#L216)
```qinp
\\ Internal function to print a value using a specified format modifier
\\ @param str The string to print
\\ @param mod The format modifier to use
\\ @return Positive on success, negative on error.
```
#### <a id="ref_2429c551516f61cdca2e363ab9b74350"/>fn\<u8\> std.getchar() nodiscard
> [Declaration](/stdlib/stdio.qnp?plain=1#L101) | [Definition](/stdlib/stdio.qnp?plain=1#L237)
```qinp
\\ Reads a character from stdin
\\ @return The character read
```
#### <a id="ref_681d0735318e7582191a6dac62c8b927"/>fn\<i32\> std.print(T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L39) | [Definition](/stdlib/stdio.qnp?plain=1#L130)
```qinp
\\ Print a base-10 value with any other integral type to stdout
\\ @param num The integer to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_5e066c72a6c50411410df4851174b4e0"/>fn\<i32\> std.print(u8 const* format, T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L45) | [Definition](/stdlib/stdio.qnp?plain=1#L134)
```qinp
\\ Format a string and print it to stdout
\\ @param format Format string
\\ @param val The first argument placed in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_586b298024c5f840d713391c45c1f481"/>fn\<i32\> std.print(u8 const* format, T val, ...)
> [Declaration](/stdlib/stdio.qnp?plain=1#L52) | [Definition](/stdlib/stdio.qnp?plain=1#L143)
```qinp
\\ Format a string and print it to stdout
\\ @param format Format string
\\ @param val The first argument placed in the string
\\ @param ... The remaining arguments placed in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_6edaa46df87737c2fd35cd07f677c122"/>fn\<i32\> std.print(bool val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L34) | [Definition](/stdlib/stdio.qnp?plain=1#L127)
```qinp
\\ Print a boolean value to stdout
\\ @param val The boolean to print (printed as "true" or "false")
\\ @return Positive on success, negative on error.
```
#### <a id="ref_2a64ebd9be54c8488f7cf98a5c2b5837"/>fn\<i32\> std.print(u8 char)
> [Declaration](/stdlib/stdio.qnp?plain=1#L29) | [Definition](/stdlib/stdio.qnp?plain=1#L124)
```qinp
\\ Print a single character to stdout
\\ @param char The character to print
\\ @return Positive on success, negative on error.
```
#### <a id="ref_29740113bb79b0cb7d1c131548ff0e1f"/>fn\<i32\> std.print(u8 const* str)
> [Declaration](/stdlib/stdio.qnp?plain=1#L24) | [Definition](/stdlib/stdio.qnp?plain=1#L117)
```qinp
\\ Print a null-terminated string to stdout
\\ @param str The null-terminated string to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_37010d86dac151fe6c59b96e986d8278"/>fn\<i32\> std.println()
> [Declaration](/stdlib/stdio.qnp?plain=1#L86) | [Definition](/stdlib/stdio.qnp?plain=1#L224)
```qinp
\\ Prints a newline to stdout
\\ @return Positive on success, negative on error.
```
#### <a id="ref_9e2171085cb5b7b5490df6abe30bfbe1"/>fn\<i32\> std.println(T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L91) | [Definition](/stdlib/stdio.qnp?plain=1#L227)
```qinp
\\ Same as print but appends a newline
\\ @param val The value to print (passed to appropriate print function)
\\ @return Positive on success, negative on error.
```
#### <a id="ref_6bf6d9c9c0fb93b9a3744f05bb3e75c9"/>fn\<i32\> std.println(u8 const* format, ...)
> [Declaration](/stdlib/stdio.qnp?plain=1#L97) | [Definition](/stdlib/stdio.qnp?plain=1#L232)
```qinp
\\ Same as print but appends a newline
\\ @param format Format string
\\ @param ... The remaining values to print (passed to appropriate print function)
\\ @return Positive on success, negative on error.
```
#### <a id="ref_136b6a7ba51a95ace40f4bab7a28482a"/>fn\<i32\> std.printn(u8 const* str, u64 num) ...
> [Declaration](/stdlib/stdio.qnp?plain=1#L19) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L16)
```qinp
\\ Print the first num chars of a string to stdout
\\ @param str The string to print
\\ @param len The length of the string to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_14749def6dac39e09280b4ed5ffa05ca"/>fn\<i32\> std.scann(u8* dest, u64 num) ...
> [Declaration](/stdlib/stdio.qnp?plain=1#L107) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L23)
```qinp
\\ Reads a specified number of characters from stdin and appends a null character
\\ @param dest The destination buffer (Minimum buffer size: [num] + 1)
\\ @param num The number of characters to read
\\ @return Number of characters read, negative on error
```

