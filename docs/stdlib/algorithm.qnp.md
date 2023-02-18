
# Stdlib - stdlib/algorithm.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [fn<u64> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn<bool>(void const*, void const*) compare)](#ref_5af8465e291191edf9036fc6192a9848)
 - [fn<void*> std.sort(void* array, u64 count, u64 elemSize, fn<bool>(void const*, void const*) compare)](#ref_7645c7776629edd0f68703f75383657d)
 - [fn<> std.swap(void* a, void* b, u64 size)](#ref_a8f12f9a256147d65bea1c21eab20585)

## Details
#### <a id="ref_5af8465e291191edf9036fc6192a9848"/>fn<u64> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn<bool>(void const*, void const*) compare)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L26) | [Definition](/stdlib/algorithm.qnp?plain=1#L54)
```qinp
\\ Returns the index of any elemnt in an array that matches the given key.
\\ @param key The key to search for.
\\ @param array The array to search. (Must be sorted in the same order as compare would sort the array)
\\ @param count The number of elements in the array.
\\ @param elemSize The size of each element.
\\ @param compare A function that compares two elements. (Ascending: true if a < b)
\\ @return The index of the element that matches the key, or -1 if no element matches the key.
```
#### <a id="ref_7645c7776629edd0f68703f75383657d"/>fn<void*> std.sort(void* array, u64 count, u64 elemSize, fn<bool>(void const*, void const*) compare)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L11) | [Definition](/stdlib/algorithm.qnp?plain=1#L32)
```qinp
\\ Sorts an array of elements.
\\ @param array The array to sort.
\\ @param count The number of elements in the array.
\\ @param elemSize The size of each element.
\\ @param compare A function that compares two elements. (Ascending: true if a < b)
\\ @return The sorted array.
```
#### <a id="ref_a8f12f9a256147d65bea1c21eab20585"/>fn<> std.swap(void* a, void* b, u64 size)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L17) | [Definition](/stdlib/algorithm.qnp?plain=1#L46)
```qinp
\\ Swaps the content of two arrays.
\\ @param a The first array.
\\ @param b The second array.
\\ @param size The size of each array.
```

