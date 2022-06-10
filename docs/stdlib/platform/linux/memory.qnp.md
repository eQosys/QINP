
# Stdlib - stdlib/platform/linux/memory.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)


## Globals
 - [std.__MemBlockHeader* __memBlocks]()

## Functions
 - [std.__MemBlockHeader* __allocBlock(u64 sizeAvail)]()
 - [u64 __brk(u64 size)]()
 - [std.__MemBlockHeader* __getAvailBlock(u64 sizeAvail)]()
 - [std.__MemBlockHeader* __insertAvailBlock(std.__MemBlockHeader* block)]()
 - [bool __isAdjacentToNext(std.__MemBlockHeader const* block)]()
 - [std.__MemBlockHeader* __mergeBlockWithNeighbors(std.__MemBlockHeader* block)]()
 - [std.__MemBlockHeader* __mergeBlockWithNext(std.__MemBlockHeader* block)]()
 - [void __removeAvailBlock(std.__MemBlockHeader* block)]()
 - [u64 __roundDown(u64 value, u64 alignment)]()
 - [u64 __roundUp(u64 value, u64 alignment)]()
 - [void* __sbrk(i64 increment)]()
 - [std.__MemBlockHeader* __splitBlock(std.__MemBlockHeader* block, u64 newSizeAvail)]()
 - [void free(void* ptr)]()
 - [void* malloc(u64 size)]()

## Packs/Unions
 - [pack __MemBlockHeader]()

## Macros
 - [__FREE_PAGES_COUNT_MAX]()
 - [__MEM_ALIGNMENT_SIZE]()
 - [__PAGE_SIZE]()

