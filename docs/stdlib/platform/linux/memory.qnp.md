
# Stdlib - stdlib/platform/linux/memory.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)
 - [Details](#details)


## Globals
 - [var<std.__MemBlockHeader*> std.__memBlocks]

## Functions
 - [fn<std.__MemBlockHeader*> std.__allocBlock(u64 minSizeAvail)](#ref_044c7126caa3710257ace62a290def2a)
 - [fn<u64> std.__brk(u64 size)](#ref_8c2471de8675d73e6e5841c8811a9c36)
 - [fn<std.__MemBlockHeader*> std.__getAvailBlock(u64 minSizeAvail)](#ref_6c92042d4c9cdd21c5e0c761609dd786)
 - [fn<std.__MemBlockHeader*> std.__getNextAdjacentAvailBlock(std.__MemBlockHeader* block)](#ref_a3ab94a3346f60f044d6989f0b1c885f)
 - [fn<std.__MemBlockHeader*> std.__insertAvailBlock(std.__MemBlockHeader* block)](#ref_247c31d9d4dd64c4d0459e042cfa6b0e)
 - [fn<bool> std.__isAdjacentToNext(std.__MemBlockHeader const* block)](#ref_55337e11f29c135c921eef789be9e948)
 - [fn<bool> std.__isAdjacentToNext(std.__MemBlockHeader const* block, std.__MemBlockHeader const* next)](#ref_19495c9eb8552a985835bf8ba894ad31)
 - [fn<std.__MemBlockHeader*> std.__mergeBlockWithNeighbors(std.__MemBlockHeader* block)](#ref_9c7fc6ee6c2a1521f7ef80720ddd6744)
 - [fn<std.__MemBlockHeader*> std.__mergeBlockWithNext(std.__MemBlockHeader* block)](#ref_e38a38288bfe4f2074bcb49d90dc7af1)
 - [fn<> std.__removeAvailBlock(std.__MemBlockHeader* block)](#ref_272bee41ae7e75bf864e653e8924c4c6)
 - [fn<void*> std.__sbrk(i64 increment)](#ref_40d4ac3cc43e60f94fed5d1606f5bebd)
 - [fn<std.__MemBlockHeader*> std.__splitBlock(std.__MemBlockHeader* block, u64 newSizeAvail)](#ref_5e92eff4eb8e802669fe18193c8ca6d7)
 - [fn<> std.free(void* pBuff)](#ref_8dcfb6fda3b3e948fa3b63029e159615)
 - [fn<void*> std.malloc(u64 size) nodiscard](#ref_57364cd638ae6a542acdb4218579fe9d)
 - [fn<void*> std.realloc(void* pBuff, u64 size) nodiscard](#ref_73224cc2d187a9780b454e06431870b8)

## Packs/Unions
 - [pack std.__MemBlockHeader]

## Macros
 - [std.__FREE_PAGES_COUNT_MAX]
 - [std.__MEM_ALIGNMENT_SIZE]
 - [std.__PAGE_SIZE]

## Details
#### <a id="ref_044c7126caa3710257ace62a290def2a"/>fn<std.__MemBlockHeader*> std.__allocBlock(u64 minSizeAvail)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L81) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L196)
```qinp
\\ Allocates a block of memory padded to a page size while being big enough to hold the given available size and the block header.
\\ @param minSizeAvail The size of the block to allocate
\\ @return A pointer to the header of the allocated block (null on failure)
```
#### <a id="ref_8c2471de8675d73e6e5841c8811a9c36"/>fn<u64> std.__brk(u64 size)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L20) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L92)
```qinp
\\ Changes the data segment size to the given size and returns the new size.
\\ @param size The new size
\\ @return The size after the change (Current size on failure)
```
#### <a id="ref_6c92042d4c9cdd21c5e0c761609dd786"/>fn<std.__MemBlockHeader*> std.__getAvailBlock(u64 minSizeAvail)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L31) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L100)
```qinp
\\ Returns the smallest block of memory that is large enough to hold the given size.
\\ If no block is large enough, returns the last block in the list.
\\ @param minSizeAvail The available size of the block to find
\\ @return The smallest block of memory that is large enough to hold the given size or the last block in the list
```
#### <a id="ref_a3ab94a3346f60f044d6989f0b1c885f"/>fn<std.__MemBlockHeader*> std.__getNextAdjacentAvailBlock(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L36) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L114)
```qinp
\\ Returns the next adjacent block if there is one available
\\ @param block The block to search for the adjacent next
\\ @return The next available adjacent block, null if it doesn't exist
```
#### <a id="ref_247c31d9d4dd64c4d0459e042cfa6b0e"/>fn<std.__MemBlockHeader*> std.__insertAvailBlock(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L41) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L126)
```qinp
\\ Inserts a new block of memory into the list (sorted by address)
\\ @param block The block of memory to insert
\\ @return The block before the inserted block (null if the block was inserted at the beginning of the list)
```
#### <a id="ref_55337e11f29c135c921eef789be9e948"/>fn<bool> std.__isAdjacentToNext(std.__MemBlockHeader const* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L64) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L178)
```qinp
\\ Checks whether the given block of memory is adjacent to the next one.
\\ @param block The block of memory to check
\\ @return True if the block is adjacent to the next one, otherwise false
```
#### <a id="ref_19495c9eb8552a985835bf8ba894ad31"/>fn<bool> std.__isAdjacentToNext(std.__MemBlockHeader const* block, std.__MemBlockHeader const* next)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L71) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L182)
```qinp
\\ Checks whether the given blocks of memory are adjacent.
\\ @param block The block of memory to check
\\ @param next The block of memory that may be adjacent to block
\\ @return True if block is adjacent to next, otherwise false
```
#### <a id="ref_9c7fc6ee6c2a1521f7ef80720ddd6744"/>fn<std.__MemBlockHeader*> std.__mergeBlockWithNeighbors(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L76) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L186)
```qinp
\\ Merges the given block with the previous and next blocks if they are adjacent.
\\ @param block The block of memory to merge
\\ @return Pointer to the merged block
```
#### <a id="ref_e38a38288bfe4f2074bcb49d90dc7af1"/>fn<std.__MemBlockHeader*> std.__mergeBlockWithNext(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L59) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L169)
```qinp
\\ Merges the given blocks of memory into a single block.
\\ Both blocks must be adjacent.
\\ @param block The block to merge with its next block
\\ @return The merged block of memory
```
#### <a id="ref_272bee41ae7e75bf864e653e8924c4c6"/>fn<> std.__removeAvailBlock(std.__MemBlockHeader* block)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L45) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L146)
```qinp
\\ Removes a block of memory from the list of free blocks.
\\ @param block The block of memory to remove
```
#### <a id="ref_40d4ac3cc43e60f94fed5d1606f5bebd"/>fn<void*> std.__sbrk(i64 increment)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L25) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L95)
```qinp
\\ Changes the data segment size by the given amount and returns a pointer to the newly allocated memory.
\\ @param increment The amount to increase/decrease the data segment size by
\\ @return A pointer pointing to the previous data segment end (null on failure)
```
#### <a id="ref_5e92eff4eb8e802669fe18193c8ca6d7"/>fn<std.__MemBlockHeader*> std.__splitBlock(std.__MemBlockHeader* block, u64 newSizeAvail)
> [Declaration](/stdlib/platform/linux/memory.qnp?plain=1#L53) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L156)
```qinp
\\ Splits the given block of memory into two. The first block gets the given size (and a possible padding for alignment) and the second block gets the remaining size.
\\ If the new block would have an available size of 0 or less, the block is not split and null is returned.
\\ The new block is always aligned to __MEM_ALIGNMENT_SIZE.
\\ @param block The block of memory to shrink
\\ @param newSizeAvail The new available size of the block
\\ @return The new block of memory (null if the block was not split)
```
#### <a id="ref_8dcfb6fda3b3e948fa3b63029e159615"/>fn<> std.free(void* pBuff)
> [Declaration](/stdlib/memory.qnp?plain=1#L42) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L275)
```qinp
\\ Free a buffer
\\ @param pBuff The buffer to free
```
#### <a id="ref_57364cd638ae6a542acdb4218579fe9d"/>fn<void*> std.malloc(u64 size) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L32) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L207)
```qinp
\\ Allocate a buffer of a given size
\\ @param size The size of the buffer to allocate
\\ @return The allocated buffer (null if failed)
```
#### <a id="ref_73224cc2d187a9780b454e06431870b8"/>fn<void*> std.realloc(void* pBuff, u64 size) nodiscard
> [Declaration](/stdlib/memory.qnp?plain=1#L38) | [Definition](/stdlib/platform/linux/memory.qnp?plain=1#L227)
```qinp
\\ Reallocate a buffer to a given size
\\ @param pBuff The pointer to the block of memory to reallocate (Must be previously allocated by malloc)
\\ @param size The new size of the buffer
\\ @return The pointer to the newly allocated memory.
```

