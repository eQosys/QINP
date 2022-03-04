# QINP Keywords

This page lists the keywords in the QINP language.
Keywords are special identifiers in the QINP language and cannot be used for any other purpose. The [builtin types](./builtin-types.md) are also treated as keywords.

## Overview
 - [asm](#inline-assembly)
 - [assembly](#inline-assembly)
 - [import](#import)
 - [pass](#pass)
 - [return](#return)

---

### Inline Assembly
Inline Assembly can marked through the `asm` and `assembly` keywords as single-line or multiline code respectively.

Variables can also be used in inline assembly. Global variables are replaced with their mangled name, local variables are replaced by their offset to the base pointer (including a +/- sign).

#### Usage

> Single-line assembly:
> ```qinp
> asm: [assembly string]
> ```

> Multiline assembly:
> ```qinp
> assembly:
>   [assembly string1]
>   [assembly string2]
>   ...
> ```

> Variables:
> ```qinp
> $([name])
> ```

#### Examples

```qinp
asm: "mov rax, [$(global_var)]"
asm: "add rax, [rbp $(local_var)]"
```

```qinp
assembly:
	"mov rax, 1"
	"add rax, 3"
```

Both examples are equivalent.

---

### Import

QINP code files can be imported using the `import` keyword.
A single file is imported once and all other imports of the same file are ignored.

The filepaths are resolved with the compiler import-directory options.
Resolving import paths relative to the importing file is planned.

#### Usage

```qinp
import [file_string]
```

#### Example

```qinp
import "std.qnp"
```

---

### Pass

---

### Return