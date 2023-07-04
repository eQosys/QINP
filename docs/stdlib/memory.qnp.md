
# Stdlib - stdlib/memory.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<\> std.free(void* ptr) ...](#ref_180a442d5d592aa64e4af0d74fd702f4)
 - [fn\<\> std.freec(void** ppBuff)](#ref_2d0b452532be61bc0522821f9a33bb5e)
 - [fn\<void*\> std.malloc(u64 size) nodiscard ...](#ref_10f8f64c1590f95440f24093373de61c)
 - [fn\<i64\> std.memcmp(void const* a, void const* b, u64 len) nodiscard](#ref_f92803298a2f8a1c3ab0e70d68068d28)
 - [fn\<void*\> std.memcpy(void* dest, void const* src, u64 len)](#ref_1d215a9dab34b1a5e7ae3498a51292e5)
 - [fn\<void*\> std.memset(void* dest, u8 val, u64 len)](#ref_136a1f6851b37081a15590abe2901f01)
 - [fn\<void*\> std.realloc(void* ptr, u64 newSize) nodiscard ...](#ref_bed44dc95cf9c18a87c9d24751c94420)

## Macros
 - std.__MIN_MEM_LEN_USE_ALIGN

## Details
#### <a id="ref_180a442d5d592aa64e4af0d74fd702f4"/>fn\<\> std.free(void* ptr) ...
> [Declaration](/stdlib/memory.qnp?plain=1#L44) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L235)
```qinp
Free a buffer
@param pBuff The buffer to free
```
#### <a id="ref_2d0b452532be61bc0522821f9a33bb5e"/>fn\<\> std.freec(void** ppBuff)
> [Declaration](/stdlib/memory.qnp?plain=1#L48) | [Definition](/stdlib/memory.qnp?plain=1#L145)
```qinp
Free a buffer and set the pointer to null
@param ppBuff Pointer to the buffer to free
```
#### <a id="ref_10f8f64c1590f95440f24093373de61c"/>fn\<void*\> std.malloc(u64 size) nodiscard ...
> [Declaration](/stdlib/memory.qnp?plain=1#L34) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L217)
```qinp
Allocate a buffer of a given size
@param size The size of the buffer to allocate
@return The allocated buffer (null if failed)
```
#### <a id="ref_f92803298a2f8a1c3ab0e70d68068d28"/>fn\<i64\> std.memcmp(void const* a, void const* b, u64 len) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L29) | [Definition](/stdlib/memory.qnp?plain=1#L114)
```qinp
Compare two buffers
@param a The first buffer
@param b The second buffer
@param num The number of bytes to compare
@return 0 if the buffers are equal, < 0 if the first byte that differs in a is less than in b, > 0 if the first byte that differs in a is greater than in b
```
#### <a id="ref_1d215a9dab34b1a5e7ae3498a51292e5"/>fn\<void*\> std.memcpy(void* dest, void const* src, u64 len)
> [Declaration](/stdlib/memory.qnp?plain=1#L15) | [Definition](/stdlib/memory.qnp?plain=1#L54)
```qinp
Copy data from one buffer to another
Undefined behavior if the buffers overlap
@param dest The destination buffer
@param src The source buffer
@param num The number of bytes to copy
@return The destination buffer
```
#### <a id="ref_136a1f6851b37081a15590abe2901f01"/>fn\<void*\> std.memset(void* dest, u8 val, u64 len)
> [Declaration](/stdlib/memory.qnp?plain=1#L22) | [Definition](/stdlib/memory.qnp?plain=1#L83)
```qinp
Set every byte in a buffer to a value
@param dest The destination buffer
@param val The value to set every byte to
@param num The number of bytes to set
@return The destination buffer
```
#### <a id="ref_bed44dc95cf9c18a87c9d24751c94420"/>fn\<void*\> std.realloc(void* ptr, u64 newSize) nodiscard ...
> [Declaration](/stdlib/memory.qnp?plain=1#L40) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L230)
```qinp
Reallocate a buffer to a given size
@param pBuff The pointer to the block of memory to reallocate (Must be previously allocated by malloc)
@param size The new size of the buffer
@return The pointer to the newly allocated memory.
```

