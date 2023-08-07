
# Stdlib - stdlib/platform/linux/memory.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)
 - [Details](#details)


## Globals
 - [var\<std.__MemBlockHeader*\> std.__memBlocks](#ref_d94fdb461948cf19fa7fd5656137a6f8)

## Functions
 - [fn\<\> std.__addBlockToList(std.__MemBlockHeader* block)](#ref_5da1a02c15d125cc23101707481f1ef8)
 - [fn\<std.__MemBlockHeader*\> std.__allocateBlock(u64 minSize) nodiscard](#ref_705fb981bb6b23b1dea5efcbdd13dfb6)
 - [fn\<u64\> std.__brk(u64 size)](#ref_d968550035e40ca21779cf81106ff600)
 - [fn\<std.__MemBlockHeader*\> std.__getFreeBlock(u64 minSize) nodiscard](#ref_f9a4ffbc6d72db8030b9fc8578428e48)
 - [fn\<std.__MemBlockHeader*\> std.__mergeBlocks(std.__MemBlockHeader* block)](#ref_8170e06f3f23cd983bf381b8ecfd2718)
 - [fn\<\> std.__removeBlockFromList(std.__MemBlockHeader* block)](#ref_7dd915a43c9073af308cb2c6395ff9fb)
 - [fn\<void*\> std.__sbrk(i64 increment)](#ref_9ef372736f3a8d4ad83e20374764ab82)
 - [fn\<\> std.__truncateBlock(std.__MemBlockHeader* block, u64 newBodySize)](#ref_0e7debc4ba8de34a39ded8b6dd23549b)
 - [fn\<\> std.free(void* ptr)](#ref_ff55c5d36de2ef6e1236d38f68acf334)
 - [fn\<void*\> std.malloc(u64 size) nodiscard](#ref_ed884f46d3fc4317f94703c1c65177b4)
 - [fn\<void*\> std.realloc(void* ptr, u64 newSize) nodiscard](#ref_d356f92991930cf687ff1bc9d3a236b1)

## Packs/Unions
 - [pack std.__MemBlockHeader](#ref_fea58120e31b62b4fa2945aed315480d)

## Macros
 - std.__FREE_PAGES_COUNT_MAX
 - std.__MEM_ALIGNMENT_SIZE
 - std.__PAGE_SIZE

## Details
#### <a id="ref_fea58120e31b62b4fa2945aed315480d"/>pack std.__MemBlockHeader
> [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L16)
```qinp
The header for every allocated block of memory (used for freeing/keeping track of allocated, unused memory)
```
#### <a id="ref_5da1a02c15d125cc23101707481f1ef8"/>fn\<\> std.__addBlockToList(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L52) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L152)
```qinp
Frees the given block of memory.
If the block is adjacent to other free blocks, they are merged.
If the block is at the end of the data segment and above a certain size, the data segment is shrunk.
@param block The block to free
```
#### <a id="ref_705fb981bb6b23b1dea5efcbdd13dfb6"/>fn\<std.__MemBlockHeader*\> std.__allocateBlock(u64 minSize) nodiscard
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L40) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L113)
```qinp
Allocates a new block of memory of at least the given size.
@param minSize The minimum size of the block to allocate
@return A pointer to the block (null on failure)
```
#### <a id="ref_d968550035e40ca21779cf81106ff600"/>fn\<u64\> std.__brk(u64 size)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L24) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L72)
```qinp
Changes the data segment size to the given size and returns the new size.
@param size The new size
@return The size after the change (Current size on failure)
```
#### <a id="ref_f9a4ffbc6d72db8030b9fc8578428e48"/>fn\<std.__MemBlockHeader*\> std.__getFreeBlock(u64 minSize) nodiscard
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L35) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L83)
```qinp
Retrieves a free block of memory of at least the given size.
If no such block exists, a new block is allocated.
@param minSize The minimum size of the block to retrieve
@return A pointer to the block (null on failure)
```
#### <a id="ref_d94fdb461948cf19fa7fd5656137a6f8"/>var\<std.__MemBlockHeader*\> std.__memBlocks
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L65)
```qinp
First node in the list of allocated data blocks sorted by address
```
#### <a id="ref_8170e06f3f23cd983bf381b8ecfd2718"/>fn\<std.__MemBlockHeader*\> std.__mergeBlocks(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L62) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L195)
```qinp
Merges the given block with adjacent blocks if possible.
The block must be in the list of free blocks.
@param block The block to merge
@return A pointer to the merged block
```
#### <a id="ref_7dd915a43c9073af308cb2c6395ff9fb"/>fn\<\> std.__removeBlockFromList(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L56) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L185)
```qinp
Removes the given block from the list of free blocks.
@param block The block to remove
```
#### <a id="ref_9ef372736f3a8d4ad83e20374764ab82"/>fn\<void*\> std.__sbrk(i64 increment)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L29) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L75)
```qinp
Changes the data segment size by the given amount and returns a pointer to the newly allocated memory.
@param increment The amount to increase/decrease the data segment size by
@return A pointer pointing to the previous data segment end (null on failure)
```
#### <a id="ref_0e7debc4ba8de34a39ded8b6dd23549b"/>fn\<\> std.__truncateBlock(std.__MemBlockHeader* block, u64 newBodySize)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L46) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L128)
```qinp
Truncates the given block of memory to the given size and adds the remaining memory to the list of free blocks.
@param block The block to split
@param newBodySize The size the first block should have
@return A pointer to the second block (null on failure)
```
#### <a id="ref_ff55c5d36de2ef6e1236d38f68acf334"/>fn\<\> std.free(void* ptr)
> [Declaration](/stdlib/memory.qnp?plain=1#L47) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L239)
```qinp
Free a buffer
@param pBuff The buffer to free
```
#### <a id="ref_ed884f46d3fc4317f94703c1c65177b4"/>fn\<void*\> std.malloc(u64 size) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L37) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L220)
```qinp
Allocate a buffer of a given size
@param size The size of the buffer to allocate
@return The allocated buffer (null if failed)
```
#### <a id="ref_d356f92991930cf687ff1bc9d3a236b1"/>fn\<void*\> std.realloc(void* ptr, u64 newSize) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L43) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L234)
```qinp
Reallocate a buffer to a given size
@param pBuff The pointer to the block of memory to reallocate (Must be previously allocated by malloc)
@param size The new size of the buffer
@return The pointer to the newly allocated memory.
```

