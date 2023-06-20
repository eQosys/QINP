
# Stdlib - stdlib/time.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [fn\<u64\> std.getTime() nodiscard ...](#ref_d6617d835a52997b1a3b0e2015bf811c)
 - [fn\<i32\> std.sleep(u64 ms) ...](#ref_79c78e99cd6aa4ad1529f8670b76c4a0)

## Details
#### <a id="ref_d6617d835a52997b1a3b0e2015bf811c"/>fn\<u64\> std.getTime() nodiscard ...
> [Declaration](/stdlib/time.qnp?plain=1#L7) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L34)
```qinp
Returns the current time in milliseconds since the epoch.
@return Current time in milliseconds
```
#### <a id="ref_79c78e99cd6aa4ad1529f8670b76c4a0"/>fn\<i32\> std.sleep(u64 ms) ...
> [Declaration](/stdlib/time.qnp?plain=1#L12) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L40)
```qinp
Sleep for the specified number of milliseconds
@param ms The number of milliseconds to sleep
@return Zero on success, negative on error
```

