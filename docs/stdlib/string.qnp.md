
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
 - [fn\<u8*\> std.itos(i64 num, u8* str, i64 base) nodiscard](#ref_34463c6fab2d4a04f6e2668d0d9c6042)
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
> [Declaration](/stdlib/string.qnp?plain=1#L186) | [Definition](/stdlib/string.qnp?plain=1#L416)
```qinp
Returns wether a character is a letter or a digit
@param c The character to check
@return True if the character is a letter or a digit, false otherwise
```
#### <a id="ref_1810b29239e4f51451bfa0cdf36bc00b"/>fn\<bool\> std.isalpha(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L181) | [Definition](/stdlib/string.qnp?plain=1#L413)
```qinp
Returns wether a character is a letter
@param c The character to check
@return True if the character is a letter, false otherwise
```
#### <a id="ref_5cce27e95be0c2bbcf08898292a3d97a"/>fn\<bool\> std.isnum(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L176) | [Definition](/stdlib/string.qnp?plain=1#L410)
```qinp
Returns wether a character is a number
@param c The character to check
@return True if the character is a number, false otherwise
```
#### <a id="ref_e2ca17504d3e1a69b4336ffa868d87c8"/>fn\<bool\> std.isspace(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L191) | [Definition](/stdlib/string.qnp?plain=1#L419)
```qinp
Returns wether a character is a whitespace character
@param c The character to check
@return True if the character is a whitespace character, false otherwise
```
#### <a id="ref_34463c6fab2d4a04f6e2668d0d9c6042"/>fn\<u8*\> std.itos(i64 num, u8* str, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L148) | [Definition](/stdlib/string.qnp?plain=1#L352)
```qinp
Convert an integer to a string
@param num The integer to convert
@param str The string to store the converted integer in
@param base The base to use for the conversion (Valid values are 2 to 36 (inclusive))
@return The null-terminated string representation of the integer
```
#### <a id="ref_3bf5a26a032bb58ff2d53e04c1c0d822"/>fn\<i64\> std.stoi(u8 const* str, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L154) | [Definition](/stdlib/string.qnp?plain=1#L374)
```qinp
Convert a string to a base-10 integer
@param str The string to convert
@param base The base to use for the conversion (Valid values are 2 to 36 (inclusive))
@return The integer represented by the string
```
#### <a id="ref_4a7d161f40478de04a97c48a225a00b1"/>fn\<i64\> std.stoi(u8 const* str, u8 const** pNextOut, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L161) | [Definition](/stdlib/string.qnp?plain=1#L377)
```qinp
Convert a string to a base-10 integer
@param str The string to convert
@param pNextOut A pointer to a pointer to the first character after the converted integer (set by the function)
@param base The base to use for the conversion (Valid values are 2 to 36 (inclusive))
@return The integer represented by the string
```
#### <a id="ref_1b8eac4fa8f7258b56cd6c32f45cc214"/>fn\<u8 const*\> std.strchr(u8 const* str, u8 ch) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L130) | [Definition](/stdlib/string.qnp?plain=1#L323)
```qinp
Locate the first occurencce of a character in a string
@param str The string to search
@param ch The character to search for
@return The address of the first occurence of ch in str, or NULL if not found
```
#### <a id="ref_1ddb3c2646043df42d044dfd1e31b53b"/>fn\<i64\> std.strcmp(u8 const* str1, u8 const* str2) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L124) | [Definition](/stdlib/string.qnp?plain=1#L313)
```qinp
Compare two null-terminated strings
@param str1 The first string
@param str2 The second string
@return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_c4bde52892a632c8cd48352ae7235680"/>fn\<u64\> std.strlen(u8 const* str) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L118) | [Definition](/stdlib/string.qnp?plain=1#L304)
```qinp
Determine the length of a string (without the null terminator)
@param str The null-terminated string to measure
@return The length of the string
```
#### <a id="ref_ee07764fe157d5dabaa4c07f6acd4200"/>fn\<u8*\> std.strrev(u8* str)
> [Declaration](/stdlib/string.qnp?plain=1#L135) | [Definition](/stdlib/string.qnp?plain=1#L331)
```qinp
Reverse a null-terminated string (in-place)
@param str The null-terminated string to reverse
@return The reversed string
```
#### <a id="ref_c64e1e2f0896b5ad920d1b505678868c"/>fn\<u8*\> std.strrev(u8* begin, u8* end)
> [Declaration](/stdlib/string.qnp?plain=1#L141) | [Definition](/stdlib/string.qnp?plain=1#L338)
```qinp
Reverse a string of specified length (in-place)
@param begin Pointer to the first character of the string to reverse
@param end Pointer to the character after the last character of the string to reverse (usually the null-terminator)
@return The reversed string
```
#### <a id="ref_39608303a9ea8c9a53b85e065cd581ca"/>fn\<u8\> std.tolower(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L171) | [Definition](/stdlib/string.qnp?plain=1#L407)
```qinp
Converts a character to lowercase. Values not in the range 'A' to 'Z' are unchanged.
@param c The character to convert
@return The lowercase character
```
#### <a id="ref_d5143f66f35780c2301272595fa2e37e"/>fn\<u8\> std.toupper(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L166) | [Definition](/stdlib/string.qnp?plain=1#L404)
```qinp
Converts a character to uppercase. Values not in the range 'a' to 'z' are unchanged.
@param c The character to convert
@return The uppercase character
```

