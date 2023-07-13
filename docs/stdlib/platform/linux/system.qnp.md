
# Stdlib - stdlib/platform/linux/system.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [fn\<i32\> std.__system(u8 const* progPath, u8* const* argv, u8* const* envp)](#ref_3572f4eeaec019299c9b7e2698f6d37f)
 - [fn\<\> std.exit(i32 status)](#ref_88820b50f3813315f9ee27702ff470fd)

## Details
#### <a id="ref_3572f4eeaec019299c9b7e2698f6d37f"/>fn\<i32\> std.__system(u8 const* progPath, u8* const* argv, u8* const* envp)
> [Declaration](/stdlib/system.qnp?plain=1#L58) | [Definition](/stdlib/platform/linux/system.qnp?plain=1#L16)
```qinp
Internal function to launch a program
@param progPath The path to the program to launch
@param argv The arguments to pass to the program
@param envp The environment variables to pass to the program
@return The exit status of the launched program
```
#### <a id="ref_88820b50f3813315f9ee27702ff470fd"/>fn\<\> std.exit(i32 status)
> [Declaration](/stdlib/system.qnp?plain=1#L18) | [Definition](/stdlib/platform/linux/system.qnp?plain=1#L13)
```qinp
Terminate the calling process and return the specified exit status.
@param status The exit status to return to the parent process.
```

