
# Stdlib - stdlib/string.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn<bool> std.isalnum(u8 c) nodiscard](#ref_fe9a83d9c4ccd2be5e7ebe64ef638e38)
 - [fn<bool> std.isalpha(u8 c) nodiscard](#ref_c6066bc2d83c1afa00d730e1a5b207b3)
 - [fn<bool> std.isnum(u8 c) nodiscard](#ref_e77082388c98eeb19dfdf13f712b6f69)
 - [fn<bool> std.isspace(u8 c) nodiscard](#ref_a2e1d141a550291148701b652d6feb2a)
 - [fn<u8*> std.itos(i64 num, u8* str, i64 base) nodiscard](#ref_602a9dedd9e0521d28cf38b8b257ce22)
 - [fn<i64> std.stoi(u8 const* str, i64 base) nodiscard](#ref_ce823ff2ca831f415598ba48246a6549)
 - [fn<u8*> std.strcat(u8* dest, u8 const* src)](#ref_b853496b032c60c71f2bee16bd397daf)
 - [fn<u8*> std.strcat(u8* dest, u8 const* src, u64 num)](#ref_99fa5fbcb4d2f79f6a819fc513abac96)
 - [fn<u8 const*> std.strchr(u8 const* str, u8 ch) nodiscard](#ref_abb24f44b645fe264002cc1d7d4cc54e)
 - [fn<i64> std.strcmp(u8 const* str1, u8 const* str2) nodiscard](#ref_c22b5ad30e1e52431329c1f7b07b7da2)
 - [fn<i64> std.strcmp(u8 const* str1, u8 const* str2, u64 num) nodiscard](#ref_2864713ed02b032bf649b0b8b561b8c4)
 - [fn<i64> std.strcmpi(u8 const* str1, u8 const* str2) nodiscard](#ref_dc2791c73aeb147efe6950a21742fec1)
 - [fn<i64> std.strcmpi(u8 const* str1, u8 const* str2, u64 num) nodiscard](#ref_b04926d106f89494aa2d66705681acd0)
 - [fn<u8*> std.strcpy(u8* dest, u8 const* src)](#ref_08d1e3304c63f40fae23c901485180b8)
 - [fn<u8*> std.strcpy(u8* dest, u8 const* src, u64 num)](#ref_d9e866c63ecedaa8ff6e69aed208eec3)
 - [fn<u64> std.strlen(u8 const* str) nodiscard](#ref_18048b43676094d5ed6ad668c3786275)
 - [fn<u8*> std.strlwr(u8* str)](#ref_aa8ea91739bfc6f43955757164f8eda5)
 - [fn<u8*> std.strlwr(u8* str, u64 num)](#ref_dad4cb6fb817ef138d420c625bf6c215)
 - [fn<u8 const*> std.strrchr(u8 const* str, u8 ch) nodiscard](#ref_60fa0294c8e1928afa9b01d6a34a6739)
 - [fn<u8*> std.strrev(u8* str)](#ref_a59fe38a73d73c37fcdad000160aa4ca)
 - [fn<u8*> std.strrev(u8* begin, u8* end)](#ref_65603eea7fad733a72e4cef38cceaa3b)
 - [fn<u8 const*> std.strrstr(u8 const* str, u8 const* substr) nodiscard](#ref_25124e5515ade551f863aa39cf95c681)
 - [fn<u8*> std.strset(u8* str, u8 ch)](#ref_d3574ee54ce76a279df9a18e0ac0fdab)
 - [fn<u8*> std.strset(u8* str, u8 ch, u64 num)](#ref_0e37c79f291c006550cd1b805992c57b)
 - [fn<u8 const*> std.strstr(u8 const* str, u8 const* substr) nodiscard](#ref_d9a438679e952728ba4bc59248a0408f)
 - [fn<u8*> std.strtok(u8* str, u8 const* delimiters) nodiscard](#ref_ed8badd4acecc61a14fc609eab7a10d3)
 - [fn<u8*> std.strupr(u8* str)](#ref_74c31d610e3abc965bd33e0eecffc6d3)
 - [fn<u8*> std.strupr(u8* str, u64 num)](#ref_56c9338a00add50df7b348ec3a6a57f1)
 - [fn<u8> std.tolower(u8 c) nodiscard](#ref_67783fb9ae6f501a3a31880c498385f6)
 - [fn<u8> std.toupper(u8 c) nodiscard](#ref_dbe44cd709c6558a6db8e50f7491693d)

## Macros
 - std.__BASE_CHARS_LOWER

## Details
#### <a id="ref_fe9a83d9c4ccd2be5e7ebe64ef638e38"/>fn<bool> std.isalnum(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L182) | [Definition](/stdlib/string.qnp?plain=1#L380)
```qinp
\\ Returns wether a character is a letter or a digit
\\ @param c The character to check
\\ @return True if the character is a letter or a digit, false otherwise
```
#### <a id="ref_c6066bc2d83c1afa00d730e1a5b207b3"/>fn<bool> std.isalpha(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L177) | [Definition](/stdlib/string.qnp?plain=1#L377)
```qinp
\\ Returns wether a character is a letter
\\ @param c The character to check
\\ @return True if the character is a letter, false otherwise
```
#### <a id="ref_e77082388c98eeb19dfdf13f712b6f69"/>fn<bool> std.isnum(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L172) | [Definition](/stdlib/string.qnp?plain=1#L374)
```qinp
\\ Returns wether a character is a number
\\ @param c The character to check
\\ @return True if the character is a number, false otherwise
```
#### <a id="ref_a2e1d141a550291148701b652d6feb2a"/>fn<bool> std.isspace(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L187) | [Definition](/stdlib/string.qnp?plain=1#L383)
```qinp
\\ Returns wether a character is a whitespace character
\\ @param c The character to check
\\ @return True if the character is a whitespace character, false otherwise
```
#### <a id="ref_602a9dedd9e0521d28cf38b8b257ce22"/>fn<u8*> std.itos(i64 num, u8* str, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L151) | [Definition](/stdlib/string.qnp?plain=1#L337)
```qinp
\\ Convert an integer to a string
\\ @param num The integer to convert
\\ @param str The string to store the converted integer in
\\ @param base The base to use for the conversion (Valid values are 2 to 36)
\\ @return The null-terminated string representation of the integer
```
#### <a id="ref_ce823ff2ca831f415598ba48246a6549"/>fn<i64> std.stoi(u8 const* str, i64 base) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L157) | [Definition](/stdlib/string.qnp?plain=1#L355)
```qinp
\\ Convert a string to a base-10 integer
\\ @param str The string to convert
\\ @param base The base to use for the conversion (Valid values are 2 to 36)
\\ @return The integer represented by the string
```
#### <a id="ref_b853496b032c60c71f2bee16bd397daf"/>fn<u8*> std.strcat(u8* dest, u8 const* src)
> [Declaration](/stdlib/string.qnp?plain=1#L41) | [Definition](/stdlib/string.qnp?plain=1#L216)
```qinp
\\ Concatenates two strings
\\ @param dest The destination string to concatenate to
\\ @param src The source string to concatenate
\\ @return The pointer to the destination string
```
#### <a id="ref_99fa5fbcb4d2f79f6a819fc513abac96"/>fn<u8*> std.strcat(u8* dest, u8 const* src, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L48) | [Definition](/stdlib/string.qnp?plain=1#L222)
```qinp
\\ Concatenates two strings
\\ @param dest The destination string to concatenate to
\\ @param src The source string to concatenate
\\ @param len The number of characters to concatenate (excluding the null terminator)
\\ @return The pointer to the destination string
```
#### <a id="ref_abb24f44b645fe264002cc1d7d4cc54e"/>fn<u8 const*> std.strchr(u8 const* str, u8 ch) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L93) | [Definition](/stdlib/string.qnp?plain=1#L264)
```qinp
\\ Locate the first occurencce of a character in a string
\\ @param str The string to search
\\ @param ch The character to search for
\\ @return The address of the first occurence of ch in str, or NULL if not found
```
#### <a id="ref_c22b5ad30e1e52431329c1f7b07b7da2"/>fn<i64> std.strcmp(u8 const* str1, u8 const* str2) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L67) | [Definition](/stdlib/string.qnp?plain=1#L240)
```qinp
\\ Compare two null-terminated strings
\\ @param str1 The first string
\\ @param str2 The second string
\\ @return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_2864713ed02b032bf649b0b8b561b8c4"/>fn<i64> std.strcmp(u8 const* str1, u8 const* str2, u64 num) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L74) | [Definition](/stdlib/string.qnp?plain=1#L246)
```qinp
\\ Compare the first num characters of two strings
\\ @param str1 The first string
\\ @param str2 The second string
\\ @param num The number of characters to compare
\\ @return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_dc2791c73aeb147efe6950a21742fec1"/>fn<i64> std.strcmpi(u8 const* str1, u8 const* str2) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L80) | [Definition](/stdlib/string.qnp?plain=1#L252)
```qinp
\\ Compare two null-terimated strings without case sensitivity
\\ @param str1 The first string
\\ @param str2 The second string
\\ @return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_b04926d106f89494aa2d66705681acd0"/>fn<i64> std.strcmpi(u8 const* str1, u8 const* str2, u64 num) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L87) | [Definition](/stdlib/string.qnp?plain=1#L258)
```qinp
\\ Compare the first num characters of two strings without case sensitivity
\\ @param str1 The first string
\\ @param str2 The second string
\\ @param num The number of characters to compare
\\ @return 0 if the strings are equal, < 0 if str1 is less than str2, > 0 if str1 is greater than str2
```
#### <a id="ref_08d1e3304c63f40fae23c901485180b8"/>fn<u8*> std.strcpy(u8* dest, u8 const* src)
> [Declaration](/stdlib/string.qnp?plain=1#L54) | [Definition](/stdlib/string.qnp?plain=1#L228)
```qinp
\\ Copy a null-terminated string
\\ @param dest The destination buffer
\\ @param src The source string
\\ @return The destination buffer
```
#### <a id="ref_d9e866c63ecedaa8ff6e69aed208eec3"/>fn<u8*> std.strcpy(u8* dest, u8 const* src, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L61) | [Definition](/stdlib/string.qnp?plain=1#L234)
```qinp
\\ Copy the first num characters of a string
\\ @param dest The destination buffer
\\ @param src The source string
\\ @param len The number of characters to copy (excluding the null terminator)
\\ @return The destination buffer
```
#### <a id="ref_18048b43676094d5ed6ad668c3786275"/>fn<u64> std.strlen(u8 const* str) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L11) | [Definition](/stdlib/string.qnp?plain=1#L191)
```qinp
\\ Determine the length of a string (without the null terminator)
\\ @param str The null-terminated string to measure
\\ @return The length of the string
```
#### <a id="ref_aa8ea91739bfc6f43955757164f8eda5"/>fn<u8*> std.strlwr(u8* str)
> [Declaration](/stdlib/string.qnp?plain=1#L16) | [Definition](/stdlib/string.qnp?plain=1#L196)
```qinp
\\ Convert a string to lowercase (in place)
\\ @param str The string to convert
\\ @return The pointer to the passed string
```
#### <a id="ref_dad4cb6fb817ef138d420c625bf6c215"/>fn<u8*> std.strlwr(u8* str, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L23) | [Definition](/stdlib/string.qnp?plain=1#L201)
```qinp
\\ Convert the first num characters of a string to lowercase (in place)
\\ Does not check for null terminator
\\ @param str The string to convert
\\ @param num The number of characters to convert
\\ @return The pointer to the passed string
```
#### <a id="ref_60fa0294c8e1928afa9b01d6a34a6739"/>fn<u8 const*> std.strrchr(u8 const* str, u8 ch) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L99) | [Definition](/stdlib/string.qnp?plain=1#L268)
```qinp
\\ Locate the last occurence of a character in a string
\\ @param str The string to search
\\ @param ch The character to search for
\\ @return The address of the last occurence of ch in str, or NULL if not found
```
#### <a id="ref_a59fe38a73d73c37fcdad000160aa4ca"/>fn<u8*> std.strrev(u8* str)
> [Declaration](/stdlib/string.qnp?plain=1#L129) | [Definition](/stdlib/string.qnp?plain=1#L305)
```qinp
\\ Reverse a null-terminated string (in-place)
\\ @param str The null-terminated string to reverse
\\ @return The reversed string
```
#### <a id="ref_65603eea7fad733a72e4cef38cceaa3b"/>fn<u8*> std.strrev(u8* begin, u8* end)
> [Declaration](/stdlib/string.qnp?plain=1#L135) | [Definition](/stdlib/string.qnp?plain=1#L308)
```qinp
\\ Reverse a string of specified length (in-place)
\\ @param begin Pointer to the first character of the string to reverse
\\ @param end Pointer to the character after the last character of the string to reverse (usually the null-terminator)
\\ @return The reversed string
```
#### <a id="ref_25124e5515ade551f863aa39cf95c681"/>fn<u8 const*> std.strrstr(u8 const* str, u8 const* substr) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L111) | [Definition](/stdlib/string.qnp?plain=1#L285)
```qinp
\\ Locate the last occurence of a substring in a string
\\ @param str The string to search
\\ @param substr The substring to search for
\\ @return The address of the last occurence of substr in str, or NULL if not found
```
#### <a id="ref_d3574ee54ce76a279df9a18e0ac0fdab"/>fn<u8*> std.strset(u8* str, u8 ch)
> [Declaration](/stdlib/string.qnp?plain=1#L117) | [Definition](/stdlib/string.qnp?plain=1#L295)
```qinp
\\ Set all characters in a string to a specified character
\\ @param str The string to modify
\\ @param ch The character to set
\\ @return The pointer to the passed string
```
#### <a id="ref_0e37c79f291c006550cd1b805992c57b"/>fn<u8*> std.strset(u8* str, u8 ch, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L124) | [Definition](/stdlib/string.qnp?plain=1#L300)
```qinp
\\ Set the first num characters of a string to a specified character
\\ @param str The string to modify
\\ @param ch The character to set
\\ @param num The number of characters to set
\\ @return The pointer to the passed string
```
#### <a id="ref_d9a438679e952728ba4bc59248a0408f"/>fn<u8 const*> std.strstr(u8 const* str, u8 const* substr) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L105) | [Definition](/stdlib/string.qnp?plain=1#L276)
```qinp
\\ Locate the first occurence of a substring in a string
\\ @param str The string to search
\\ @param substr The substring to search for
\\ @return The address of the first occurence of substr in str, or NULL if not found
```
#### <a id="ref_ed8badd4acecc61a14fc609eab7a10d3"/>fn<u8*> std.strtok(u8* str, u8 const* delimiters) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L144) | [Definition](/stdlib/string.qnp?plain=1#L318)
```qinp
\\ Split a string into tokens
\\ On the first call pass the string to split, on subsequent calls pass null.
\\ The returned string is the next token in the string, or null if there are no more tokens.
\\ All found delimiters are replaced with null terminators.
\\ @param str The string to split
\\ @param delimiters The delimiters to split on
\\ @return A pointer to the next token
```
#### <a id="ref_74c31d610e3abc965bd33e0eecffc6d3"/>fn<u8*> std.strupr(u8* str)
> [Declaration](/stdlib/string.qnp?plain=1#L28) | [Definition](/stdlib/string.qnp?plain=1#L206)
```qinp
\\ Convert a string to uppercase (in place)
\\ @param str The string to convert
\\ @return The pointer to the passed string
```
#### <a id="ref_56c9338a00add50df7b348ec3a6a57f1"/>fn<u8*> std.strupr(u8* str, u64 num)
> [Declaration](/stdlib/string.qnp?plain=1#L35) | [Definition](/stdlib/string.qnp?plain=1#L211)
```qinp
\\ Convert the first num characters of a string to uppercase (in place)
\\ Does not check for null terminator
\\ @param str The string to convert
\\ @param num The number of characters to convert
\\ @return The pointer to the passed string
```
#### <a id="ref_67783fb9ae6f501a3a31880c498385f6"/>fn<u8> std.tolower(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L167) | [Definition](/stdlib/string.qnp?plain=1#L371)
```qinp
\\ Converts a character to lowercase. Values not in the range 'A' to 'Z' are unchanged.
\\ @param c The character to convert
\\ @return The lowercase character
```
#### <a id="ref_dbe44cd709c6558a6db8e50f7491693d"/>fn<u8> std.toupper(u8 c) nodiscard
> [Declaration](/stdlib/string.qnp?plain=1#L162) | [Definition](/stdlib/string.qnp?plain=1#L368)
```qinp
\\ Converts a character to uppercase. Values not in the range 'a' to 'z' are unchanged.
\\ @param c The character to convert
\\ @return The uppercase character
```

