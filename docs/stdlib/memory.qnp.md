
# Stdlib - stdlib/memory.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<\> std.free(void* pBuff) ...](#ref_953a0a3a352127b01d83889b86ce45f7)
 - [fn\<void*\> std.malloc(u64 size) nodiscard ...](#ref_10f8f64c1590f95440f24093373de61c)
 - [fn\<i64\> std.memcmp(void const* a, void const* b, u64 len) nodiscard](#ref_f92803298a2f8a1c3ab0e70d68068d28)
 - [fn\<void*\> std.memcpy(void* dest, void const* src, u64 len)](#ref_1d215a9dab34b1a5e7ae3498a51292e5)
 - [fn\<void*\> std.memset(void* dest, u8 val, u64 len)](#ref_136a1f6851b37081a15590abe2901f01)
 - [fn\<void*\> std.realloc(void* pBuff, u64 size) nodiscard ...](#ref_28da28462f0cfd1db599ca703c7a652b)

## Macros
 - std.__MIN_MEM_LEN_USE_ALIGN

## Details
#### <a id="ref_953a0a3a352127b01d83889b86ce45f7"/>fn\<\> std.free(void* pBuff) ...
> [Declaration](/stdlib/memory.qnp?plain=1#L44) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L303)
```qinp
\\ Free a buffer
\\ @param pBuff The buffer to free
```
#### <a id="ref_10f8f64c1590f95440f24093373de61c"/>fn\<void*\> std.malloc(u64 size) nodiscard ...
> [Declaration](/stdlib/memory.qnp?plain=1#L34) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L233)
```qinp
\\ Allocate a buffer of a given size
\\ @param size The size of the buffer to allocate
\\ @return The allocated buffer (null if failed)
```
#### <a id="ref_f92803298a2f8a1c3ab0e70d68068d28"/>fn\<i64\> std.memcmp(void const* a, void const* b, u64 len) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L29) | [Definition](/stdlib/memory.qnp?plain=1#L110)
```qinp
\\ Compare two buffers
\\ @param a The first buffer
\\ @param b The second buffer
\\ @param num The number of bytes to compare
\\ @return 0 if the buffers are equal, < 0 if the first byte that differs in a is less than in b, > 0 if the first byte that differs in a is greater than in b
```
#### <a id="ref_1d215a9dab34b1a5e7ae3498a51292e5"/>fn\<void*\> std.memcpy(void* dest, void const* src, u64 len)
> [Declaration](/stdlib/memory.qnp?plain=1#L15) | [Definition](/stdlib/memory.qnp?plain=1#L50)
```qinp
\\ Copy data from one buffer to another
\\ Undefined behavior if the buffers overlap
\\ @param dest The destination buffer
\\ @param src The source buffer
\\ @param num The number of bytes to copy
\\ @return The destination buffer
```
#### <a id="ref_136a1f6851b37081a15590abe2901f01"/>fn\<void*\> std.memset(void* dest, u8 val, u64 len)
> [Declaration](/stdlib/memory.qnp?plain=1#L22) | [Definition](/stdlib/memory.qnp?plain=1#L79)
```qinp
\\ Set every byte in a buffer to a value
\\ @param dest The destination buffer
\\ @param val The value to set every byte to
\\ @param num The number of bytes to set
\\ @return The destination buffer
```
#### <a id="ref_28da28462f0cfd1db599ca703c7a652b"/>fn\<void*\> std.realloc(void* pBuff, u64 size) nodiscard ...
> [Declaration](/stdlib/memory.qnp?plain=1#L40) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L255)
```qinp
\\ Reallocate a buffer to a given size
\\ @param pBuff The pointer to the block of memory to reallocate (Must be previously allocated by malloc)
\\ @param size The new size of the buffer
\\ @return The pointer to the newly allocated memory.
```

