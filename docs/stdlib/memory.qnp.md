
# Stdlib - stdlib/memory.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<T\> std.__new(T nullptr) nodiscard](#ref_fd4e34556266afbf28a2530306c3b620)
 - [fn\<T\> std.__new(T nullptr, ...) nodiscard](#ref_1b1a66ddba6c1ffceb57e8868df1f299)
 - [fn\<\> std.delete(T obj)](#ref_b4b370d89065599646db728365d3b95b)
 - [fn\<\> std.free(void* ptr) ...](#ref_180a442d5d592aa64e4af0d74fd702f4)
 - [fn\<\> std.freec(void** ppBuff)](#ref_2d0b452532be61bc0522821f9a33bb5e)
 - [fn\<void*\> std.malloc(u64 size) nodiscard ...](#ref_10f8f64c1590f95440f24093373de61c)
 - [fn\<i64\> std.memcmp(void const* a, void const* b, u64 len) nodiscard](#ref_f92803298a2f8a1c3ab0e70d68068d28)
 - [fn\<void*\> std.memcpy(void* dest, void const* src, u64 len)](#ref_1d215a9dab34b1a5e7ae3498a51292e5)
 - [fn\<void*\> std.memset(void* dest, u8 val, u64 len)](#ref_136a1f6851b37081a15590abe2901f01)
 - [fn\<void*\> std.realloc(void* ptr, u64 newSize) nodiscard ...](#ref_bed44dc95cf9c18a87c9d24751c94420)

## Macros
 - [std.__MIN_MEM_LEN_USE_ALIGN](#ref_4c4be01f49513fd8a5af9cf19d0f4f75)
 - std.new(T)

## Details
#### <a id="ref_4c4be01f49513fd8a5af9cf19d0f4f75"/>std.__MIN_MEM_LEN_USE_ALIGN
> [Declaration](/stdlib/memory.qnp?plain=1#L10)
```qinp
Internal constant
Specifies minimum length of memory to use aligned algorithms on
```
#### <a id="ref_ca5a8074486abaa71f826c96ee6e452d"/>T nullptr
> [Declaration](/stdlib/memory.qnp?plain=1#L197)
```qinp
TODO: Future implementation
\\fn<?T*> new(...) nodiscard!:
\\	return null
```
#### <a id="ref_fd4e34556266afbf28a2530306c3b620"/>fn\<T\> std.__new(T nullptr) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L61) | [Definition](/stdlib/memory.qnp?plain=1#L182)
```qinp
TODO: Future implementation
Allocate memory for a class and call its constructor
@param nullptr A null pointer specifying the type of the object to allocate
@return The allocated object (null if failed)
@note The object must be freed with delete
\\fn<?T*> new() nodiscard...
```
#### <a id="ref_1b1a66ddba6c1ffceb57e8868df1f299"/>fn\<T\> std.__new(T nullptr, ...) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L70) | [Definition](/stdlib/memory.qnp?plain=1#L197)
```qinp
TODO: Future implementation
Allocate memory for a class and call its constructor with the given args
@param nullptr A null pointer specifying the type of the object to allocate
@param ... The arguments to pass to the constructor
@return The allocated object (null if failed)
@note The object must be freed with delete
\\fn<?T*> new(...) nodiscard...
```
#### <a id="ref_68b271aa96cfc881c456e4270fa049ed"/>T obj
> [Declaration](/stdlib/memory.qnp?plain=1#L212)
```qinp
TODO: Future implementation
\\fn<> delete(?T* obj)!:
\\	return
```
#### <a id="ref_b4b370d89065599646db728365d3b95b"/>fn\<\> std.delete(T obj)
> [Declaration](/stdlib/memory.qnp?plain=1#L77) | [Definition](/stdlib/memory.qnp?plain=1#L212)
```qinp
TODO: Future implementation
Call an objects constructor and free its memory
@param obj The object to free
@note The object must have been allocated with new
\\fn<> delete(?T* obj)...
```
#### <a id="ref_180a442d5d592aa64e4af0d74fd702f4"/>fn\<\> std.free(void* ptr) ...
> [Declaration](/stdlib/memory.qnp?plain=1#L47) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L239)
```qinp
Free a buffer
@param pBuff The buffer to free
```
#### <a id="ref_2d0b452532be61bc0522821f9a33bb5e"/>fn\<\> std.freec(void** ppBuff)
> [Declaration](/stdlib/memory.qnp?plain=1#L51) | [Definition](/stdlib/memory.qnp?plain=1#L172)
```qinp
Free a buffer and set the pointer to null
@param ppBuff Pointer to the buffer to free
```
#### <a id="ref_10f8f64c1590f95440f24093373de61c"/>fn\<void*\> std.malloc(u64 size) nodiscard ...
> [Declaration](/stdlib/memory.qnp?plain=1#L37) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L220)
```qinp
Allocate a buffer of a given size
@param size The size of the buffer to allocate
@return The allocated buffer (null if failed)
```
#### <a id="ref_f92803298a2f8a1c3ab0e70d68068d28"/>fn\<i64\> std.memcmp(void const* a, void const* b, u64 len) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L32) | [Definition](/stdlib/memory.qnp?plain=1#L141)
```qinp
Compare two buffers
@param a The first buffer
@param b The second buffer
@param num The number of bytes to compare
@return 0 if the buffers are equal, < 0 if the first byte that differs in a is less than in b, > 0 if the first byte that differs in a is greater than in b
```
#### <a id="ref_1d215a9dab34b1a5e7ae3498a51292e5"/>fn\<void*\> std.memcpy(void* dest, void const* src, u64 len)
> [Declaration](/stdlib/memory.qnp?plain=1#L18) | [Definition](/stdlib/memory.qnp?plain=1#L81)
```qinp
Copy data from one buffer to another
Undefined behavior if the buffers overlap
@param dest The destination buffer
@param src The source buffer
@param num The number of bytes to copy
@return The destination buffer
```
#### <a id="ref_136a1f6851b37081a15590abe2901f01"/>fn\<void*\> std.memset(void* dest, u8 val, u64 len)
> [Declaration](/stdlib/memory.qnp?plain=1#L25) | [Definition](/stdlib/memory.qnp?plain=1#L110)
```qinp
Set every byte in a buffer to a value
@param dest The destination buffer
@param val The value to set every byte to
@param num The number of bytes to set
@return The destination buffer
```
#### <a id="ref_bed44dc95cf9c18a87c9d24751c94420"/>fn\<void*\> std.realloc(void* ptr, u64 newSize) nodiscard ...
> [Declaration](/stdlib/memory.qnp?plain=1#L43) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L234)
```qinp
Reallocate a buffer to a given size
@param pBuff The pointer to the block of memory to reallocate (Must be previously allocated by malloc)
@param size The new size of the buffer
@return The pointer to the newly allocated memory.
```

