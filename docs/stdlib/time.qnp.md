
# Stdlib - stdlib/time.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [fn<u64> std.getTime() nodiscard ...](#ref_b6a0c9f4f9da322ffb97cc5522ba6586)
 - [fn<i32> std.sleep(u64 ms) ...](#ref_977fd3d0a570cb32c6fc18422594dd9f)

## Details
#### <a id="ref_b6a0c9f4f9da322ffb97cc5522ba6586"/>fn<u64> std.getTime() nodiscard ...
> [Declaration](/stdlib/time.qnp?plain=1#L7) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L30)
```qinp
\\ Returns the current time in milliseconds since the epoch.
\\ @return Current time in milliseconds
```
#### <a id="ref_977fd3d0a570cb32c6fc18422594dd9f"/>fn<i32> std.sleep(u64 ms) ...
> [Declaration](/stdlib/time.qnp?plain=1#L12) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L36)
```qinp
\\ Sleep for the specified number of milliseconds
\\ @param ms The number of milliseconds to sleep
\\ @return Zero on success, negative on error
```

