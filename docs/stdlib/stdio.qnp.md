
# Stdlib - stdlib/stdio.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [blueprint fn<i32> std.__printf(u8 const** pStr, T val)](#ref_5c548d50a62e41943eb4522237760aff)
 - [fn<u8> std.getchar() nodiscard](#ref_76fecaf8690d625128fa68de74fb70d6)
 - [blueprint fn<i32> std.print(T val)](#ref_f911f938c5caae164e9b6747fa41fd54)
 - [blueprint fn<i32> std.print(T val, ...)](#ref_3e72e89e1a071c31d44d88eb9d36f342)
 - [blueprint fn<i32> std.print(u8 const* str, T val)](#ref_76052300ae4c3caf8218676efdc87983)
 - [blueprint fn<i32> std.print(u8 const* str, T val, ...)](#ref_bdf729eb87a6b4e6cc942beac4faeb18)
 - [fn<i32> std.print(u8 char)](#ref_a388b9205001ba95d248317a25957212)
 - [fn<i32> std.print(u8 const* str)](#ref_2c377c9449edfe997008ebae6a89c01c)
 - [fn<i32> std.println()](#ref_bd854160adb0b0a08885584935647bbc)
 - [blueprint fn<i32> std.println(T val)](#ref_7506373c16f2b3b93801d1f7eaca6e58)
 - [blueprint fn<i32> std.println(T val, ...)](#ref_192a1b60464cb652aecf20f39ec473d4)
 - [fn<i32> std.printn(u8 const* str, u64 num) ...](#ref_547043127b25e8f522a331d71189b4a3)
 - [fn<i32> std.scann(u8* dest, u64 num) ...](#ref_0d124b96487848ab4d7d56f75e0cd43a)

## Details
#### <a id="ref_5c548d50a62e41943eb4522237760aff"/>blueprint fn<i32> std.__printf(u8 const** pStr, T val)
```qinp
\\ Internal function to print a formatted string to stdout
\\ @param pStr A pointer to the string to print
\\ @param val The value to place in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_76fecaf8690d625128fa68de74fb70d6"/>fn<u8> std.getchar() nodiscard
```qinp
\\ Reads a character from stdin
\\ @return The character read
```
#### <a id="ref_f911f938c5caae164e9b6747fa41fd54"/>blueprint fn<i32> std.print(T val)
```qinp
\\ Print a base-10 value with any other integral type to stdout
\\ @param num The integer to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_3e72e89e1a071c31d44d88eb9d36f342"/>blueprint fn<i32> std.print(T val, ...)
```qinp
\\ Print two or more values of different types consecutively to stdout
\\ @param val The first value to print
\\ @param ... The remaining values to print
\\ @return Positive on success, negative on error.
```
#### <a id="ref_76052300ae4c3caf8218676efdc87983"/>blueprint fn<i32> std.print(u8 const* str, T val)
```qinp
\\ Format a string and print it to stdout
\\ @param str The string to print
\\ @param val The first argument placed in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_bdf729eb87a6b4e6cc942beac4faeb18"/>blueprint fn<i32> std.print(u8 const* str, T val, ...)
```qinp
\\ Format a string and print it to stdout
\\ @param str The string to print
\\ @param val The first argument placed in the string
\\ @param ... The remaining arguments placed in the string
\\ @return Positive on success, negative on error.
```
#### <a id="ref_a388b9205001ba95d248317a25957212"/>fn<i32> std.print(u8 char)
```qinp
\\ Print a single character to stdout
\\ @param char The character to print
\\ @return Positive on success, negative on error.
```
#### <a id="ref_2c377c9449edfe997008ebae6a89c01c"/>fn<i32> std.print(u8 const* str)
```qinp
\\ Print a null-terminated string to stdout
\\ @param str The null-terminated string to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_bd854160adb0b0a08885584935647bbc"/>fn<i32> std.println()
```qinp
\\ Prints a newline to stdout
\\ @return Positive on success, negative on error.
```
#### <a id="ref_7506373c16f2b3b93801d1f7eaca6e58"/>blueprint fn<i32> std.println(T val)
```qinp
\\ Same as print but appends a newline
\\ @param val The value to print (passed to appropriate print function)
\\ @return Positive on success, negative on error.
```
#### <a id="ref_192a1b60464cb652aecf20f39ec473d4"/>blueprint fn<i32> std.println(T val, ...)
```qinp
\\ Same as print but appends a newline
\\ @param val The value to print (passed to appropriate print function)
\\ @param ... The remaining values to print (passed to appropriate print function)
\\ @return Positive on success, negative on error.
```
#### <a id="ref_547043127b25e8f522a331d71189b4a3"/>fn<i32> std.printn(u8 const* str, u64 num) ...
```qinp
\\ Print the first num chars of a string to stdout
\\ @param str The string to print
\\ @param len The length of the string to print
\\ @return The number of characters printed. Negative on error.
```
#### <a id="ref_0d124b96487848ab4d7d56f75e0cd43a"/>fn<i32> std.scann(u8* dest, u64 num) ...
```qinp
\\ Reads a specified number of characters from stdin and appends a null character
\\ @param dest The destination buffer (Minimum buffer size: [num] + 1)
\\ @param num The number of characters to read
\\ @return Number of characters read, negative on error
```

