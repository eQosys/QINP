
# Stdlib - stdlib/stdio.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Enums](#enums)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<i32\> std.__extractFmtMod(u8 const* format, std.__PrintFmtMod* pMod)](#ref_8d858898d0b08f9b97737f3c3eff04cd)
 - [fn\<i32\> std.__getPrintLen(T val, std.__PrintFmtMod const* pMod)](#ref_15c4a375e7cefb736fe9aef870e0264f)
 - [fn\<i32\> std.__getPrintLen(bool val, std.__PrintFmtMod const* pMod)](#ref_bb1126fb57306b42b2b3387b2bbdfb5c)
 - [fn\<i32\> std.__getPrintLen(u8 const* str, std.__PrintFmtMod const* pMod)](#ref_1cc38996e99d43f41cbe8f42e18ef134)
 - [fn\<\> std.__initFmtMod(std.__PrintFmtMod* pMod)](#ref_37bcdadf02c99d8ea10acde888a42b6b)
 - [fn\<i32\> std.__printMaxLen(u8 const* str, u64 len)](#ref_a87f9feb670f6c64c4bb885fdfcdd9bf)
 - [fn\<i32\> std.__printMod(T val, std.__PrintFmtMod const* pMod)](#ref_6b9a103f73ae694630941f8282cd151e)
 - [fn\<i32\> std.__printMod(bool val, std.__PrintFmtMod const* pMod)](#ref_0bd4ef163eb56c2d0b45ad437af1146b)
 - [fn\<i32\> std.__printMod(u8 const* str, std.__PrintFmtMod const* pMod)](#ref_abb0f5ceef094fb8614a253482e3ada6)
 - [fn\<i32\> std.__printf(u8 const** pFormat, T val)](#ref_a6781587f4aa25c30fa24af9483e6dc1)
 - [fn\<i32\> std.__printn_unbuf(u8 const* str, u64 num) ...](#ref_ad5c980d9e34cc30d0cc7c1fdad9d881)
 - [fn\<i32\> std.flush()](#ref_b04dfb89a27e84cf694c3e073b62e0ca)
 - [fn\<u8\> std.getchar() nodiscard](#ref_2429c551516f61cdca2e363ab9b74350)
 - [fn\<i32\> std.print(T val)](#ref_681d0735318e7582191a6dac62c8b927)
 - [fn\<i32\> std.print(u8 const* format, T val)](#ref_5e066c72a6c50411410df4851174b4e0)
 - [fn\<i32\> std.print(u8 const* format, T val, ...)](#ref_586b298024c5f840d713391c45c1f481)
 - [fn\<i32\> std.print(bool val)](#ref_6edaa46df87737c2fd35cd07f677c122)
 - [fn\<i32\> std.print(u8 char)](#ref_2a64ebd9be54c8488f7cf98a5c2b5837)
 - [fn\<i32\> std.print(u8 const* str)](#ref_29740113bb79b0cb7d1c131548ff0e1f)
 - [fn\<i32\> std.printArray(void const* arr, u64 count, u64 elemSize, fn\<i32\>(void const*) printElem, u8 const* sep)](#ref_52b5afd4a2680699eba5ede30e4842e7)
 - [fn\<i32\> std.printArray(void const* arr, u64 count, u64 elemSize, fn\<i32\>(void const*) printElem, u8 const* pre, u8 const* sep, u8 const* post)](#ref_c2952d644882bd28f8816b5386710435)
 - [fn\<i32\> std.printRep(T val, u64 n)](#ref_b8d3dfd7974fda8c277d368fa0d89280)
 - [fn\<i32\> std.println()](#ref_37010d86dac151fe6c59b96e986d8278)
 - [fn\<i32\> std.println(T val)](#ref_9e2171085cb5b7b5490df6abe30bfbe1)
 - [fn\<i32\> std.println(u8 const* format, ...)](#ref_6bf6d9c9c0fb93b9a3744f05bb3e75c9)
 - [fn\<i32\> std.printn(u8 const* str, u64 num)](#ref_92f1683f8006d30c1ee29b97fa13578e)
 - [fn\<i32\> std.scann(u8* dest, u64 num) ...](#ref_14749def6dac39e09280b4ed5ffa05ca)

## Packs/Unions
 - [pack std.__PrintFmtMod](#ref_c3200582f96c5cf5a6c65b486e805ae2)

## Enums
 - [enum std.__PrintFmtMod_Alignment](#ref_a7d69720aaa334aa199a76e5d68a5912)
 - [enum std.__PrintFmtMod_Type](#ref_89a63c089ecd89af9191d65312989c68)

## Macros
 - [std.FN_PRINT_ELEM](#ref_8bc662deaf09131583628995e3fcb002)
 - [std.LMBD_PRINT_ELEM(type, fmt)](#ref_d4cb127c2e88afe9ddbbf9de7e19b025)
 - [std.__PRINT_BUF_SIZE](#ref_e9ab306906725b90e3dda2314b1b298a)

## Details
#### <a id="ref_8bc662deaf09131583628995e3fcb002"/>std.FN_PRINT_ELEM
> [Declaration](/stdlib/stdio.qnp?plain=1#L12)
```qinp
Function signature for a function printing a single element of an array
```
#### <a id="ref_d4cb127c2e88afe9ddbbf9de7e19b025"/>std.LMBD_PRINT_ELEM(type, fmt)
> [Declaration](/stdlib/stdio.qnp?plain=1#L15)
```qinp
Macro to define a lambda function that prints a single element of an array
```
#### <a id="ref_e9ab306906725b90e3dda2314b1b298a"/>std.__PRINT_BUF_SIZE
> [Declaration](/stdlib/stdio.qnp?plain=1#L19)
```qinp
Internal constant
Specifies the size of the stdout buffer
```
#### <a id="ref_c3200582f96c5cf5a6c65b486e805ae2"/>pack std.__PrintFmtMod
> [Declaration](/stdlib/stdio.qnp?plain=1#L32) | [Definition](/stdlib/stdio.qnp?plain=1#L196)
```qinp
Pack used to store a format modifier for printing
```
#### <a id="ref_a7d69720aaa334aa199a76e5d68a5912"/>enum std.__PrintFmtMod_Alignment
> [Declaration](/stdlib/stdio.qnp?plain=1#L28)
```qinp
Internal enum used to specify the alignment of a format modifier
```
#### <a id="ref_89a63c089ecd89af9191d65312989c68"/>enum std.__PrintFmtMod_Type
> [Declaration](/stdlib/stdio.qnp?plain=1#L22)
```qinp
Internal enum used to specify the type of a format modifier
```
#### <a id="ref_8d858898d0b08f9b97737f3c3eff04cd"/>fn\<i32\> std.__extractFmtMod(u8 const* format, std.__PrintFmtMod* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L119) | [Definition](/stdlib/stdio.qnp?plain=1#L350)
```qinp
Internal function to extract the format modifier from the format string
@param format Format string
@param pMod Pointer to a format modifier pack to store the info in
@return Number of characters read (Length of the format modifier)
```
#### <a id="ref_15c4a375e7cefb736fe9aef870e0264f"/>fn\<i32\> std.__getPrintLen(T val, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L153) | [Definition](/stdlib/stdio.qnp?plain=1#L456)
```qinp
Internal function to retrieve the length of a value to print
@param val The value to get the length of
@param pMod The format modifier used to print the value
@return The length of the value
```
#### <a id="ref_bb1126fb57306b42b2b3387b2bbdfb5c"/>fn\<i32\> std.__getPrintLen(bool val, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L165) | [Definition](/stdlib/stdio.qnp?plain=1#L473)
```qinp
Internal function to retrieve the length of a boolean value to print
@param val The value to get the length of
@param pMod The format modifier used to print the value
@return The length of the value
```
#### <a id="ref_1cc38996e99d43f41cbe8f42e18ef134"/>fn\<i32\> std.__getPrintLen(u8 const* str, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L159) | [Definition](/stdlib/stdio.qnp?plain=1#L470)
```qinp
Internal function to retrieve the length of a string to print
@param val The value to get the length of
@param pMod The format modifier used to print the value
@return The length of the value
```
#### <a id="ref_37bcdadf02c99d8ea10acde888a42b6b"/>fn\<\> std.__initFmtMod(std.__PrintFmtMod* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L147) | [Definition](/stdlib/stdio.qnp?plain=1#L449)
```qinp
Internal function to initialize a format modifier
@param pMod Pointer to the format modifier to initialize
```
#### <a id="ref_a87f9feb670f6c64c4bb885fdfcdd9bf"/>fn\<i32\> std.__printMaxLen(u8 const* str, u64 len)
> [Declaration](/stdlib/stdio.qnp?plain=1#L143) | [Definition](/stdlib/stdio.qnp?plain=1#L442)
```qinp
Internal function to print len characters of a string or less.
@param str The string to print
@param len The maximum number of characters to print
@return The number of characters printed, negative on error.
```
#### <a id="ref_6b9a103f73ae694630941f8282cd151e"/>fn\<i32\> std.__printMod(T val, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L125) | [Definition](/stdlib/stdio.qnp?plain=1#L419)
```qinp
Internal function to print a value using a specified format modifier
@param val The value to print
@param mod The format modifier to use
@return The number of characters printed, negative on error.
```
#### <a id="ref_0bd4ef163eb56c2d0b45ad437af1146b"/>fn\<i32\> std.__printMod(bool val, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L137) | [Definition](/stdlib/stdio.qnp?plain=1#L438)
```qinp
Internal function to print a value using a specified format modifier
@param val The value to print
@param mod The format modifier to use
@return The number of characters printed, negative on error.
```
#### <a id="ref_abb0f5ceef094fb8614a253482e3ada6"/>fn\<i32\> std.__printMod(u8 const* str, std.__PrintFmtMod const* pMod)
> [Declaration](/stdlib/stdio.qnp?plain=1#L131) | [Definition](/stdlib/stdio.qnp?plain=1#L433)
```qinp
Internal function to print a value using a specified format modifier
@param str The string to print
@param mod The format modifier to use
@return The number of characters printed, negative on error.
```
#### <a id="ref_a6781587f4aa25c30fa24af9483e6dc1"/>fn\<i32\> std.__printf(u8 const** pFormat, T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L113) | [Definition](/stdlib/stdio.qnp?plain=1#L310)
```qinp
Internal function to print a formatted string to stdout
@param pFormat A pointer to the format string
@param val The value to place in the string
@return The number of characters printed, negative on error.
```
#### <a id="ref_ad5c980d9e34cc30d0cc7c1fdad9d881"/>fn\<i32\> std.__printn_unbuf(u8 const* str, u64 num) ...
> [Declaration](/stdlib/stdio.qnp?plain=1#L38) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L19)
```qinp
Print the first num chars of a string to stdout (unbuffered)
@param str The string to print
@param len The length of the string to print
@return The number of characters printed. Negative on error.
```
#### <a id="ref_b04dfb89a27e84cf694c3e073b62e0ca"/>fn\<i32\> std.flush()
> [Declaration](/stdlib/stdio.qnp?plain=1#L48) | [Definition](/stdlib/stdio.qnp?plain=1#L234)
```qinp
Flushes the stdout buffer
@return The number of characters flushed. Negative on error.
```
#### <a id="ref_2429c551516f61cdca2e363ab9b74350"/>fn\<u8\> std.getchar() nodiscard
> [Declaration](/stdlib/stdio.qnp?plain=1#L184) | [Definition](/stdlib/stdio.qnp?plain=1#L493)
```qinp
Reads a character from stdin
@return The character read
```
#### <a id="ref_681d0735318e7582191a6dac62c8b927"/>fn\<i32\> std.print(T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L68) | [Definition](/stdlib/stdio.qnp?plain=1#L250)
```qinp
Print a base-10 value with any other integral type to stdout
@param num The integer to print
@return The number of characters printed. Negative on error.
```
#### <a id="ref_5e066c72a6c50411410df4851174b4e0"/>fn\<i32\> std.print(u8 const* format, T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L74) | [Definition](/stdlib/stdio.qnp?plain=1#L254)
```qinp
Format a string and print it to stdout
@param format Format string
@param val The first argument placed in the string
@return The number of characters printed, negative on error.
```
#### <a id="ref_586b298024c5f840d713391c45c1f481"/>fn\<i32\> std.print(u8 const* format, T val, ...)
> [Declaration](/stdlib/stdio.qnp?plain=1#L81) | [Definition](/stdlib/stdio.qnp?plain=1#L266)
```qinp
Format a string and print it to stdout
@param format Format string
@param val The first argument placed in the string
@param ... The remaining arguments placed in the string
@return The number of characters printed, negative on error.
```
#### <a id="ref_6edaa46df87737c2fd35cd07f677c122"/>fn\<i32\> std.print(bool val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L63) | [Definition](/stdlib/stdio.qnp?plain=1#L247)
```qinp
Print a boolean value to stdout
@param val The boolean to print (printed as "true" or "false")
@return The number of characters printed, negative on error.
```
#### <a id="ref_2a64ebd9be54c8488f7cf98a5c2b5837"/>fn\<i32\> std.print(u8 char)
> [Declaration](/stdlib/stdio.qnp?plain=1#L58) | [Definition](/stdlib/stdio.qnp?plain=1#L244)
```qinp
Print a single character to stdout
@param char The character to print
@return The number of characters printed, negative on error.
```
#### <a id="ref_29740113bb79b0cb7d1c131548ff0e1f"/>fn\<i32\> std.print(u8 const* str)
> [Declaration](/stdlib/stdio.qnp?plain=1#L53) | [Definition](/stdlib/stdio.qnp?plain=1#L237)
```qinp
Print a null-terminated string to stdout
@param str The null-terminated string to print
@return The number of characters printed. Negative on error.
```
#### <a id="ref_52b5afd4a2680699eba5ede30e4842e7"/>fn\<i32\> std.printArray(void const* arr, u64 count, u64 elemSize, fn\<i32\>(void const*) printElem, u8 const* sep)
> [Declaration](/stdlib/stdio.qnp?plain=1#L96) | [Definition](/stdlib/stdio.qnp?plain=1#L286)
```qinp
Print an array of values to stdout
@param arr The array to print
@param count The number of elements in the array
@param elemSize The size of each element in the array
@param printElem The function to use to print each element
@param sep The separator to print between each element
@return The number of characters printed, negative on error.
```
#### <a id="ref_c2952d644882bd28f8816b5386710435"/>fn\<i32\> std.printArray(void const* arr, u64 count, u64 elemSize, fn\<i32\>(void const*) printElem, u8 const* pre, u8 const* sep, u8 const* post)
> [Declaration](/stdlib/stdio.qnp?plain=1#L107) | [Definition](/stdlib/stdio.qnp?plain=1#L301)
```qinp
Print an array of values to stdout, surrounded by pre/post strings
@param arr The array to print
@param count The number of elements in the array
@param elemSize The size of each element in the array
@param printElem The function to use to print each element
@param pre The string to print before the array
@param sep The separator to print between each element
@param post The string to print after the array
@return The number of characters printed, negative on error.
```
#### <a id="ref_b8d3dfd7974fda8c277d368fa0d89280"/>fn\<i32\> std.printRep(T val, u64 n)
> [Declaration](/stdlib/stdio.qnp?plain=1#L87) | [Definition](/stdlib/stdio.qnp?plain=1#L278)
```qinp
Print a value n timess to stdout
@param val The value to print
@param n The number of times to print it
@return The number of characters printed, negative on error.
```
#### <a id="ref_37010d86dac151fe6c59b96e986d8278"/>fn\<i32\> std.println()
> [Declaration](/stdlib/stdio.qnp?plain=1#L169) | [Definition](/stdlib/stdio.qnp?plain=1#L476)
```qinp
Prints a newline to stdout
@return Positive on success, negative on error.
```
#### <a id="ref_9e2171085cb5b7b5490df6abe30bfbe1"/>fn\<i32\> std.println(T val)
> [Declaration](/stdlib/stdio.qnp?plain=1#L174) | [Definition](/stdlib/stdio.qnp?plain=1#L479)
```qinp
Same as print but appends a newline
@param val The value to print (passed to appropriate print function)
@return The number of characters printed, negative on error.
```
#### <a id="ref_6bf6d9c9c0fb93b9a3744f05bb3e75c9"/>fn\<i32\> std.println(u8 const* format, ...)
> [Declaration](/stdlib/stdio.qnp?plain=1#L180) | [Definition](/stdlib/stdio.qnp?plain=1#L486)
```qinp
Same as print but appends a newline
@param format Format string
@param ... The remaining values to print (passed to appropriate print function)
@return The number of characters printed, negative on error.
```
#### <a id="ref_92f1683f8006d30c1ee29b97fa13578e"/>fn\<i32\> std.printn(u8 const* str, u64 num)
> [Declaration](/stdlib/stdio.qnp?plain=1#L44) | [Definition](/stdlib/stdio.qnp?plain=1#L203)
```qinp
Print the first num chars of a string to stdout
@param str The string to print
@param len The length of the string to print
@return The number of characters printed. Negative on error.
```
#### <a id="ref_14749def6dac39e09280b4ed5ffa05ca"/>fn\<i32\> std.scann(u8* dest, u64 num) ...
> [Declaration](/stdlib/stdio.qnp?plain=1#L190) | [Definition](/stdlib/platform/linux/stdio.qnp?plain=1#L26)
```qinp
Reads a specified number of characters from stdin and appends a null character
@param dest The destination buffer (Minimum buffer size: [num] + 1)
@param num The number of characters to read
@return Number of characters read, negative on error
```

