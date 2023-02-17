
# Stdlib - stdlib/fileio.qnp

## Overview
 - [Functions](#functions)
 - [Packs/Unions](#packs-unions)
 - [Enums](#enums)
 - [Details](#details)


## Functions
 - [fn<> std.fclose(std.File* pFile) ...](#ref_9d88eebd2af888dcce049ae45aa49bb8)
 - [fn<std.File*> std.fopen(u8 const* filename, u8 const* mode) nodiscard ...](#ref_8b5607b8ea33422886283e593c621d1f)
 - [fn<u64> std.fread(std.File* pFile, void* dest, u64 num) ...](#ref_50cc280d0699a1cd11a369f7ae47577b)
 - [fn<u64> std.fseek(std.File* pFile, u64 offset, std.Seek origin) ...](#ref_42f8918d0a26f1de07fece93bcf07610)
 - [fn<u64> std.fwrite(std.File* pFile, void const* src, u64 num) ...](#ref_1f8563dcf12abebac75334241dac213a)

## Packs/Unions
 - pack std.File ...

## Enums
 - std.Seek

## Details
#### <a id="ref_9d88eebd2af888dcce049ae45aa49bb8"/>fn<> std.fclose(std.File* pFile) ...
> [Declaration](/stdlib/fileio.qnp?plain=1#L49) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L52)
```qinp
\\ Closes the file.
\\ @param pFile The file object.
```
#### <a id="ref_8b5607b8ea33422886283e593c621d1f"/>fn<std.File*> std.fopen(u8 const* filename, u8 const* mode) nodiscard ...
> [Declaration](/stdlib/fileio.qnp?plain=1#L20) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L21)
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
#### <a id="ref_50cc280d0699a1cd11a369f7ae47577b"/>fn<u64> std.fread(std.File* pFile, void* dest, u64 num) ...
> [Declaration](/stdlib/fileio.qnp?plain=1#L28) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L36)
```qinp
\\ Reads num bytes from the file into the destination buffer.
\\ May return less than num bytes if the end of the file is reached.
\\ @param pFile The file object.
\\ @param dest The destination buffer.
\\ @param num The number of bytes to read.
\\ @return The number of bytes read.
```
#### <a id="ref_42f8918d0a26f1de07fece93bcf07610"/>fn<u64> std.fseek(std.File* pFile, u64 offset, std.Seek origin) ...
> [Declaration](/stdlib/fileio.qnp?plain=1#L45) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L42)
```qinp
\\ Seeks to the specified offset in the file.
\\ @param pFile The file object.
\\ @param offset The offset to seek to.
\\ @param origin The origin to seek from. The origin can be one of the following:
\\  - Seek.Begin for the beginning of the file.
\\  - Seek.Current for the current position in the file.
\\  - Seek.End for the end of the file.
\\ @return The new position in the file, negative on error
```
#### <a id="ref_1f8563dcf12abebac75334241dac213a"/>fn<u64> std.fwrite(std.File* pFile, void const* src, u64 num) ...
> [Declaration](/stdlib/fileio.qnp?plain=1#L35) | [Definition](/stdlib/platform/linux/fileio.qnp?plain=1#L39)
```qinp
\\ Writes num bytes from the source buffer to the file.
\\ @param pFile The file object.
\\ @param src The source buffer.
\\ @param num The number of bytes to write.
\\ @return The number of bytes written.
```

