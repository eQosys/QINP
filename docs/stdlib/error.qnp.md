
# Stdlib - stdlib/error.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Enums](#enums)
 - [Details](#details)


## Globals
 - [var\<std.error.ID\> std.error.__lastErrorID](#ref_38b19f655517fdcc2e574db8ef85aa9c)

## Functions
 - [fn\<u8 const**\> std.error.__genErrorStrings()](#ref_1da32c02d2e1b84dd73106f6242bfb20)
 - [fn\<\> std.error.clear()](#ref_4a5ec4c965dca5c596e82065f9b44eae)
 - [fn\<std.error.ID\> std.error.get()](#ref_b02942fd928b05dd1e9234338c5fbee2)
 - [fn\<\> std.error.set(std.error.ID err)](#ref_1e4af950465692606de70fa31a5c319b)
 - [fn\<u8 const*\> std.error.toString(std.error.ID err)](#ref_576f588031db164d77fe8b235ad0f8bb)

## Enums
 - [enum std.error.ID](#ref_c464b09d77929bb17dd296443d27d4c9)

## Details
#### <a id="ref_c464b09d77929bb17dd296443d27d4c9"/>enum std.error.ID
> [Declaration](/stdlib/error.qnp?plain=1#L8)
```qinp
Error IDs
```
#### <a id="ref_1da32c02d2e1b84dd73106f6242bfb20"/>fn\<u8 const**\> std.error.__genErrorStrings()
> [Declaration](/stdlib/error.qnp?plain=1#L37) | [Definition](/stdlib/error.qnp?plain=1#L63)
```qinp
Generates the error strings for the error IDs.
@return Array of error strings.
```
#### <a id="ref_38b19f655517fdcc2e574db8ef85aa9c"/>var\<std.error.ID\> std.error.__lastErrorID
> [Declaration](/stdlib/error.qnp?plain=1#L17)
```qinp
Internal variable to hold the set error.
```
#### <a id="ref_4a5ec4c965dca5c596e82065f9b44eae"/>fn\<\> std.error.clear()
> [Declaration](/stdlib/error.qnp?plain=1#L28) | [Definition](/stdlib/error.qnp?plain=1#L48)
```qinp
Clears the last error set.
```
#### <a id="ref_b02942fd928b05dd1e9234338c5fbee2"/>fn\<std.error.ID\> std.error.get()
> [Declaration](/stdlib/error.qnp?plain=1#L25) | [Definition](/stdlib/error.qnp?plain=1#L45)
```qinp
Retrieves the last error set.
@return Last error id that has been set.
```
#### <a id="ref_1e4af950465692606de70fa31a5c319b"/>fn\<\> std.error.set(std.error.ID err)
> [Declaration](/stdlib/error.qnp?plain=1#L21) | [Definition](/stdlib/error.qnp?plain=1#L42)
```qinp
Sets the global error value. Can be retrieved via a call to 'getError'
@param err id of the error to be set.
```
#### <a id="ref_576f588031db164d77fe8b235ad0f8bb"/>fn\<u8 const*\> std.error.toString(std.error.ID err)
> [Declaration](/stdlib/error.qnp?plain=1#L33) | [Definition](/stdlib/error.qnp?plain=1#L51)
```qinp
Returns a string containing a description for the provided error.
@param err ID of the error to return a description for. (null if no description is present)
@return Description of the error. null on error.
```

