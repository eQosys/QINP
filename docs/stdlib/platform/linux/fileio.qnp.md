
# Stdlib - stdlib/platform/linux/fileio.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn<> std.fclose(std.File* pFile)](#ref_35169ca5dd216e528e147ea957cba2d8)
 - [fn<std.File*> std.fopen(u8 const* filename, u8 const* mode) nodiscard](#ref_ece803274f97461d27d16cd3b47c8fc7)
 - [fn<u64> std.fread(std.File* pFile, void* dest, u64 num)](#ref_9d7f70070618d92ce8009ca46d8f90a0)
 - [fn<u64> std.fwrite(std.File* pFile, void const* src, u64 num)](#ref_e0a479a319c37a1d66962080214f3742)

## Packs/Unions
 - [pack std.File]

## Macros
 - [std.__OPEN_READONLY]
 - [std.__OPEN_READWRITE]
 - [std.__OPEN_WRITEONLY]

## Details
#### <a id="ref_35169ca5dd216e528e147ea957cba2d8"/>fn<> std.fclose(std.File* pFile)
> [Declaration](/stdlib/fileio.qnp?plain=1#L36) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L42)
```qinp
\\ Closes the file.
\\ @param pFile The file object.
```
#### <a id="ref_ece803274f97461d27d16cd3b47c8fc7"/>fn<std.File*> std.fopen(u8 const* filename, u8 const* mode) nodiscard
> [Declaration](/stdlib/fileio.qnp?plain=1#L17) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L21)
```qinp
\\ Opens a file for reading and/or writing.
\\ If the file could not be opened, null is returned.
\\ @param filename The name of the file to open.
\\ @param mode The mode to open the file in. The mode can be one of the following:
\\  - "r" for reading.
\\  - "w" for writing.
\\  - "rw" for reading and writing.
\\ @return The file object.
```
#### <a id="ref_9d7f70070618d92ce8009ca46d8f90a0"/>fn<u64> std.fread(std.File* pFile, void* dest, u64 num)
> [Declaration](/stdlib/fileio.qnp?plain=1#L25) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L36)
```qinp
\\ Reads num bytes from the file into the destination buffer.
\\ May return less than num bytes if the end of the file is reached.
\\ @param pFile The file object.
\\ @param dest The destination buffer.
\\ @param num The number of bytes to read.
\\ @return The number of bytes read.
```
#### <a id="ref_e0a479a319c37a1d66962080214f3742"/>fn<u64> std.fwrite(std.File* pFile, void const* src, u64 num)
> [Declaration](/stdlib/fileio.qnp?plain=1#L32) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L39)
```qinp
\\ Writes num bytes from the source buffer to the file.
\\ @param pFile The file object.
\\ @param src The source buffer.
\\ @param num The number of bytes to write.
\\ @return The number of bytes written.
```

