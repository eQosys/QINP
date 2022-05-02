# Standard Library Reference

The Standard Library is a collection of functions, packs and definitions that are available to all QINP programs and are essential to the language.

All files building the Standard Library can be found in the [stdlib](../../stdlib/) directory:
 - [./math.qnp](./math-qnp.md)
 - [./memory.qnp](./memory-qnp.md)
 - [./std.qnp](./std-qnp.md)
 - [./string.qnp](./string-qnp.md)
 - [./linux/memory.qnp](./linux/memory-qnp.md)
 - [./linux/std.qnp](./linux/std-qnp.md)
 - [./linux/syscall.qnp](./linux/syscall-qnp.md)
 - [./windows/std.qnp](./windows/std-qnp.md)

The main file for the Standard Library is [./std.qnp](./std-qnp.md). It imports the most fundamental functions, definitions and packs.

Every Function/pack defined in the standard library exists in the `std` space.

Identifiers with a leading double underscore are implementation details and should not be used directly.