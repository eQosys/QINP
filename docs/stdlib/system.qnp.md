
# Stdlib - stdlib/system.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<\> std.exit(i32 status) ...](#ref_9bf931dbfbf6971477769341ef0c87f9)
 - [fn\<u64\> std.getargs(u8 const* const** pargv) nodiscard](#ref_8fe7cd1cd74ae31344c7038fc26c5b89)
 - [fn\<u8 const* const*\> std.getenv() nodiscard](#ref_ebb5fd0cc4d3b2ac13f1f706034f9a3d)

## Macros
 - [std.EXIT_FAILURE](#ref_43e9d3884164119de47fbc9edc85146c)
 - [std.EXIT_SUCCESS](#ref_e1fff1fb026986947dccf12f4f923778)

## Details
#### <a id="ref_43e9d3884164119de47fbc9edc85146c"/>std.EXIT_FAILURE
> [Declaration](/stdlib/system.qnp?plain=1#L10)
```qinp
Value to be returned by the program to indicate failure.
```
#### <a id="ref_e1fff1fb026986947dccf12f4f923778"/>std.EXIT_SUCCESS
> [Declaration](/stdlib/system.qnp?plain=1#L7)
```qinp
Value to be returned by the program to indicate success.
```
#### <a id="ref_9bf931dbfbf6971477769341ef0c87f9"/>fn\<\> std.exit(i32 status) ...
> [Declaration](/stdlib/system.qnp?plain=1#L14) | [Definition](/stdlib/platform/linux/system.qnp?plain=1#L13)
```qinp
Terminate the calling process and return the specified exit status.
@param status The exit status to return to the parent process.
```
#### <a id="ref_8fe7cd1cd74ae31344c7038fc26c5b89"/>fn\<u64\> std.getargs(u8 const* const** pargv) nodiscard
> [Declaration](/stdlib/system.qnp?plain=1#L19) | [Definition](/stdlib/system.qnp?plain=1#L29)
```qinp
Get the arguments passed to the program
@param pargv Address of the pointer where the pointer of the first argument will be stored
@return The number of arguments passed to the program
```
#### <a id="ref_ebb5fd0cc4d3b2ac13f1f706034f9a3d"/>fn\<u8 const* const*\> std.getenv() nodiscard
> [Declaration](/stdlib/system.qnp?plain=1#L23) | [Definition](/stdlib/system.qnp?plain=1#L46)
```qinp
Get the environment variables passed to the program
@return The pointer to an array of environment variables
```

