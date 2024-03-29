
# Stdlib - stdlib/string.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<bool\> std.isalnum(u8 c) nodiscard](#ref_8f439e4c57f68e41f372499cbbc726f3)
 - [fn\<bool\> std.isalpha(u8 c) nodiscard](#ref_1810b29239e4f51451bfa0cdf36bc00b)
 - [fn\<bool\> std.isnum(u8 c) nodiscard](#ref_5cce27e95be0c2bbcf08898292a3d97a)
 - [fn\<bool\> std.isspace(u8 c) nodiscard](#ref_e2ca17504d3e1a69b4336ffa868d87c8)
 - [fn\<u8 const*\> std.itos(i64 num) nodiscard](#ref_be7070c07b16cd403ccb979886acbe40)
 - [fn\<u8 const*\> std.itos(i64 num, i64 base) nodiscard](#ref_ebe2c47186f09e7f07b6dc144479373b)
 - [fn\<u8 const*\> std.itos(i64 num, u8* str, i64 base) nodiscard](#ref_feeb2eba0e13d3e31e246bcf5b742cf4)
 - [fn\<i64\> std.stoi(u8 const* str, i64 base) nodiscard](#ref_3bf5a26a032bb58ff2d53e04c1c0d822)
 - [fn\<i64\> std.stoi(u8 const* str, u8 const** pNextOut, i64 base) nodiscard](#ref_4a7d161f40478de04a97c48a225a00b1)
 - [fn\<u8 const*\> std.strchr(u8 const* str, u8 ch) nodiscard](#ref_1b8eac4fa8f7258b56cd6c32f45cc214)
 - [fn\<i64\> std.strcmp(u8 const* str1, u8 const* str2) nodiscard](#ref_1ddb3c2646043df42d044dfd1e31b53b)
 - [fn\<u64\> std.strlen(u8 const* str) nodiscard](#ref_c4bde52892a632c8cd48352ae7235680)
 - [fn\<u8*\> std.strrev(u8* str)](#ref_ee07764fe157d5dabaa4c07f6acd4200)
 - [fn\<u8*\> std.strrev(u8* begin, u8* end)](#ref_c64e1e2f0896b5ad920d1b505678868c)
 - [fn\<u8\> std.tolower(u8 c) nodiscard](#ref_39608303a9ea8c9a53b85e065cd581ca)
 - [fn\<u8\> std.toupper(u8 c) nodiscard](#ref_d5143f66f35780c2301272595fa2e37e)

## Packs/Unions
 - [pack std.String](#ref_933ef196305d467ac0d1c58011cb8bf6)

## Macros
 - [std.__BASE_CHARS_LOWER](#ref_077bd649d47868cdfff08a8a6c8d2cdf)

## Details
#### <a id="ref_933ef196305d467ac0d1c58011cb8bf6"/>pack std.String
> [Definition](/stdlib/string.qnp?plain=1#L13)
```qinp
String class
```
#### <a id="ref_077bd649d47868cdfff08a8a6c8d2cdf"/>std.__BASE_CHARS_LOWER
> [Declaration](/stdlib/string.qnp?plain=1#L10)
```qinp
Used to convert between string and integer
```
#### <a id="ref_8f439e4c57f68e41f372499cbbc726f3"/>fn\<bool\> std.isalnum(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L295) | [Definition](/stdlib/string.qnp?plain=1#L597)
```qinp
Returns wether a character is a letter or a digit
@param c The character to check
@return True if the character is a letter or a digit, false otherwise
```
#### <a id="ref_1810b29239e4f51451bfa0cdf36bc00b"/>fn\<bool\> std.isalpha(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L290) | [Definition](/stdlib/string.qnp?plain=1#L594)
```qinp
Returns wether a character is a letter
@param c The character to check
@return True if the character is a letter, false otherwise
```
#### <a id="ref_5cce27e95be0c2bbcf08898292a3d97a"/>fn\<bool\> std.isnum(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L285) | [Definition](/stdlib/string.qnp?plain=1#L591)
```qinp
Returns wether a character is a number
@param c The character to check
@return True if the character is a number, false otherwise
```
#### <a id="ref_e2ca17504d3e1a69b4336ffa868d87c8"/>fn\<bool\> std.isspace(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L300) | [Definition](/stdlib/string.qnp?plain=1#L600)
```qinp
Returns wether a character is a whitespace character
@param c The character to check
@return True if the character is a whitespace character, false otherwise
```
#### <a id="ref_be7070c07b16cd403ccb979886acbe40"/>fn\<u8 const*\> std.itos(i64 num) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L244) | [Definition](/stdlib/string.qnp?plain=1#L525)
```qinp
Convert an integer to a string
Using base 10 and an internal buffer
@param num The integer to convert
@return The null-terminated string representation of the integer
```
#### <a id="ref_ebe2c47186f09e7f07b6dc144479373b"/>fn\<u8 const*\> std.itos(i64 num, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L250) | [Definition](/stdlib/string.qnp?plain=1#L528)
```qinp
Convert an integer to a string
Using an internal buffer
@param num The integer to convert
@param str The string to store the converted integer in
```
#### <a id="ref_feeb2eba0e13d3e31e246bcf5b742cf4"/>fn\<u8 const*\> std.itos(i64 num, u8* str, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L257) | [Definition](/stdlib/string.qnp?plain=1#L532)
```qinp
Convert an integer to a string
@param num The integer to convert
@param str The string to store the converted integer in
@param base The base to use for the conversion (Valid values are 2 to 36 (inclusive))
@return The null-terminated string representation of the integer
```
#### <a id="ref_3bf5a26a032bb58ff2d53e04c1c0d822"/>fn\<i64\> std.stoi(u8 const* str, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L263) | [Definition](/stdlib/string.qnp?plain=1#L554)
```qinp
Convert a string to a base-10 integer
@param str The string to convert
@param base The base to use for the conversion (Valid values are 2 to 36 (inclusive))
@return The integer represented by the string
```
#### <a id="ref_4a7d161f40478de04a97c48a225a00b1"/>fn\<i64\> std.stoi(u8 const* str, u8 const** pNextOut, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L270) | [Definition](/stdlib/string.qnp?plain=1#L557)
```qinp
Convert a string to a base-10 integer
@param str The string to convert
@param pNextOut A pointer to a pointer to the first character after the converted integer (set by the function)
@param base The base to use for the conversion (Valid values are 2 to 36 (inclusive))
@return The integer represented by the string
```
#### <a id="ref_1b8eac4fa8f7258b56cd6c32f45cc214"/>fn\<u8 const*\> std.strchr(u8 const* str, u8 ch) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L227) | [Definition](/stdlib/string.qnp?plain=1#L496)
```qinp
Locate the first occurencce of a character in a string
@param str The string to search
@param ch The character to search for
@return The address of the first occurence of ch in str, or NULL if not found
```
#### <a id="ref_1ddb3c2646043df42d044dfd1e31b53b"/>fn\<i64\> std.strcmp(u8 const* str1, u8 const* str2) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L221) | [Definition](/stdlib/string.qnp?plain=1#L486)
```qinp
Compare two null-terminated strings
@param str1 The first string
@param str2 The second string
@return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_c4bde52892a632c8cd48352ae7235680"/>fn\<u64\> std.strlen(u8 const* str) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L215) | [Definition](/stdlib/string.qnp?plain=1#L477)
```qinp
Determine the length of a string (without the null terminator)
@param str The null-terminated string to measure
@return The length of the string
```
#### <a id="ref_ee07764fe157d5dabaa4c07f6acd4200"/>fn\<u8*\> std.strrev(u8* str)
> [Declaration](/stdlib/string.qnp?plain=1#L232) | [Definition](/stdlib/string.qnp?plain=1#L504)
```qinp
Reverse a null-terminated string (in-place)
@param str The null-terminated string to reverse
@return The reversed string
```
#### <a id="ref_c64e1e2f0896b5ad920d1b505678868c"/>fn\<u8*\> std.strrev(u8* begin, u8* end)
> [Declaration](/stdlib/string.qnp?plain=1#L238) | [Definition](/stdlib/string.qnp?plain=1#L511)
```qinp
Reverse a string of specified length (in-place)
@param begin Pointer to the first character of the string to reverse
@param end Pointer to the character after the last character of the string to reverse (usually the null-terminator)
@return The reversed string
```
#### <a id="ref_39608303a9ea8c9a53b85e065cd581ca"/>fn\<u8\> std.tolower(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L280) | [Definition](/stdlib/string.qnp?plain=1#L588)
```qinp
Converts a character to lowercase. Values not in the range 'A' to 'Z' are unchanged.
@param c The character to convert
@return The lowercase character
```
#### <a id="ref_d5143f66f35780c2301272595fa2e37e"/>fn\<u8\> std.toupper(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L275) | [Definition](/stdlib/string.qnp?plain=1#L585)
```qinp
Converts a character to uppercase. Values not in the range 'a' to 'z' are unchanged.
@param c The character to convert
@return The uppercase character
```

