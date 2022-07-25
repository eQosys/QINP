
# Stdlib - stdlib/platform/linux/memory.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)


## Globals
 - [std.__MemBlockHeader* std.__memBlocks]()

## Functions
 - [std.__MemBlockHeader* std.__allocBlock(u64 minSizeAvail)]()
 - [u64 std.__brk(u64 size)]()
 - [std.__MemBlockHeader* std.__getAvailBlock(u64 minSizeAvail)]()
 - [std.__MemBlockHeader* std.__getNextAdjacentAvailBlock(std.__MemBlockHeader* block)]()
 - [std.__MemBlockHeader* std.__insertAvailBlock(std.__MemBlockHeader* block)]()
 - [bool std.__isAdjacentToNext(std.__MemBlockHeader const* block)]()
 - [bool std.__isAdjacentToNext(std.__MemBlockHeader const* block, std.__MemBlockHeader const* next)]()
 - [std.__MemBlockHeader* std.__mergeBlockWithNeighbors(std.__MemBlockHeader* block)]()
 - [std.__MemBlockHeader* std.__mergeBlockWithNext(std.__MemBlockHeader* block)]()
 - [void std.__removeAvailBlock(std.__MemBlockHeader* block)]()
 - [u64 std.__roundDown(u64 value, u64 alignment)]()
 - [u64 std.__roundUp(u64 value, u64 alignment)]()
 - [void* std.__sbrk(i64 increment)]()
 - [std.__MemBlockHeader* std.__splitBlock(std.__MemBlockHeader* block, u64 newSizeAvail)]()
 - [void std.free(void* ptr)]()
 - [void* std.malloc(u64 size)]()
 - [void* std.realloc(void* ptr, u64 size)]()

## Packs/Unions
 - [pack std.__MemBlockHeader]()

## Macros
 - [std.__FREE_PAGES_COUNT_MAX]()
 - [std.__MEM_ALIGNMENT_SIZE]()
 - [std.__PAGE_SIZE]()

