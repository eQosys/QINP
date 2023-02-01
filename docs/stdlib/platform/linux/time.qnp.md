
# Stdlib - stdlib/platform/linux/time.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn<i32> std.__clock_gettime(i32 clockID, std.__timespec* pts)](#ref_b2a541613ccc739589d4abf777dec202)
 - [fn<u64> std.getTime() nodiscard](#ref_a78766b95a50084ed85ef8e95706f809)
 - [fn<i32> std.sleep(u64 ms)](#ref_a24a6505449e20f9ee3e4c93c8cd73d5)

## Packs/Unions
 - pack std.__timespec

## Macros
 - std.__CLOCK_MONOTONIC
 - std.__CLOCK_PROCESS_CPUTIME_ID
 - std.__CLOCK_REALTIME
 - std.__CLOCK_THREAD_CPUTIME_ID

## Details
#### <a id="ref_b2a541613ccc739589d4abf777dec202"/>fn<i32> std.__clock_gettime(i32 clockID, std.__timespec* pts)
> [Declaration](/stdlib/platform/linux/time.qnp?plain=1#L21) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L27)
```qinp
\\ Retrieves the current time of the system
\\ @param clockID The clock to use
\\ @param t A pointer to a __timespec pack to store the time in
\\ @return 0 on success, -1 on error
```
#### <a id="ref_a78766b95a50084ed85ef8e95706f809"/>fn<u64> std.getTime() nodiscard
> [Declaration](/stdlib/time.qnp?plain=1#L7) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L30)
```qinp
\\ Returns the current time in milliseconds since the epoch.
\\ @return Current time in milliseconds
```
#### <a id="ref_a24a6505449e20f9ee3e4c93c8cd73d5"/>fn<i32> std.sleep(u64 ms)
> [Declaration](/stdlib/time.qnp?plain=1#L12) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L36)
```qinp
\\ Sleep for the specified number of milliseconds
\\ @param ms The number of milliseconds to sleep
\\ @return Zero on success, negative on error
```

