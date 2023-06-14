
# Stdlib - stdlib/platform/linux/memory.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)
 - [Details](#details)


## Globals
 - var\<std.__MemBlockHeader*\> std.__memBlocks

## Functions
 - [fn\<std.__MemBlockHeader*\> std.__allocBlock(u64 minSizeAvail)](#ref_5284f03abb7acb24462f8a7a937481d7)
 - [fn\<u64\> std.__brk(u64 size)](#ref_d968550035e40ca21779cf81106ff600)
 - [fn\<std.__MemBlockHeader*\> std.__getAvailBlock(u64 minSizeAvail)](#ref_b8c5fcfbee1b566cc69717ae5f71d896)
 - [fn\<std.__MemBlockHeader*\> std.__getNextAdjacentAvailBlock(std.__MemBlockHeader* block)](#ref_4822d47537586388e702754cc7c5c6d4)
 - [fn\<std.__MemBlockHeader*\> std.__insertAvailBlock(std.__MemBlockHeader* block)](#ref_c95af63a38bc955dad2fb2bcc24d0568)
 - [fn\<bool\> std.__isAdjacentToNext(std.__MemBlockHeader const* block)](#ref_cf9261bf2502ba87a6bf1484107500fa)
 - [fn\<bool\> std.__isAdjacentToNext(std.__MemBlockHeader const* block, std.__MemBlockHeader const* next)](#ref_0c8c089dc1a1dba19674f2c657c8cb5b)
 - [fn\<std.__MemBlockHeader*\> std.__mergeBlockWithNeighbors(std.__MemBlockHeader* block)](#ref_e1595932126d7e8bcbe0747eb7b7aff5)
 - [fn\<std.__MemBlockHeader*\> std.__mergeBlockWithNext(std.__MemBlockHeader* block)](#ref_cd1c3a148ca6c80a4ca20106fcd487c2)
 - [fn\<\> std.__removeAvailBlock(std.__MemBlockHeader* block)](#ref_b6d3e3920cc7b0f2330772916ff4fb9d)
 - [fn\<void*\> std.__sbrk(i64 increment)](#ref_9ef372736f3a8d4ad83e20374764ab82)
 - [fn\<std.__MemBlockHeader*\> std.__splitBlock(std.__MemBlockHeader* block, u64 newSizeAvail)](#ref_1dbf5eac888d340741bd2677467ed627)
 - [fn\<\> std.free(void* pBuff)](#ref_d9a2acfa92c7e2a645dc38c9a0695836)
 - [fn\<void*\> std.malloc(u64 size) nodiscard](#ref_ed884f46d3fc4317f94703c1c65177b4)
 - [fn\<void*\> std.realloc(void* pBuff, u64 size) nodiscard](#ref_04a1fa7a62588505a449a3b241dbaab1)

## Packs/Unions
 - pack std.__MemBlockHeader

## Macros
 - std.__FREE_PAGES_COUNT_MAX
 - std.__MEM_ALIGNMENT_SIZE
 - std.__PAGE_SIZE

## Details
#### <a id="ref_5284f03abb7acb24462f8a7a937481d7"/>fn\<std.__MemBlockHeader*\> std.__allocBlock(u64 minSizeAvail)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L83) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L220)
```qinp
\\ Allocates a block of memory padded to a page size while being big enough to hold the given available size and the block header.
\\ @param minSizeAvail The size of the block to allocate
\\ @return A pointer to the header of the allocated block (null on failure)
```
#### <a id="ref_d968550035e40ca21779cf81106ff600"/>fn\<u64\> std.__brk(u64 size)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L22) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L94)
```qinp
\\ Changes the data segment size to the given size and returns the new size.
\\ @param size The new size
\\ @return The size after the change (Current size on failure)
```
#### <a id="ref_b8c5fcfbee1b566cc69717ae5f71d896"/>fn\<std.__MemBlockHeader*\> std.__getAvailBlock(u64 minSizeAvail)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L33) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L105)
```qinp
\\ Returns the smallest block of memory that is large enough to hold the given size.
\\ If no block is large enough, returns the last block in the list.
\\ @param minSizeAvail The available size of the block to find
\\ @return The smallest block of memory that is large enough to hold the given size or the last block in the list
```
#### <a id="ref_4822d47537586388e702754cc7c5c6d4"/>fn\<std.__MemBlockHeader*\> std.__getNextAdjacentAvailBlock(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L38) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L119)
```qinp
\\ Returns the next adjacent block if there is one available
\\ @param block The block to search for the adjacent next
\\ @return The next available adjacent block, null if it doesn't exist
```
#### <a id="ref_c95af63a38bc955dad2fb2bcc24d0568"/>fn\<std.__MemBlockHeader*\> std.__insertAvailBlock(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L43) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L133)
```qinp
\\ Inserts a new block of memory into the list (sorted by address)
\\ @param block The block of memory to insert
\\ @return The block before the inserted block (null if the block was inserted at the beginning of the list)
```
#### <a id="ref_cf9261bf2502ba87a6bf1484107500fa"/>fn\<bool\> std.__isAdjacentToNext(std.__MemBlockHeader const* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L66) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L195)
```qinp
\\ Checks whether the given block of memory is adjacent to the next one.
\\ @param block The block of memory to check
\\ @return True if the block is adjacent to the next one, otherwise false
```
#### <a id="ref_0c8c089dc1a1dba19674f2c657c8cb5b"/>fn\<bool\> std.__isAdjacentToNext(std.__MemBlockHeader const* block, std.__MemBlockHeader const* next)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L73) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L202)
```qinp
\\ Checks whether the given blocks of memory are adjacent.
\\ @param block The block of memory to check
\\ @param next The block of memory that may be adjacent to block
\\ @return True if block is adjacent to next, otherwise false
```
#### <a id="ref_e1595932126d7e8bcbe0747eb7b7aff5"/>fn\<std.__MemBlockHeader*\> std.__mergeBlockWithNeighbors(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L78) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L208)
```qinp
\\ Merges the given block with the previous and next blocks if they are adjacent.
\\ @param block The block of memory to merge
\\ @return Pointer to the merged block
```
#### <a id="ref_cd1c3a148ca6c80a4ca20106fcd487c2"/>fn\<std.__MemBlockHeader*\> std.__mergeBlockWithNext(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L61) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L184)
```qinp
\\ Merges the given blocks of memory into a single block.
\\ Both blocks must be adjacent.
\\ @param block The block to merge with its next block
\\ @return The merged block of memory
```
#### <a id="ref_b6d3e3920cc7b0f2330772916ff4fb9d"/>fn\<\> std.__removeAvailBlock(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L47) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L155)
```qinp
\\ Removes a block of memory from the list of free blocks.
\\ @param block The block of memory to remove
```
#### <a id="ref_9ef372736f3a8d4ad83e20374764ab82"/>fn\<void*\> std.__sbrk(i64 increment)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L27) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L97)
```qinp
\\ Changes the data segment size by the given amount and returns a pointer to the newly allocated memory.
\\ @param increment The amount to increase/decrease the data segment size by
\\ @return A pointer pointing to the previous data segment end (null on failure)
```
#### <a id="ref_1dbf5eac888d340741bd2677467ed627"/>fn\<std.__MemBlockHeader*\> std.__splitBlock(std.__MemBlockHeader* block, u64 newSizeAvail)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L55) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L168)
```qinp
\\ Splits the given block of memory into two. The first block gets the given size (and a possible padding for alignment) and the second block gets the remaining size.
\\ If the new block would have an available size of 0 or less, the block is not split and null is returned.
\\ The new block is always aligned to __MEM_ALIGNMENT_SIZE.
\\ @param block The block of memory to shrink
\\ @param newSizeAvail The new available size of the block
\\ @return The new block of memory (null if the block was not split)
```
#### <a id="ref_d9a2acfa92c7e2a645dc38c9a0695836"/>fn\<\> std.free(void* pBuff)
> [Declaration](/stdlib/memory.qnp?plain=1#L44) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L303)
```qinp
\\ Free a buffer
\\ @param pBuff The buffer to free
```
#### <a id="ref_ed884f46d3fc4317f94703c1c65177b4"/>fn\<void*\> std.malloc(u64 size) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L34) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L233)
```qinp
\\ Allocate a buffer of a given size
\\ @param size The size of the buffer to allocate
\\ @return The allocated buffer (null if failed)
```
#### <a id="ref_04a1fa7a62588505a449a3b241dbaab1"/>fn\<void*\> std.realloc(void* pBuff, u64 size) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L40) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L255)
```qinp
\\ Reallocate a buffer to a given size
\\ @param pBuff The pointer to the block of memory to reallocate (Must be previously allocated by malloc)
\\ @param size The new size of the buffer
\\ @return The pointer to the newly allocated memory.
```

