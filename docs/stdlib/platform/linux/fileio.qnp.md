
# Stdlib - stdlib/platform/linux/fileio.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Macros](#macros)
 - [Details](#details)


## Functions
 - [fn\<\> std.fclose(std.File* pFile)](#ref_5d668723bcfba6ce11011f7b6ae97bf6)
 - [fn\<std.File*\> std.fopen(u8 const* filename, u8 const* modeStr) nodiscard](#ref_f9331ab8684ba190709da22b2d519fa0)
 - [fn\<u64\> std.fread(std.File* pFile, void* dest, u64 num)](#ref_2fef09ea4725ef1fccbc8a59e2265bd5)
 - [fn\<u64\> std.fseek(std.File* pFile, u64 offset, std.Seek origin)](#ref_e88978765371b1f5ab1497fa5b56b8b5)
 - [fn\<u64\> std.fwrite(std.File* pFile, void const* src, u64 num)](#ref_a90a02b94bce7e88b866031f0500b730)

## Packs/Unions
 - [pack std.File](#ref_b25eae4d8f926169fb555cf1169a353f)

## Macros
 - std.__OPEN_CREATE
 - std.__OPEN_READONLY
 - std.__OPEN_READWRITE
 - std.__OPEN_TRUNCATE
 - std.__OPEN_WRITEONLY

## Details
#### <a id="ref_b25eae4d8f926169fb555cf1169a353f"/>pack std.File
> [Declaration](/stdlib/fileio.qnp?plain=1#L7) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L21)
```qinp
The implementation dependent file pack.
It should not be used directly/dereferenced outside of the implementation.
```
#### <a id="ref_5d668723bcfba6ce11011f7b6ae97bf6"/>fn\<\> std.fclose(std.File* pFile)
> [Declaration](/stdlib/fileio.qnp?plain=1#L50) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L81)
```qinp
Closes the file.
@param pFile The file object.
```
#### <a id="ref_f9331ab8684ba190709da22b2d519fa0"/>fn\<std.File*\> std.fopen(u8 const* filename, u8 const* modeStr) nodiscard
> [Declaration](/stdlib/fileio.qnp?plain=1#L21) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L24)
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
#### <a id="ref_2fef09ea4725ef1fccbc8a59e2265bd5"/>fn\<u64\> std.fread(std.File* pFile, void* dest, u64 num)
> [Declaration](/stdlib/fileio.qnp?plain=1#L29) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L51)
```qinp
Reads num bytes from the file into the destination buffer.
May return less than num bytes if the end of the file is reached.
@param pFile The file object.
@param dest The destination buffer.
@param num The number of bytes to read.
@return The number of bytes read.
```
#### <a id="ref_e88978765371b1f5ab1497fa5b56b8b5"/>fn\<u64\> std.fseek(std.File* pFile, u64 offset, std.Seek origin)
> [Declaration](/stdlib/fileio.qnp?plain=1#L46) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L65)
```qinp
Seeks to the specified offset in the file.
@param pFile The file object.
@param offset The offset to seek to.
@param origin The origin to seek from. The origin can be one of the following:
 - Seek.Begin for the beginning of the file.
 - Seek.Current for the current position in the file.
 - Seek.End for the end of the file.
@return The new position in the file, negative on error
```
#### <a id="ref_a90a02b94bce7e88b866031f0500b730"/>fn\<u64\> std.fwrite(std.File* pFile, void const* src, u64 num)
> [Declaration](/stdlib/fileio.qnp?plain=1#L36) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L58)
```qinp
Writes num bytes from the source buffer to the file.
@param pFile The file object.
@param src The source buffer.
@param num The number of bytes to write.
@return The number of bytes written.
```

