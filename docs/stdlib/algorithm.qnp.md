
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
 - [std.FN_COMPARE](#ref_3108e91a5c48769163cc6624f068efd4)
 - [std.FN_RANDOM](#ref_0bb2a59d76a8ee4551a6e71931197b31)
 - [std.FN_SWAP](#ref_c21fea908082a6590afa66534aac291d)
 - [std.LMBD_COMPARE(check)](#ref_ddc596d61f92363c82dbffbe1da9f53d)
 - [std.LMBD_COMPARE_DT(type)](#ref_5b91eafb258b9be7e596d2d9179b6b43)
 - [std.LMBD_SWAP(size)](#ref_90e85b2068dfe6c3582b342c4b67c5b4)
 - [std.LMBD_SWAP_DT(type)](#ref_37571d12e94a9eb9c066cc52c7b39c74)

## Details
#### <a id="ref_3108e91a5c48769163cc6624f068efd4"/>std.FN_COMPARE
> [Declaration](/stdlib/algorithm.qnp?plain=1#L8)
```qinp
Function signature for a comparison function.
```
#### <a id="ref_0bb2a59d76a8ee4551a6e71931197b31"/>std.FN_RANDOM
> [Declaration](/stdlib/algorithm.qnp?plain=1#L14)
```qinp
Function signature for a random number generator.
```
#### <a id="ref_c21fea908082a6590afa66534aac291d"/>std.FN_SWAP
> [Declaration](/stdlib/algorithm.qnp?plain=1#L11)
```qinp
Function signature for a swap function.
```
#### <a id="ref_ddc596d61f92363c82dbffbe1da9f53d"/>std.LMBD_COMPARE(check)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L26)
```qinp
Macro to define a lambda function that compares two elements according to the given check.
@param check The check to use. (e.g. *(i64*)a < *(i64*)b)
```
#### <a id="ref_5b91eafb258b9be7e596d2d9179b6b43"/>std.LMBD_COMPARE_DT(type)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L30)
```qinp
Macro to define a lambda function that compares two elements of the given type using the `<` operator.
@param type The type of the elements.
```
#### <a id="ref_90e85b2068dfe6c3582b342c4b67c5b4"/>std.LMBD_SWAP(size)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L18)
```qinp
Macro to define a lambda function that swaps two elements by copying them per byte.
@param size The size of the elements.
```
#### <a id="ref_37571d12e94a9eb9c066cc52c7b39c74"/>std.LMBD_SWAP_DT(type)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L22)
```qinp
Macro to define a lambda function that swaps two elements of the given type by copying them per byte.
@param type The type of the elements.
```
#### <a id="ref_7620a3b85a107268b00cfae846da61b1"/>fn\<u64\> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L62) | [Definition](/stdlib/algorithm.qnp?plain=1#L106)
```qinp
Returns the index of any elemnt in an array that matches the given key.
@param key The key to search for.
@param array The array to search. (Must be sorted in the same order as compare would sort the array)
@param count The number of elements in the array.
@param elemSize The size of each element.
@param compare A function that compares two elements. (Ascending: true if a < b)
@return The index of the element that matches the key, or -1 if no element matches the key.
```
#### <a id="ref_83bccb6ff12a3cf3e7e3005acf7489b0"/>fn\<\> std.shuffle(void* array, u64 count, u64 elemSize, fn\<void\>(void*, void*) swap, fn\<u64\>() random)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L47) | [Definition](/stdlib/algorithm.qnp?plain=1#L85)
```qinp
Shuffles an array of elements.
@param array The array to shuffle.
@param count The number of elements in the array.
@param elemSize The size of each element.
@param swap A function that swaps two elements.
@param random A function that returns a random number.
```
#### <a id="ref_5c35fd71ac9180a270dc73eddf63e700"/>fn\<void*\> std.sort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare, fn\<void\>(void*, void*) swap)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L39) | [Definition](/stdlib/algorithm.qnp?plain=1#L68)
```qinp
Sorts an array of elements.
@param array The array to sort.
@param count The number of elements in the array.
@param elemSize The size of each element.
@param compare A function that compares two elements. (Ascending: true if a < b)
@param swap A function that swaps two elements.
@return The sorted array.
```
#### <a id="ref_89283c97d6eb9724f1a59f3d6ff0da26"/>fn\<\> std.swap(void* a, void* b, u64 size)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L53) | [Definition](/stdlib/algorithm.qnp?plain=1#L94)
```qinp
Swaps the content of two arrays.
@param a The first array.
@param b The second array.
@param size The size of each array.
```

