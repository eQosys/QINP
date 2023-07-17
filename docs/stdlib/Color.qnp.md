
# Stdlib - stdlib/Color.qnp

## Overview
 - [Functions](#functions)
 - [Details](#details)


## Functions
 - [fn\<u8 const*\> std.Color.reset() nodiscard](#ref_c87b592f1f544483e6bb868564f002ac)
 - [fn\<u8 const*\> std.Color.rgb(u8 r, u8 g, u8 b) nodiscard](#ref_9fdfd357ffbdc84032416de89a6382a3)

## Details
#### <a id="ref_c87b592f1f544483e6bb868564f002ac"/>fn\<u8 const*\> std.Color.reset() nodiscard
> [Declaration](/stdlib/Color.qnp?plain=1#L19) | [Definition](/stdlib/Color.qnp?plain=1#L38)
```qinp
Returns a string that resets the foreground color to the default value.
The string is valid until the next call to this function.
@return A string that resets the foreground color to the default value.
```
#### <a id="ref_9fdfd357ffbdc84032416de89a6382a3"/>fn\<u8 const*\> std.Color.rgb(u8 r, u8 g, u8 b) nodiscard
> [Declaration](/stdlib/Color.qnp?plain=1#L14) | [Definition](/stdlib/Color.qnp?plain=1#L26)
```qinp
Returns a string that sets the foreground color to the specified RGB value.
The string is valid until the next call to this function.
@param r The red component of the color.
@param g The green component of the color.
@param b The blue component of the color.
@return A string that sets the foreground color to the specified RGB value.
```

