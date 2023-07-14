
# Stdlib - stdlib/platform/linux/fileio.qnp

## Overview
 - [Functions](#functions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<\> std.__fclose(i64 fd)](#ref_e867971f564d362b0879dd5b5c2f0016)
 - [fn\<i64\> std.__fopen(u8 const* filename, u8 const* modeStr) nodiscard](#ref_9adc735223aa4f9dcf7fe6a91728ebb8)
 - [fn\<u64\> std.__fread(i64 fd, void* dest, u64 num)](#ref_537bdcb1682ebb5533faf7cdda5bd5f1)
 - [fn\<u64\> std.__fseek(i64 fd, u64 offset, std.File.Seek origin)](#ref_9a3244ca8e00b3a063a92ad446a951b1)
 - [fn\<u64\> std.__fsize(i64 fd) nodiscard ...](#ref_9369a68611b8f987e5f3704cfec2401f)
 - [fn\<u64\> std.__fwrite(i64 fd, void const* src, u64 num)](#ref_7ebf11b3da02555be58f4e4e5321cc10)

## Macros
 - std.__FILE_DESC
 - std.__OPEN_CREATE
 - std.__OPEN_READONLY
 - std.__OPEN_READWRITE
 - std.__OPEN_TRUNCATE
 - std.__OPEN_WRITEONLY

## Details
#### <a id="ref_e867971f564d362b0879dd5b5c2f0016"/>fn\<\> std.__fclose(i64 fd)
> [Declaration](/stdlib/platform/linux/fileio.qnp?plain=1#L61) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L140)
```qinp
Closes the file.
@param pFile The file object.
```
#### <a id="ref_9adc735223aa4f9dcf7fe6a91728ebb8"/>fn\<i64\> std.__fopen(u8 const* filename, u8 const* modeStr) nodiscard
> [Declaration](/stdlib/platform/linux/fileio.qnp?plain=1#L27) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L87)
```qinp
Opens a file for reading and/or writing.
If the file could not be opened, null is returned.
@param filename The name of the file to open.
@param mode The mode to open the file in. The mode can be one of the following:
 - "r" for reading.
 - "w" for writing.
 - "rw" for reading and writing.
@return The file object.
```
#### <a id="ref_537bdcb1682ebb5533faf7cdda5bd5f1"/>fn\<u64\> std.__fread(i64 fd, void* dest, u64 num)
> [Declaration](/stdlib/platform/linux/fileio.qnp?plain=1#L35) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L110)
```qinp
Reads num bytes from the file into the destination buffer.
May return less than num bytes if the end of the file is reached.
@param pFile The file object.
@param dest The destination buffer.
@param num The number of bytes to read.
@return The number of bytes read.
```
#### <a id="ref_9a3244ca8e00b3a063a92ad446a951b1"/>fn\<u64\> std.__fseek(i64 fd, u64 offset, std.File.Seek origin)
> [Declaration](/stdlib/platform/linux/fileio.qnp?plain=1#L52) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L124)
```qinp
Seeks to the specified offset in the file.
@param pFile The file object.
@param offset The offset to seek to.
@param origin The origin to seek from. The origin can be one of the following:
 - Seek.Begin for the beginning of the file.
 - Seek.Current for the current position in the file.
 - Seek.End for the end of the file.
@return The new position in the file, U64_MAX on error.
```
#### <a id="ref_c81cfecb66a99d799b853ee646eed0a7"/>i64 fd
> [Declaration](/stdlib/platform/linux/fileio.qnp?plain=1#L57)
```qinp
Returns the size of the file in bytes.
@param pFile The file object.
@return The size of the file in bytes, U64_MAX on error.
```
#### <a id="ref_9369a68611b8f987e5f3704cfec2401f"/>fn\<u64\> std.__fsize(i64 fd) nodiscard ...
> [Declaration](/stdlib/platform/linux/fileio.qnp?plain=1#L57)
```qinp
Returns the size of the file in bytes.
@param pFile The file object.
@return The size of the file in bytes, U64_MAX on error.
```
#### <a id="ref_7ebf11b3da02555be58f4e4e5321cc10"/>fn\<u64\> std.__fwrite(i64 fd, void const* src, u64 num)
> [Declaration](/stdlib/platform/linux/fileio.qnp?plain=1#L42) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L117)
```qinp
Writes num bytes from the source buffer to the file.
@param pFile The file object.
@param src The source buffer.
@param num The number of bytes to write.
@return The number of bytes written.
```

