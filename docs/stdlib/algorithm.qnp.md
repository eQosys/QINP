
# Stdlib - stdlib/algorithm.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn<u64> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn<bool>(void const*, void const*) compare)](#ref_5af8465e291191edf9036fc6192a9848)
 - [fn<void*> std.sort(void* array, u64 count, u64 elemSize, fn<bool>(void const*, void const*) compare, fn<void>(void*, void*) swap)](#ref_3c704e6f2b3be08b6873d7a813d533e5)
 - [fn<> std.swap(void* a, void* b, u64 size)](#ref_a8f12f9a256147d65bea1c21eab20585)

## Macros
 - std.compare_lmbd
 - std.swap_lmbd

## Details
#### <a id="ref_5af8465e291191edf9036fc6192a9848"/>fn<u64> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn<bool>(void const*, void const*) compare)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L35) | [Definition](/stdlib/algorithm.qnp?plain=1#L63)
```qinp
\\ Returns the index of any elemnt in an array that matches the given key.
\\ @param key The key to search for.
\\ @param array The array to search. (Must be sorted in the same order as compare would sort the array)
\\ @param count The number of elements in the array.
\\ @param elemSize The size of each element.
\\ @param compare A function that compares two elements. (Ascending: true if a < b)
\\ @return The index of the element that matches the key, or -1 if no element matches the key.
```
#### <a id="ref_3c704e6f2b3be08b6873d7a813d533e5"/>fn<void*> std.sort(void* array, u64 count, u64 elemSize, fn<bool>(void const*, void const*) compare, fn<void>(void*, void*) swap)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L20) | [Definition](/stdlib/algorithm.qnp?plain=1#L41)
```qinp
\\ Sorts an array of elements.
\\ @param array The array to sort.
\\ @param count The number of elements in the array.
\\ @param elemSize The size of each element.
\\ @param compare A function that compares two elements. (Ascending: true if a < b)
\\ @param swap A function that swaps two elements.
\\ @return The sorted array.
```
#### <a id="ref_a8f12f9a256147d65bea1c21eab20585"/>fn<> std.swap(void* a, void* b, u64 size)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L26) | [Definition](/stdlib/algorithm.qnp?plain=1#L55)
```qinp
\\ Swaps the content of two arrays.
\\ @param a The first array.
\\ @param b The second array.
\\ @param size The size of each array.
```

