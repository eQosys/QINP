
# Stdlib - stdlib/assert.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<\> std.__assert(bool check, u8 const* msg, u8 const* file, i32 line)](#ref_f7c1565a7f7a6a4ebc8526f6438517ce)

## Macros
 - std.assert

## Details
#### <a id="ref_f7c1565a7f7a6a4ebc8526f6438517ce"/>fn\<\> std.__assert(bool check, u8 const* msg, u8 const* file, i32 line)
> [Declaration](/stdlib/assert.qnp?plain=1#L15) | [Definition](/stdlib/assert.qnp?plain=1#L21)
```qinp
\\ Internal assert function. Prints error message and exits if check is false.
\\ @param check Boolean expression to check.
\\ @param msg Error message to print.
\\ @param file File name where assert was called.
\\ @param line Line number where assert was called.
```

