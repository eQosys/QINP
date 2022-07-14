# Blueprints

Blueprints are used to define generalized language elements which can work with different types of data.

### Overview
 - [Syntax](./blueprints.md#syntax)
 - [Functions](./blueprints.md#functions)
 - [Variadic Functions](./blueprints.md#variadic-functions)

---

### Syntax

#### Usage

```qinp
blueprint [param1], [param2], ...
[specific code]
```

---

### Functions

#### Usage

> Declaration:
> ```qinp
> blueprint [blueprint params]
> [retType] [name]([parameters])...
> ```

> Definition:
> ```qinp
> blueprint [blueprint params]
> [retType] [name]([parameters]):
>     [function body]
> ```

#### Example

```qinp
blueprint T
void printValue(u8 const* name, T value):
	std.print(name)
	std.print(": ")
	std.print(value)
	std.print("\n")
```

---

### Variadic Functions

Variadic functions are functions which can take an arbitrary number of arguments. Variadic functions are declared by having `...` as the last parameter in a function declaration.

#### Usage

> Declaration:
> ```qinp
> blueprint [optional blueprint params]
> [retType] [name]([parameters], ...)...
> ```

> Definition:
> ```qinp
> blueprint [optional blueprint params]
> [retType] [name]([parameters], ...):
>     [function body]
> ```

#### Example

```qinp
blueprint T
T sum(T a, ...):
	return a + sum(...)

blueprint T
T sum(T a, T b):
	return a + b
```