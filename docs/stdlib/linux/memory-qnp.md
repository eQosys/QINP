# Stdlib - linux/memory.qnp

## Overview
 - [Functions](#functions)
 - [Packs](#packs)
 - [Globals](#globals)
 - [Definitions](#definitions)

## Functions
 - [__MemBlockHeader* __allocBlock ( u64 sizeAvail )]()
 - [u64 __brk ( u64 addr)]()
 - [__MemBlockHeader* __getAvailBlock ( u64 sizeAvail )]()
 - [void* __sbrk ( i64 increment )]()
 - [__MemBlockHeader* __insertAvailBlock ( __MemBlockHeader* block )]()
 - [bool __isAdjacentToNext( __MemBlockHeader* block )]()
 - [__MemBlockHeader* __mergeBlockWithNeighbors( __MemBlockHeader* block )]()
 - [__MemBlockHeader* __mergeBlocks( __MemBlockHeader* block1, __MemBlockHeader* block2 )]()
 - [void __removeAvailBlock ( __MemBlockHeader* block )]()
 - [void* __roundToPageSize ( u64 size )]()
 - [__MemBlockHeader* __splitBlock ( __MemBlockHeader* block, u64 newSizeAvail)]()
 - [void free ( void* ptr )]()
 - [void* malloc ( u64 size )]()

## Packs
 - [__MemBlockHeader]()

## Globals
 - [__MemBlockHeader* __memBlocks]()

## Definitions
 - [FREE_PAGES_COUNT_MAX]()
 - [PAGE_SIZE]()