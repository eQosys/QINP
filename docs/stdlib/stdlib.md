
# Standard Library Reference

The Standard Library is a collection of functions, packs and definitions that are available to all QINP programs and are essential to the language.

All files building the Standard Library can be found in the [stdlib](../../stdlib/) directory:
 - [./fileio.qnp](fileio.qnp.md)
 - [./math.qnp](math.qnp.md)
 - [./memory.qnp](memory.qnp.md)
 - [./platform/linux/fileio.qnp](platform/linux/fileio.qnp.md)
 - [./platform/linux/memory.qnp](platform/linux/memory.qnp.md)
 - [./platform/linux/std.qnp](platform/linux/std.qnp.md)
 - [./platform/linux/syscall.qnp](platform/linux/syscall.qnp.md)
 - [./platform/linux/system.qnp](platform/linux/system.qnp.md)
 - [./platform/linux/time.qnp](platform/linux/time.qnp.md)
 - [./std.qnp](std.qnp.md)
 - [./string.qnp](string.qnp.md)
 - [./system.qnp](system.qnp.md)
 - [./time.qnp](time.qnp.md)


The main file for the Standard Library is [./std.qnp](./std.qnp.md). It imports the most fundamental functions, definitions and packs.

Every Function/pack defined in the standard library exists in the `std` space.

Identifiers with a leading double underscore are implementation details and should not be used directly.
