
# Stdlib - stdlib/algorithm.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<\> std.__merge(void* array, void* left, void* right, u64 leftCount, u64 rightCount, u64 elemSize, fn\<bool\>(void const*, void const*) compare)](#ref_ebbea47ab0159895afefb71fdb4ff376)
 - [fn\<void*\> std.__mergeSort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare, void* temp)](#ref_1f22f6cdd86dbcb9114a5c400deaab51)
 - [fn\<u64\> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare)](#ref_7620a3b85a107268b00cfae846da61b1)
 - [fn\<u64\> std.countVarArgs(T first)](#ref_a4780167fcc663d4b761f1085e061ac8)
 - [fn\<u64\> std.countVarArgs(T first, ...)](#ref_e891ca08f81c297ec7505f67ffe5fd3d)
 - [fn\<void*\> std.mergeSort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare)](#ref_f8335c7fc67db19698fca73fe7d4164c)
 - [fn\<\> std.shuffle(void* array, u64 count, u64 elemSize, fn\<void\>(void*, void*) swap, fn\<u64\>() random)](#ref_83bccb6ff12a3cf3e7e3005acf7489b0)
 - [fn\<void*\> std.sort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare, fn\<void\>(void*, void*) swap)](#ref_5c35fd71ac9180a270dc73eddf63e700)
 - [fn\<\> std.swap(void* a, void* b, u64 size)](#ref_89283c97d6eb9724f1a59f3d6ff0da26)

## Macros
 - [std.FN_COMPARE](#ref_3108e91a5c48769163cc6624f068efd4)
 - [std.FN_DELETE](#ref_ff917cbaff2d3e1adcb695ec32d9985c)
 - [std.FN_RANDOM](#ref_0bb2a59d76a8ee4551a6e71931197b31)
 - [std.FN_SWAP](#ref_c21fea908082a6590afa66534aac291d)
 - [std.LMBD_COMPARE(check)](#ref_ddc596d61f92363c82dbffbe1da9f53d)
 - [std.LMBD_COMPARE_DT(type)](#ref_5b91eafb258b9be7e596d2d9179b6b43)
 - [std.LMBD_DELETE(type)](#ref_d104f5f260cc5abf4b7c3cfdcc576d81)
 - [std.LMBD_SWAP(size)](#ref_90e85b2068dfe6c3582b342c4b67c5b4)
 - [std.LMBD_SWAP_DT(type)](#ref_37571d12e94a9eb9c066cc52c7b39c74)
 - [std.swap_obj(a, b)](#ref_829ac646b96c2fe9aeed71e858c87d5f)

## Details
#### <a id="ref_3108e91a5c48769163cc6624f068efd4"/>std.FN_COMPARE
> [Declaration](/stdlib/algorithm.qnp?plain=1#L11)
```qinp
Function signature for a comparison function.
```
#### <a id="ref_ff917cbaff2d3e1adcb695ec32d9985c"/>std.FN_DELETE
> [Declaration](/stdlib/algorithm.qnp?plain=1#L20)
```qinp
Function signature for deleting an object.
```
#### <a id="ref_0bb2a59d76a8ee4551a6e71931197b31"/>std.FN_RANDOM
> [Declaration](/stdlib/algorithm.qnp?plain=1#L17)
```qinp
Function signature for a random number generator.
```
#### <a id="ref_c21fea908082a6590afa66534aac291d"/>std.FN_SWAP
> [Declaration](/stdlib/algorithm.qnp?plain=1#L14)
```qinp
Function signature for a swap function.
```
#### <a id="ref_ddc596d61f92363c82dbffbe1da9f53d"/>std.LMBD_COMPARE(check)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L24)
```qinp
Macro to define a lambda function that compares two elements according to the given check.
@param check The check to use. (e.g. *(i64*)a < *(i64*)b)
```
#### <a id="ref_5b91eafb258b9be7e596d2d9179b6b43"/>std.LMBD_COMPARE_DT(type)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L28)
```qinp
Macro to define a lambda function that compares two elements of the given type using the `<` operator.
@param type The type of the elements.
```
#### <a id="ref_d104f5f260cc5abf4b7c3cfdcc576d81"/>std.LMBD_DELETE(type)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L40)
```qinp
Macro to define a lambda function that deletes an object.
@param type The type of the object.
```
#### <a id="ref_90e85b2068dfe6c3582b342c4b67c5b4"/>std.LMBD_SWAP(size)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L32)
```qinp
Macro to define a lambda function that swaps two elements by copying them per byte.
@param size The size of the elements.
```
#### <a id="ref_37571d12e94a9eb9c066cc52c7b39c74"/>std.LMBD_SWAP_DT(type)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L36)
```qinp
Macro to define a lambda function that swaps two elements of the given type by copying them per byte.
@param type The type of the elements.
```
#### <a id="ref_ebbea47ab0159895afefb71fdb4ff376"/>fn\<\> std.__merge(void* array, void* left, void* right, u64 leftCount, u64 rightCount, u64 elemSize, fn\<bool\>(void const*, void const*) compare)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L77) | [Definition](/stdlib/algorithm.qnp?plain=1#L166)
```qinp
Merges two sorted arrays into a single sorted array. (Copies elements per byte)
@param array The array to merge into.
@param left The left array.
@param right The right array.
@param leftCount The number of elements in the left array.
@param rightCount The number of elements in the right array.
@param elemSize The size of each element.
@param compare A function that compares two elements. (Ascending: true if a < b)
```
#### <a id="ref_1f22f6cdd86dbcb9114a5c400deaab51"/>fn\<void*\> std.__mergeSort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare, void* temp)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L67) | [Definition](/stdlib/algorithm.qnp?plain=1#L145)
```qinp
Internal function to sort an array of elements using the merge sort algorithm. (Copies elements per byte)
@param array The array to sort.
@param count The number of elements in the array.
@param elemSize The size of each element.
@param compare A function that compares two elements. (Ascending: true if a < b)
@param temp A temporary buffer to use for sorting. (Must be at least `count * elemSize` bytes large)
@return The sorted array.
```
#### <a id="ref_7620a3b85a107268b00cfae846da61b1"/>fn\<u64\> std.bsearch(void const* key, void const* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L105) | [Definition](/stdlib/algorithm.qnp?plain=1#L215)
```qinp
Returns the index of any elemnt in an array that matches the given key.
@param key The key to search for.
@param array The array to search. (Must be sorted in the same order as compare would sort the array)
@param count The number of elements in the array.
@param elemSize The size of each element.
@param compare A function that compares two elements. (Ascending: true if a < b)
@return The index of the element that matches the key, or -1 if no element matches the key.
```
#### <a id="ref_a4780167fcc663d4b761f1085e061ac8"/>fn\<u64\> std.countVarArgs(T first)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L110) | [Definition](/stdlib/algorithm.qnp?plain=1#L235)
```qinp
Counts the number of elements in a variadic argument list.
@param first The first argument.
@return The number of arguments. (Always 1)
```
#### <a id="ref_e891ca08f81c297ec7505f67ffe5fd3d"/>fn\<u64\> std.countVarArgs(T first, ...)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L116) | [Definition](/stdlib/algorithm.qnp?plain=1#L238)
```qinp
Counts the number of elements in a variadic argument list.
@param first The first argument.
@param ... The remaining arguments.
@return The number of arguments.
```
#### <a id="ref_f8335c7fc67db19698fca73fe7d4164c"/>fn\<void*\> std.mergeSort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L58) | [Definition](/stdlib/algorithm.qnp?plain=1#L139)
```qinp
Sorts an array of elements using the merge sort algorithm. (Copies elements per byte)
@param array The array to sort.
@param count The number of elements in the array.
@param elemSize The size of each element.
@param compare A function that compares two elements. (Ascending: true if a < b)
@param swap A function that swaps two elements.
@return The sorted array.
```
#### <a id="ref_83bccb6ff12a3cf3e7e3005acf7489b0"/>fn\<\> std.shuffle(void* array, u64 count, u64 elemSize, fn\<void\>(void*, void*) swap, fn\<u64\>() random)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L85) | [Definition](/stdlib/algorithm.qnp?plain=1#L194)
```qinp
Shuffles an array of elements.
@param array The array to shuffle.
@param count The number of elements in the array.
@param elemSize The size of each element.
@param swap A function that swaps two elements.
@param random A function that returns a random number.
```
#### <a id="ref_5c35fd71ac9180a270dc73eddf63e700"/>fn\<void*\> std.sort(void* array, u64 count, u64 elemSize, fn\<bool\>(void const*, void const*) compare, fn\<void\>(void*, void*) swap)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L49) | [Definition](/stdlib/algorithm.qnp?plain=1#L122)
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
> [Declaration](/stdlib/algorithm.qnp?plain=1#L96) | [Definition](/stdlib/algorithm.qnp?plain=1#L203)
```qinp
Swaps the content of two arrays.
@param a The first array.
@param b The second array.
@param size The size of each array.
```
#### <a id="ref_829ac646b96c2fe9aeed71e858c87d5f"/>std.swap_obj(a, b)
> [Declaration](/stdlib/algorithm.qnp?plain=1#L90)
```qinp
Swaps the content of two objects. (The size of both objects must be the same and detectable with `sizeof`)
@param a The first object.
@param b The second object.
```

