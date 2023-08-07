
# Stdlib - stdlib/control.qnp

## Overview
 - [Globals](#globals)
 - [Functions](#functions)
 - [Enums](#enums)
 - [Macros](#macros)
 - [Details](#details)


## Globals
 - var\<i64\> std.Control.__currBuffer
 - var\<std.String*[8]\> std.Control.buffers

## Functions
 - fn\<std.String*\> std.Control.__getNextBuffer() nodiscard
 - [fn\<\> std.Control.__handleControl(std.String* buffer, std.Control.ID id, bool append)](#ref_fc277bc37da7308c02e5e3378e9792de)
 - [fn\<\> std.Control.__make(std.String* buffer, i64 param, ...)](#ref_5238c0407e395a43605da6b9ed5b4d84)
 - [fn\<\> std.Control.__make(std.String* buffer, std.Control.ID id, ...)](#ref_0f668532f2c85d72775a8ce4861641e9)
 - [fn\<\> std.Control.__make(std.String* buffer, i64 param)](#ref_dfda18dcb7d9ad0fec2d3da3a0beb0c7)
 - [fn\<\> std.Control.__make(std.String* buffer, std.Control.ID id)](#ref_151f3b365300f2694b74ed5243090fb6)
 - [fn\<u8 const*\> std.Control.make(std.Control.ID id, ...) nodiscard](#ref_77bf99ba3c96a76c81dd4607c3d3e365)
 - [fn\<u8 const*\> std.Control.make(std.Control.ID id) nodiscard](#ref_b0d9c23984e1c1c38b50e2f5e34ee20e)

## Enums
 - [enum std.Control.ID](#ref_aca359a7b1179533e9299cbaf9dde206)

## Macros
 - std.Control.__N_BUFFERS

## Details
#### <a id="ref_aca359a7b1179533e9299cbaf9dde206"/>enum std.Control.ID
> [Declaration](/stdlib/control.qnp?plain=1#L10)
```qinp
Control IDs
```
#### <a id="ref_fc277bc37da7308c02e5e3378e9792de"/>fn\<\> std.Control.__handleControl(std.String* buffer, std.Control.ID id, bool append)
> [Declaration](/stdlib/control.qnp?plain=1#L56) | [Definition](/stdlib/control.qnp?plain=1#L108)
```qinp
Handles the control ID and appends required data when needed
@param buffer The buffer to append to
@param id The ID of the control sequence
@param append Whether to append to the buffer or clear it
```
#### <a id="ref_5238c0407e395a43605da6b9ed5b4d84"/>fn\<\> std.Control.__make(std.String* buffer, i64 param, ...)
> [Declaration](/stdlib/control.qnp?plain=1#L67) | [Definition](/stdlib/control.qnp?plain=1#L128)
```qinp
Appends multiple parameters to the buffer
@param buffer The buffer to append to
@param param The first parameter to append
@param ... The rest of the parameters to append
```
#### <a id="ref_0f668532f2c85d72775a8ce4861641e9"/>fn\<\> std.Control.__make(std.String* buffer, std.Control.ID id, ...)
> [Declaration](/stdlib/control.qnp?plain=1#L78) | [Definition](/stdlib/control.qnp?plain=1#L136)
```qinp
Appends a control ID and multiple parameters to the buffer
@param buffer The buffer to append to
@param id The ID of the control sequence
@param ... The parameters of the control sequence
```
#### <a id="ref_dfda18dcb7d9ad0fec2d3da3a0beb0c7"/>fn\<\> std.Control.__make(std.String* buffer, i64 param)
> [Declaration](/stdlib/control.qnp?plain=1#L61) | [Definition](/stdlib/control.qnp?plain=1#L124)
```qinp
Appends a parameter to the buffer
@param buffer The buffer to append to
@param param The parameter to append
```
#### <a id="ref_151f3b365300f2694b74ed5243090fb6"/>fn\<\> std.Control.__make(std.String* buffer, std.Control.ID id)
> [Declaration](/stdlib/control.qnp?plain=1#L72) | [Definition](/stdlib/control.qnp?plain=1#L133)
```qinp
Appends a control ID to the buffer
@param buffer The buffer to append to
@param id The ID of the control sequence
```
#### <a id="ref_77bf99ba3c96a76c81dd4607c3d3e365"/>fn\<u8 const*\> std.Control.make(std.Control.ID id, ...) nodiscard
> [Declaration](/stdlib/control.qnp?plain=1#L50) | [Definition](/stdlib/control.qnp?plain=1#L98)
```qinp
Generates a string representing a control sequence
The returned string is valid until the next call to this function
@param id The ID of the control sequence
@param ... The parameters of the control sequence
@return The control sequence string
```
#### <a id="ref_b0d9c23984e1c1c38b50e2f5e34ee20e"/>fn\<u8 const*\> std.Control.make(std.Control.ID id) nodiscard
> [Declaration](/stdlib/control.qnp?plain=1#L43) | [Definition](/stdlib/control.qnp?plain=1#L89)
```qinp
Generates a string representing a control sequence
The returned string is valid until the next call to this function
@param id The ID of the control sequence
@return The control sequence string
```

