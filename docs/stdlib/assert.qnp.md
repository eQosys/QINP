
# Stdlib - stdlib/assert.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<\> std.__assert(bool check, u8 const* msg, u8 const* file, i32 line, u8 const* spaceName)](#ref_0ac05191e5585d1621048905b459411f)

## Macros
 - [std.assert(check, msg)](#ref_cfaf82991af2561a23d60fc80ef4374e)

## Details
#### <a id="ref_0ac05191e5585d1621048905b459411f"/>fn\<\> std.__assert(bool check, u8 const* msg, u8 const* file, i32 line, u8 const* spaceName)
> [Declaration](/stdlib/assert.qnp?plain=1#L19) | [Definition](/stdlib/assert.qnp?plain=1#L25)
```qinp
Internal assert function. Prints error message and exits if check is false.
@param check Boolean expression to check.
@param msg Error message to print.
@param file File name where assert was called.
@param line Line number where assert was called.
@param spaceName Name of the space where assert was called.
```
#### <a id="ref_cfaf82991af2561a23d60fc80ef4374e"/>std.assert(check, msg)
> [Declaration](/stdlib/assert.qnp?plain=1#L11)
```qinp
Assert macro. Replaced with call to __assert and expanded with file, line and space name.
See __assert for more information.
```

