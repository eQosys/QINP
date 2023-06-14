
# Stdlib - stdlib/string.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<bool\> std.isalnum(u8 c) nodiscard](#ref_8f439e4c57f68e41f372499cbbc726f3)
 - [fn\<bool\> std.isalpha(u8 c) nodiscard](#ref_1810b29239e4f51451bfa0cdf36bc00b)
 - [fn\<bool\> std.isnum(u8 c) nodiscard](#ref_5cce27e95be0c2bbcf08898292a3d97a)
 - [fn\<bool\> std.isspace(u8 c) nodiscard](#ref_e2ca17504d3e1a69b4336ffa868d87c8)
 - [fn\<u8*\> std.itos(i64 num, u8* str, i64 base) nodiscard](#ref_34463c6fab2d4a04f6e2668d0d9c6042)
 - [fn\<i64\> std.stoi(u8 const* str, i64 base) nodiscard](#ref_3bf5a26a032bb58ff2d53e04c1c0d822)
 - [fn\<u8*\> std.strcat(u8* dest, u8 const* src)](#ref_9331dc689d95f501becfb95d460fbf6f)
 - [fn\<u8*\> std.strcat(u8* dest, u8 const* src, u64 num)](#ref_9c68be5ef2993a6d31b9c33a0913722c)
 - [fn\<u8 const*\> std.strchr(u8 const* str, u8 ch) nodiscard](#ref_1b8eac4fa8f7258b56cd6c32f45cc214)
 - [fn\<i64\> std.strcmp(u8 const* str1, u8 const* str2) nodiscard](#ref_1ddb3c2646043df42d044dfd1e31b53b)
 - [fn\<i64\> std.strcmp(u8 const* str1, u8 const* str2, u64 num) nodiscard](#ref_556b0932e9c05ffa21d6c5059ea5fc98)
 - [fn\<i64\> std.strcmpi(u8 const* str1, u8 const* str2) nodiscard](#ref_343cbb14b449a96645f460904a54dc45)
 - [fn\<i64\> std.strcmpi(u8 const* str1, u8 const* str2, u64 num) nodiscard](#ref_b33c0c561898aba3d7936df53d15b381)
 - [fn\<u8*\> std.strcpy(u8* dest, u8 const* src)](#ref_1ef14cabdbdb1f8a60c41d7dd8330acc)
 - [fn\<u8*\> std.strcpy(u8* dest, u8 const* src, u64 num)](#ref_4ec67982fc2f15e4b4212eed5311b50f)
 - [fn\<u64\> std.strlen(u8 const* str) nodiscard](#ref_c4bde52892a632c8cd48352ae7235680)
 - [fn\<u8*\> std.strlwr(u8* str)](#ref_35ba22dfa80f349a21971381eaab0797)
 - [fn\<u8*\> std.strlwr(u8* str, u64 num)](#ref_22d683f0d0df7cc08e9c40030d53976a)
 - [fn\<u8 const*\> std.strrchr(u8 const* str, u8 ch) nodiscard](#ref_56c4195ca0fd0f4ec7898b438f33bc2d)
 - [fn\<u8*\> std.strrev(u8* str)](#ref_ee07764fe157d5dabaa4c07f6acd4200)
 - [fn\<u8*\> std.strrev(u8* begin, u8* end)](#ref_c64e1e2f0896b5ad920d1b505678868c)
 - [fn\<u8 const*\> std.strrstr(u8 const* str, u8 const* substr) nodiscard](#ref_742976fd072ebf52810469994fceaf7a)
 - [fn\<u8*\> std.strset(u8* str, u8 ch)](#ref_4b0988906bc64bc9a0b8f8e45c79a931)
 - [fn\<u8*\> std.strset(u8* str, u8 ch, u64 num)](#ref_88f5086e5de800710b21fc4b8b9a8a85)
 - [fn\<u8 const*\> std.strstr(u8 const* str, u8 const* substr) nodiscard](#ref_1922b1a8a0bb85f3014f153cee60b556)
 - [fn\<u8*\> std.strtok(u8* str, u8 const* delimiters) nodiscard](#ref_af3d3457b0253b744dbf22d56473821a)
 - [fn\<u8*\> std.strupr(u8* str)](#ref_39576ab55d28ad0658bcc099a94933d8)
 - [fn\<u8*\> std.strupr(u8* str, u64 num)](#ref_285d5e0a52bc36147b65113b2f871ac5)
 - [fn\<u8\> std.tolower(u8 c) nodiscard](#ref_39608303a9ea8c9a53b85e065cd581ca)
 - [fn\<u8\> std.toupper(u8 c) nodiscard](#ref_d5143f66f35780c2301272595fa2e37e)

## Macros
 - std.__BASE_CHARS_LOWER

## Details
#### <a id="ref_8f439e4c57f68e41f372499cbbc726f3"/>fn\<bool\> std.isalnum(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L184) | [Definition](/stdlib/string.qnp?plain=1#L478)
```qinp
\\ Returns wether a character is a letter or a digit
\\ @param c The character to check
\\ @return True if the character is a letter or a digit, false otherwise
```
#### <a id="ref_1810b29239e4f51451bfa0cdf36bc00b"/>fn\<bool\> std.isalpha(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L179) | [Definition](/stdlib/string.qnp?plain=1#L475)
```qinp
\\ Returns wether a character is a letter
\\ @param c The character to check
\\ @return True if the character is a letter, false otherwise
```
#### <a id="ref_5cce27e95be0c2bbcf08898292a3d97a"/>fn\<bool\> std.isnum(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L174) | [Definition](/stdlib/string.qnp?plain=1#L472)
```qinp
\\ Returns wether a character is a number
\\ @param c The character to check
\\ @return True if the character is a number, false otherwise
```
#### <a id="ref_e2ca17504d3e1a69b4336ffa868d87c8"/>fn\<bool\> std.isspace(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L189) | [Definition](/stdlib/string.qnp?plain=1#L481)
```qinp
\\ Returns wether a character is a whitespace character
\\ @param c The character to check
\\ @return True if the character is a whitespace character, false otherwise
```
#### <a id="ref_34463c6fab2d4a04f6e2668d0d9c6042"/>fn\<u8*\> std.itos(i64 num, u8* str, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L153) | [Definition](/stdlib/string.qnp?plain=1#L427)
```qinp
\\ Convert an integer to a string
\\ @param num The integer to convert
\\ @param str The string to store the converted integer in
\\ @param base The base to use for the conversion (Valid values are 2 to 36 (inclusive))
\\ @return The null-terminated string representation of the integer
```
#### <a id="ref_3bf5a26a032bb58ff2d53e04c1c0d822"/>fn\<i64\> std.stoi(u8 const* str, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L159) | [Definition](/stdlib/string.qnp?plain=1#L449)
```qinp
\\ Convert a string to a base-10 integer
\\ @param str The string to convert
\\ @param base The base to use for the conversion (Valid values are 2 to 36 (inclusive))
\\ @return The integer represented by the string
```
#### <a id="ref_9331dc689d95f501becfb95d460fbf6f"/>fn\<u8*\> std.strcat(u8* dest, u8 const* src)
> [Declaration](/stdlib/string.qnp?plain=1#L43) | [Definition](/stdlib/string.qnp?plain=1#L238)
```qinp
\\ Concatenates two strings
\\ @param dest The destination string to concatenate to
\\ @param src The source string to concatenate
\\ @return The pointer to the destination string
```
#### <a id="ref_9c68be5ef2993a6d31b9c33a0913722c"/>fn\<u8*\> std.strcat(u8* dest, u8 const* src, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L50) | [Definition](/stdlib/string.qnp?plain=1#L248)
```qinp
\\ Concatenates two strings
\\ @param dest The destination string to concatenate to
\\ @param src The source string to concatenate
\\ @param len The number of characters to concatenate (excluding the null terminator)
\\ @return The pointer to the destination string
```
#### <a id="ref_1b8eac4fa8f7258b56cd6c32f45cc214"/>fn\<u8 const*\> std.strchr(u8 const* str, u8 ch) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L95) | [Definition](/stdlib/string.qnp?plain=1#L318)
```qinp
\\ Locate the first occurencce of a character in a string
\\ @param str The string to search
\\ @param ch The character to search for
\\ @return The address of the first occurence of ch in str, or NULL if not found
```
#### <a id="ref_1ddb3c2646043df42d044dfd1e31b53b"/>fn\<i64\> std.strcmp(u8 const* str1, u8 const* str2) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L69) | [Definition](/stdlib/string.qnp?plain=1#L278)
```qinp
\\ Compare two null-terminated strings
\\ @param str1 The first string
\\ @param str2 The second string
\\ @return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_556b0932e9c05ffa21d6c5059ea5fc98"/>fn\<i64\> std.strcmp(u8 const* str1, u8 const* str2, u64 num) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L76) | [Definition](/stdlib/string.qnp?plain=1#L288)
```qinp
\\ Compare the first num characters of two strings
\\ @param str1 The first string
\\ @param str2 The second string
\\ @param num The number of characters to compare
\\ @return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_343cbb14b449a96645f460904a54dc45"/>fn\<i64\> std.strcmpi(u8 const* str1, u8 const* str2) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L82) | [Definition](/stdlib/string.qnp?plain=1#L298)
```qinp
\\ Compare two null-terimated strings without case sensitivity
\\ @param str1 The first string
\\ @param str2 The second string
\\ @return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_b33c0c561898aba3d7936df53d15b381"/>fn\<i64\> std.strcmpi(u8 const* str1, u8 const* str2, u64 num) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L89) | [Definition](/stdlib/string.qnp?plain=1#L308)
```qinp
\\ Compare the first num characters of two strings without case sensitivity
\\ @param str1 The first string
\\ @param str2 The second string
\\ @param num The number of characters to compare
\\ @return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_1ef14cabdbdb1f8a60c41d7dd8330acc"/>fn\<u8*\> std.strcpy(u8* dest, u8 const* src)
> [Declaration](/stdlib/string.qnp?plain=1#L56) | [Definition](/stdlib/string.qnp?plain=1#L258)
```qinp
\\ Copy a null-terminated string
\\ @param dest The destination buffer
\\ @param src The source string
\\ @return The destination buffer
```
#### <a id="ref_4ec67982fc2f15e4b4212eed5311b50f"/>fn\<u8*\> std.strcpy(u8* dest, u8 const* src, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L63) | [Definition](/stdlib/string.qnp?plain=1#L268)
```qinp
\\ Copy the first num characters of a string
\\ @param dest The destination buffer
\\ @param src The source string
\\ @param len The number of characters to copy (excluding the null terminator)
\\ @return The destination buffer
```
#### <a id="ref_c4bde52892a632c8cd48352ae7235680"/>fn\<u64\> std.strlen(u8 const* str) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L13) | [Definition](/stdlib/string.qnp?plain=1#L193)
```qinp
\\ Determine the length of a string (without the null terminator)
\\ @param str The null-terminated string to measure
\\ @return The length of the string
```
#### <a id="ref_35ba22dfa80f349a21971381eaab0797"/>fn\<u8*\> std.strlwr(u8* str)
> [Declaration](/stdlib/string.qnp?plain=1#L18) | [Definition](/stdlib/string.qnp?plain=1#L202)
```qinp
\\ Convert a string to lowercase (in place)
\\ @param str The string to convert
\\ @return The pointer to the passed string
```
#### <a id="ref_22d683f0d0df7cc08e9c40030d53976a"/>fn\<u8*\> std.strlwr(u8* str, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L25) | [Definition](/stdlib/string.qnp?plain=1#L211)
```qinp
\\ Convert the first num characters of a string to lowercase (in place)
\\ Does not check for null terminator
\\ @param str The string to convert
\\ @param num The number of characters to convert
\\ @return The pointer to the passed string
```
#### <a id="ref_56c4195ca0fd0f4ec7898b438f33bc2d"/>fn\<u8 const*\> std.strrchr(u8 const* str, u8 ch) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L101) | [Definition](/stdlib/string.qnp?plain=1#L326)
```qinp
\\ Locate the last occurence of a character in a string
\\ @param str The string to search
\\ @param ch The character to search for
\\ @return The address of the last occurence of ch in str, or NULL if not found
```
#### <a id="ref_ee07764fe157d5dabaa4c07f6acd4200"/>fn\<u8*\> std.strrev(u8* str)
> [Declaration](/stdlib/string.qnp?plain=1#L131) | [Definition](/stdlib/string.qnp?plain=1#L383)
```qinp
\\ Reverse a null-terminated string (in-place)
\\ @param str The null-terminated string to reverse
\\ @return The reversed string
```
#### <a id="ref_c64e1e2f0896b5ad920d1b505678868c"/>fn\<u8*\> std.strrev(u8* begin, u8* end)
> [Declaration](/stdlib/string.qnp?plain=1#L137) | [Definition](/stdlib/string.qnp?plain=1#L390)
```qinp
\\ Reverse a string of specified length (in-place)
\\ @param begin Pointer to the first character of the string to reverse
\\ @param end Pointer to the character after the last character of the string to reverse (usually the null-terminator)
\\ @return The reversed string
```
#### <a id="ref_742976fd072ebf52810469994fceaf7a"/>fn\<u8 const*\> std.strrstr(u8 const* str, u8 const* substr) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L113) | [Definition](/stdlib/string.qnp?plain=1#L351)
```qinp
\\ Locate the last occurence of a substring in a string
\\ @param str The string to search
\\ @param substr The substring to search for
\\ @return The address of the last occurence of substr in str, or NULL if not found
```
#### <a id="ref_4b0988906bc64bc9a0b8f8e45c79a931"/>fn\<u8*\> std.strset(u8* str, u8 ch)
> [Declaration](/stdlib/string.qnp?plain=1#L119) | [Definition](/stdlib/string.qnp?plain=1#L365)
```qinp
\\ Set all characters in a string to a specified character
\\ @param str The string to modify
\\ @param ch The character to set
\\ @return The pointer to the passed string
```
#### <a id="ref_88f5086e5de800710b21fc4b8b9a8a85"/>fn\<u8*\> std.strset(u8* str, u8 ch, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L126) | [Definition](/stdlib/string.qnp?plain=1#L374)
```qinp
\\ Set the first num characters of a string to a specified character
\\ @param str The string to modify
\\ @param ch The character to set
\\ @param num The number of characters to set
\\ @return The pointer to the passed string
```
#### <a id="ref_1922b1a8a0bb85f3014f153cee60b556"/>fn\<u8 const*\> std.strstr(u8 const* str, u8 const* substr) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L107) | [Definition](/stdlib/string.qnp?plain=1#L338)
```qinp
\\ Locate the first occurence of a substring in a string
\\ @param str The string to search
\\ @param substr The substring to search for
\\ @return The address of the first occurence of substr in str, or NULL if not found
```
#### <a id="ref_af3d3457b0253b744dbf22d56473821a"/>fn\<u8*\> std.strtok(u8* str, u8 const* delimiters) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L146) | [Definition](/stdlib/string.qnp?plain=1#L404)
```qinp
\\ Split a string into tokens
\\ On the first call pass the string to split, on subsequent calls pass null.
\\ The returned string is the next token in the string, or null if there are no more tokens.
\\ All found delimiters are replaced with null terminators.
\\ @param str The string to split
\\ @param delimiters The delimiters to split on
\\ @return A pointer to the next token
```
#### <a id="ref_39576ab55d28ad0658bcc099a94933d8"/>fn\<u8*\> std.strupr(u8* str)
> [Declaration](/stdlib/string.qnp?plain=1#L30) | [Definition](/stdlib/string.qnp?plain=1#L220)
```qinp
\\ Convert a string to uppercase (in place)
\\ @param str The string to convert
\\ @return The pointer to the passed string
```
#### <a id="ref_285d5e0a52bc36147b65113b2f871ac5"/>fn\<u8*\> std.strupr(u8* str, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L37) | [Definition](/stdlib/string.qnp?plain=1#L229)
```qinp
\\ Convert the first num characters of a string to uppercase (in place)
\\ Does not check for null terminator
\\ @param str The string to convert
\\ @param num The number of characters to convert
\\ @return The pointer to the passed string
```
#### <a id="ref_39608303a9ea8c9a53b85e065cd581ca"/>fn\<u8\> std.tolower(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L169) | [Definition](/stdlib/string.qnp?plain=1#L469)
```qinp
\\ Converts a character to lowercase. Values not in the range 'A' to 'Z' are unchanged.
\\ @param c The character to convert
\\ @return The lowercase character
```
#### <a id="ref_d5143f66f35780c2301272595fa2e37e"/>fn\<u8\> std.toupper(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L164) | [Definition](/stdlib/string.qnp?plain=1#L466)
```qinp
\\ Converts a character to uppercase. Values not in the range 'a' to 'z' are unchanged.
\\ @param c The character to convert
\\ @return The uppercase character
```

