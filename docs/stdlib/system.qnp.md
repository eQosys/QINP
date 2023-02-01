
# Stdlib - stdlib/system.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn<> std.exit(i32 status) ...](#ref_f46683071af5a6ec20a057000130973f)
 - [fn<u64> std.getargs(u8 const* const** pargv) nodiscard](#ref_67eb185471cd244146fe40933a1a10a1)
 - [fn<u8 const* const*> std.getenv() nodiscard](#ref_8fb6ec3ee7f8ab51d6e5b5dbaf79d88c)

## Macros
 - std.EXIT_FAILURE
 - std.EXIT_SUCCESS

## Details
#### <a id="ref_f46683071af5a6ec20a057000130973f"/>fn<> std.exit(i32 status) ...
> [Declaration](/stdlib/system.qnp?plain=1#L10) | [Definition](/stdlib/platform/linux/system.qnp?plain=1#L13)
```qinp
\\ Terminate the calling process and return the specified exit status.
\\ @param status The exit status to return to the parent process.
```
#### <a id="ref_67eb185471cd244146fe40933a1a10a1"/>fn<u64> std.getargs(u8 const* const** pargv) nodiscard
> [Declaration](/stdlib/system.qnp?plain=1#L15) | [Definition](/stdlib/system.qnp?plain=1#L25)
```qinp
\\ Get the arguments passed to the program
\\ @param pargv Address of the pointer where the pointer of the first argument will be stored
\\ @return The number of arguments passed to the program
```
#### <a id="ref_8fb6ec3ee7f8ab51d6e5b5dbaf79d88c"/>fn<u8 const* const*> std.getenv() nodiscard
> [Declaration](/stdlib/system.qnp?plain=1#L19) | [Definition](/stdlib/system.qnp?plain=1#L42)
```qinp
\\ Get the environment variables passed to the program
\\ @return The pointer to an array of environment variables
```

