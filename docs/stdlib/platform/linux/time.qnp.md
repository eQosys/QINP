
# Stdlib - stdlib/platform/linux/time.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<i32\> std.__clock_gettime(i32 clockID, std.__timespec* pts)](#ref_c65bc21092403eded3208bfa92e082ad)
 - [fn\<u64\> std.getTime() nodiscard](#ref_3625b747e3afa4a403f29c51f9886bc5)
 - [fn\<i32\> std.sleep(u64 ms)](#ref_811d72065394f0ca8d3b5353ef372bde)

## Packs/Unions
 - pack std.__timespec

## Macros
 - std.__CLOCK_MONOTONIC
 - std.__CLOCK_PROCESS_CPUTIME_ID
 - std.__CLOCK_REALTIME
 - std.__CLOCK_THREAD_CPUTIME_ID

## Details
#### <a id="ref_c65bc21092403eded3208bfa92e082ad"/>fn\<i32\> std.__clock_gettime(i32 clockID, std.__timespec* pts)
> [Declaration](/stdlib/platform/linux/time.qnp?plain=1#L21) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L27)
```qinp
\\ Retrieves the current time of the system
\\ @param clockID The clock to use
\\ @param t A pointer to a __timespec pack to store the time in
\\ @return 0 on success, -1 on error
```
#### <a id="ref_3625b747e3afa4a403f29c51f9886bc5"/>fn\<u64\> std.getTime() nodiscard
> [Declaration](/stdlib/time.qnp?plain=1#L7) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L30)
```qinp
\\ Returns the current time in milliseconds since the epoch.
\\ @return Current time in milliseconds
```
#### <a id="ref_811d72065394f0ca8d3b5353ef372bde"/>fn\<i32\> std.sleep(u64 ms)
> [Declaration](/stdlib/time.qnp?plain=1#L12) | [Definition](/stdlib/platform/linux/time.qnp?plain=1#L36)
```qinp
\\ Sleep for the specified number of milliseconds
\\ @param ms The number of milliseconds to sleep
\\ @return Zero on success, negative on error
```

