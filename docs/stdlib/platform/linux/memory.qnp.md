
# Stdlib - stdlib/platform/linux/memory.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)


## Globals
 - [std.__MemBlockHeader* std.__memBlocks]()

## Functions
 - [std.__MemBlockHeader* std.__allocBlock(u64 std.minSizeAvail)]()
 - [u64 std.__brk(u64 std.size)]()
 - [std.__MemBlockHeader* std.__getAvailBlock(u64 std.minSizeAvail)]()
 - [std.__MemBlockHeader* std.__insertAvailBlock(std.__MemBlockHeader* std.block)]()
 - [bool std.__isAdjacentToNext(std.__MemBlockHeader const* std.block)]()
 - [std.__MemBlockHeader* std.__mergeBlockWithNeighbors(std.__MemBlockHeader* std.block)]()
 - [std.__MemBlockHeader* std.__mergeBlockWithNext(std.__MemBlockHeader* std.block)]()
 - [void std.__removeAvailBlock(std.__MemBlockHeader* std.block)]()
 - [u64 std.__roundDown(u64 std.value, u64 std.alignment)]()
 - [u64 std.__roundUp(u64 std.value, u64 std.alignment)]()
 - [void* std.__sbrk(i64 std.increment)]()
 - [std.__MemBlockHeader* std.__splitBlock(std.__MemBlockHeader* std.block, u64 std.newSizeAvail)]()
 - [void std.free(void* std.ptr)]()
 - [void* std.malloc(u64 std.size)]()

## Packs/Unions
 - [pack std.__MemBlockHeader]()

## Macros
 - [std.__FREE_PAGES_COUNT_MAX]()
 - [std.__MEM_ALIGNMENT_SIZE]()
 - [std.__PAGE_SIZE]()

