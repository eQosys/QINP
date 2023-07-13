
# Stdlib - stdlib/system.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<i32\> std.__system(u8 const* progPath, u8* const* argv, u8* const* envp) ...](#ref_74ea8799de94aed5abd1a7777a388a81)
 - [fn\<bool\> std.__systemFillArgv(u64 index, u8** argv, u8 const* arg, ...)](#ref_2c6595a9c996e6cbc2ff83e8f3f92822)
 - [fn\<bool\> std.__systemFillArgv(u64 index, u8** argv, u8 const* arg)](#ref_8a4cbc8377bc2a9f5a0b282fd3987e35)
 - [fn\<\> std.exit(i32 status) ...](#ref_9bf931dbfbf6971477769341ef0c87f9)
 - [fn\<u64\> std.getargs(u8* const** pargv) nodiscard](#ref_96acbff83b184723a97d7319704fe4c5)
 - [fn\<u8* const*\> std.getenv() nodiscard](#ref_1030391c8b2e15c79c12ecf58d6c24db)
 - [fn\<i32\> std.system(u8 const* command)](#ref_b4fdfd81408601c059b578308e650c5a)

## Macros
 - [std.EXIT_FAILURE](#ref_43e9d3884164119de47fbc9edc85146c)
 - [std.EXIT_SUCCESS](#ref_e1fff1fb026986947dccf12f4f923778)

## Details
#### <a id="ref_43e9d3884164119de47fbc9edc85146c"/>std.EXIT_FAILURE
> [Declaration](/stdlib/system.qnp?plain=1#L14)
```qinp
Value to be returned by the program to indicate failure.
```
#### <a id="ref_e1fff1fb026986947dccf12f4f923778"/>std.EXIT_SUCCESS
> [Declaration](/stdlib/system.qnp?plain=1#L11)
```qinp
Value to be returned by the program to indicate success.
```
#### <a id="ref_74ea8799de94aed5abd1a7777a388a81"/>fn\<i32\> std.__system(u8 const* progPath, u8* const* argv, u8* const* envp) ...
> [Declaration](/stdlib/system.qnp?plain=1#L52) | [Definition](/stdlib/platform/linux/system.qnp?plain=1#L16)
```qinp
Internal function to launch a program
@param progPath The path to the program to launch
@param argv The arguments to pass to the program
@param envp The environment variables to pass to the program
@return The exit status of the launched program
```
#### <a id="ref_2c6595a9c996e6cbc2ff83e8f3f92822"/>fn\<bool\> std.__systemFillArgv(u64 index, u8** argv, u8 const* arg, ...)
> [Declaration](/stdlib/system.qnp?plain=1#L45) | [Definition](/stdlib/system.qnp?plain=1#L113)
```qinp
Internal function to fill the argv array
@param index The index of the first element to put in the argv array
@param argv The argv array to fill
@param arg The first argument to put in the argv array
@param ... The remaining arguments to put in the argv array
```
#### <a id="ref_8a4cbc8377bc2a9f5a0b282fd3987e35"/>fn\<bool\> std.__systemFillArgv(u64 index, u8** argv, u8 const* arg)
> [Declaration](/stdlib/system.qnp?plain=1#L38) | [Definition](/stdlib/system.qnp?plain=1#L103)
```qinp
Internal function to fill the argv array
@param index The index of the first element to put in the argv array
@param argv The argv array to fill
@param arg The first argument to put in the argv array
```
#### <a id="ref_9bf931dbfbf6971477769341ef0c87f9"/>fn\<\> std.exit(i32 status) ...
> [Declaration](/stdlib/system.qnp?plain=1#L18) | [Definition](/stdlib/platform/linux/system.qnp?plain=1#L13)
```qinp
Terminate the calling process and return the specified exit status.
@param status The exit status to return to the parent process.
```
#### <a id="ref_96acbff83b184723a97d7319704fe4c5"/>fn\<u64\> std.getargs(u8* const** pargv) nodiscard
> [Declaration](/stdlib/system.qnp?plain=1#L23) | [Definition](/stdlib/system.qnp?plain=1#L57)
```qinp
Get the arguments passed to the program
@param pargv Address of the pointer where the pointer of the first argument will be stored
@return The number of arguments passed to the program
```
#### <a id="ref_1030391c8b2e15c79c12ecf58d6c24db"/>fn\<u8* const*\> std.getenv() nodiscard
> [Declaration](/stdlib/system.qnp?plain=1#L27) | [Definition](/stdlib/system.qnp?plain=1#L74)
```qinp
Get the environment variables passed to the program
@return The pointer to an array of environment variables
```
#### <a id="ref_b4fdfd81408601c059b578308e650c5a"/>fn\<i32\> std.system(u8 const* command)
> [Declaration](/stdlib/system.qnp?plain=1#L32) | [Definition](/stdlib/system.qnp?plain=1#L82)
```qinp
Launch a shell command
@param command The command to launch
@return The exit status of the launched command
```

