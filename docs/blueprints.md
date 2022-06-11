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

```qinp
blueprint [param1], [param2], ...
[function declaration/definition]
```

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

#### Usage

Variadic functions are functions which can take an arbitrary number of arguments. Their last argument must be `...`.

```qinp
blueprint [param1], [param2], ...
[function declaration/definition]
```

#### Example

```qinp
blueprint T
T sum(T a, ...):
	return a + sum(...)

blueprint T
T sum(T a, T b):
	return a + b
```