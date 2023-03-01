
# Stdlib - stdlib/algorithm.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<u64\> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare)](#ref_7620a3b85a107268b00cfae846da61b1)
 - [fn\<\> std.shuffle(void* array, u64 count, u64 elemSize, fn\<void\>(void*, void*) swap, fn\<u64\>() random)](#ref_83bccb6ff12a3cf3e7e3005acf7489b0)
 - [fn\<void*\> std.sort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare, fn\<void\>(void*, void*) swap)](#ref_5c35fd71ac9180a270dc73eddf63e700)
 - [fn\<\> std.swap(void* a, void* b, u64 size)](#ref_89283c97d6eb9724f1a59f3d6ff0da26)

## Macros
 - std.FN_COMPARE
 - std.FN_RANDOM
 - std.FN_SWAP
 - std.LMBD_COMPARE
 - std.LMBD_SWAP

## Details
#### <a id="ref_7620a3b85a107268b00cfae846da61b1"/>fn\<u64\> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L52) | [Definition](/stdlib/algorithm.qnp?plain=1#L84)
```qinp
\\ Returns the index of any elemnt in an array that matches the given key.
\\ @param key The key to search for.
\\ @param array The array to search. (Must be sorted in the same order as compare would sort the array)
\\ @param count The number of elements in the array.
\\ @param elemSize The size of each element.
\\ @param compare A function that compares two elements. (Ascending: true if a < b)
\\ @return The index of the element that matches the key, or -1 if no element matches the key.
```
#### <a id="ref_83bccb6ff12a3cf3e7e3005acf7489b0"/>fn\<\> std.shuffle(void* array, u64 count, u64 elemSize, fn\<void\>(void*, void*) swap, fn\<u64\>() random)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L37) | [Definition](/stdlib/algorithm.qnp?plain=1#L71)
```qinp
\\ Shuffles an array of elements.
\\ @param array The array to shuffle.
\\ @param count The number of elements in the array.
\\ @param elemSize The size of each element.
\\ @param swap A function that swaps two elements.
\\ @param random A function that returns a random number.
```
#### <a id="ref_5c35fd71ac9180a270dc73eddf63e700"/>fn\<void*\> std.sort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare, fn\<void\>(void*, void*) swap)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L29) | [Definition](/stdlib/algorithm.qnp?plain=1#L58)
```qinp
\\ Sorts an array of elements.
\\ @param array The array to sort.
\\ @param count The number of elements in the array.
\\ @param elemSize The size of each element.
\\ @param compare A function that compares two elements. (Ascending: true if a < b)
\\ @param swap A function that swaps two elements.
\\ @return The sorted array.
```
#### <a id="ref_89283c97d6eb9724f1a59f3d6ff0da26"/>fn\<\> std.swap(void* a, void* b, u64 size)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L43) | [Definition](/stdlib/algorithm.qnp?plain=1#L76)
```qinp
\\ Swaps the content of two arrays.
\\ @param a The first array.
\\ @param b The second array.
\\ @param size The size of each array.
```

