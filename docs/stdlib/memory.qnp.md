
# Stdlib - stdlib/memory.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn<> std.free(void* pBuff) ...](#ref_66d06a03afaac7bb74892566623ddaa3)
 - [fn<void*> std.malloc(u64 size) nodiscard ...](#ref_21d3a6b5ac4cde7b035ba150c56cb311)
 - [fn<i64> std.memcmp(void const* a, void const* b, u64 len) nodiscard](#ref_5b3a21754eaa17fa6bc96a9ef2ddfeb5)
 - [fn<void*> std.memcpy(void* dest, void const* src, u64 len)](#ref_6162996f10387f36467d144df5df45f6)
 - [fn<void*> std.memset(void* dest, u8 val, u64 len)](#ref_7250513a17aa94baea9c7c7b58d6df31)
 - [fn<void*> std.realloc(void* pBuff, u64 size) nodiscard ...](#ref_923326336bf1d21529fc06af7cdf46fa)

## Macros
 - std.__MIN_MEM_LEN_USE_ALIGN

## Details
#### <a id="ref_66d06a03afaac7bb74892566623ddaa3"/>fn<> std.free(void* pBuff) ...
> [Declaration](/stdlib/memory.qnp?plain=1#L42) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L275)
```qinp
\\ Free a buffer
\\ @param pBuff The buffer to free
```
#### <a id="ref_21d3a6b5ac4cde7b035ba150c56cb311"/>fn<void*> std.malloc(u64 size) nodiscard ...
> [Declaration](/stdlib/memory.qnp?plain=1#L32) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L207)
```qinp
\\ Allocate a buffer of a given size
\\ @param size The size of the buffer to allocate
\\ @return The allocated buffer (null if failed)
```
#### <a id="ref_5b3a21754eaa17fa6bc96a9ef2ddfeb5"/>fn<i64> std.memcmp(void const* a, void const* b, u64 len) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L27) | [Definition](/stdlib/memory.qnp?plain=1#L100)
```qinp
\\ Compare two buffers
\\ @param a The first buffer
\\ @param b The second buffer
\\ @param num The number of bytes to compare
\\ @return 0 if the buffers are equal, < 0 if the first byte that differs in a is less than in b, > 0 if the first byte that differs in a is greater than in b
```
#### <a id="ref_6162996f10387f36467d144df5df45f6"/>fn<void*> std.memcpy(void* dest, void const* src, u64 len)
> [Declaration](/stdlib/memory.qnp?plain=1#L13) | [Definition](/stdlib/memory.qnp?plain=1#L48)
```qinp
\\ Copy data from one buffer to another
\\ Undefined behavior if the buffers overlap
\\ @param dest The destination buffer
\\ @param src The source buffer
\\ @param num The number of bytes to copy
\\ @return The destination buffer
```
#### <a id="ref_7250513a17aa94baea9c7c7b58d6df31"/>fn<void*> std.memset(void* dest, u8 val, u64 len)
> [Declaration](/stdlib/memory.qnp?plain=1#L20) | [Definition](/stdlib/memory.qnp?plain=1#L73)
```qinp
\\ Set every byte in a buffer to a value
\\ @param dest The destination buffer
\\ @param val The value to set every byte to
\\ @param num The number of bytes to set
\\ @return The destination buffer
```
#### <a id="ref_923326336bf1d21529fc06af7cdf46fa"/>fn<void*> std.realloc(void* pBuff, u64 size) nodiscard ...
> [Declaration](/stdlib/memory.qnp?plain=1#L38) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L227)
```qinp
\\ Reallocate a buffer to a given size
\\ @param pBuff The pointer to the block of memory to reallocate (Must be previously allocated by malloc)
\\ @param size The new size of the buffer
\\ @return The pointer to the newly allocated memory.
```

