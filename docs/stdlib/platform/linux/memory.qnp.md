
# Stdlib - stdlib/platform/linux/memory.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)


## Globals
 - [var<std.__MemBlockHeader*> std.__memBlocks]()

## Functions
 - [fn<std.__MemBlockHeader*> std.__allocBlock(u64 minSizeAvail)]()
 - [fn<u64> std.__brk(u64 size)]()
 - [fn<std.__MemBlockHeader*> std.__getAvailBlock(u64 minSizeAvail)]()
 - [fn<std.__MemBlockHeader*> std.__getNextAdjacentAvailBlock(std.__MemBlockHeader* block)]()
 - [fn<std.__MemBlockHeader*> std.__insertAvailBlock(std.__MemBlockHeader* block)]()
 - [fn<bool> std.__isAdjacentToNext(std.__MemBlockHeader const* block)]()
 - [fn<bool> std.__isAdjacentToNext(std.__MemBlockHeader const* block, std.__MemBlockHeader const* next)]()
 - [fn<std.__MemBlockHeader*> std.__mergeBlockWithNeighbors(std.__MemBlockHeader* block)]()
 - [fn<std.__MemBlockHeader*> std.__mergeBlockWithNext(std.__MemBlockHeader* block)]()
 - [fn<> std.__removeAvailBlock(std.__MemBlockHeader* block)]()
 - [fn<u64> std.__roundDown(u64 value, u64 alignment)]()
 - [fn<u64> std.__roundUp(u64 value, u64 alignment)]()
 - [fn<void*> std.__sbrk(i64 increment)]()
 - [fn<std.__MemBlockHeader*> std.__splitBlock(std.__MemBlockHeader* block, u64 newSizeAvail)]()
 - [fn<> std.free(void* ptr)]()
 - [fn<void*> std.malloc(u64 size)]()
 - [fn<void*> std.realloc(void* ptr, u64 size)]()

## Packs/Unions
 - [pack std.__MemBlockHeader]()

## Macros
 - [std.__FREE_PAGES_COUNT_MAX]()
 - [std.__MEM_ALIGNMENT_SIZE]()
 - [std.__PAGE_SIZE]()

