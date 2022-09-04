# Declarations

Declarations add new names to QINP programs.
The QINP language follows the ODR (One Definition Rule).

## Overview
 - [Variables](#variables)
 - [Functions](#functions)
 - [Packs](#packs)
 - [Unions](#unions)
 - [Static](#static)
 - [Enums](#enums)

---

### Variables
Every variable declaration is a definition, variables have a type and a name.

Variables can be initialized with an expression.

Arrays can be declared with a size specified. The size must be a literal integer.

Multidimensional arrays are supported.
Arrays can not be initialized.

#### Usage

> Simple Declaration/Definition
>
> var<`datatype`> `name`

> Simple Declaration/Definition with initialization
>
> var<`datatype`> `name` = `expression`

> Array Declaration/Definition
>
> var<`datatype`> `name`[`size`]

#### Examples

> Declaration/Definition
> ```qinp
> var<u32*> foo
> ```

> Declaration/Definition with initialization
> ```qinp
> var<u32> bar = 100
> ```

> Array Declaration/Definition
> ```qinp
> var<u16> baz[10]
> var<u64> qux[10][8]
> ```

---

### Functions

A function declaration associates a name with a return type and a list of zero or more parameters.
A function definition additionally associates a list of statements.

It is possible to give a function definition or declaration a pre-declaration requirement. This can be done via a `!` after the parenthesised parameter list.
This can be useful for functions that have different definitions depending on the platform.
They are heavily used in the QINP standard library.

#### Usage

> Declaration
> ```qinp
> fn<[return-type]> [name] ( [parameter-list] ) [!*]...
> ```

> Definition
> ```qinp
> fn<[return-type]> [name] ( [parameter-list] ) [!*]:
> 	[body]

#### Examples

> Declaration
> ```qinp
> fn<u8> foo(u32 a, u16* b)...
> ```
> ```qinp
> fn<void> func()!...
> ```

> Definition
> ```qinp
> fn<u64> square(u64 x):
>    return x * x
> ```
> ```qinp
> fn<void> func()!:
>    return
> ```

#### Note

When declaring or defining a void function, the return type and '<>' can be omitted.
The following three function declarations are all equally valid.

> ```qinp
> fn<void> func1()...
> fn<> func2()...		\\ Used in the standard library
> fn func3()...
> ```

---

### Packs

A pack is a collection of variables (called members). They are similar to structs in C++.
They can be declared and defined.
A pack that has only been declared can not be used directly to instantiate a new object, but pointers to an declared-only pack can be used.
Before accessing any member of a pack, the pack must be defined.
Packs can hold pointers to objects of the same pack type.
The size of a pack is constant and only known after the pack's definition. The [sizeof operator](./operators.md#sizeof) can be used on defined packs, but not on declared-only ones.

#### Usage

> Declaration
> ```qinp
> pack [name]...
> ```

> Definition
> ```qinp
> pack [name]:
> 	[member-1]
> 	[member-2]
> 	...
> ```

#### Examples

> Declaration
> ```qinp
> pack User...
> ```

> Definition
> ```qinp
> pack User:
>    var<u32> age
>    var<u8> name[32]
>    var<User*> friends[10]
> ```

---

### Unions

Unions are similar to packs, except that each member shares it's memory with all other members.
The size of the union is the size of the largest member.

#### Usage

> Declaration
> ```qinp
> union [name]...
> ```

> Definition
> ```qinp
> union [name]:
> 	[member-1]
> 	[member-2]
> 	...
> ```

#### Examples

> Declaration
> ```qinp
> union Data...
> ```

> Definition
> ```qinp
> union Data:
> 	var<u32> var
> 	var<u8> name[5]
> ```

---

### Static

The `static` keyword can be used to declare/define a variable in a function as static, which means that its state is preserved between function calls (like a global variable), without being visible outside the function. When defining a static variable, the initializer is only run once (with the first call of the function).

#### Example

```qinp
fn<> foo():
	static var<u32> x
	std.print(++x)
```

---

### Enums

The `enum` keyword can be used to declare an enumeration.
Every member is associated with an value of type i64.
The members of enums are treated as literals.
A variable of an enum type can not be implicitly converted to another enum or builtin type.
The members of an enum are accessed via the [space operator](./operators.md#namespace).

The first element of an enum is 0 by default. The following elements are incremented by 1.
This behavior can be overridden by specifying a custom value for an element.

#### Usage

```qinp
enum [enumName]:
	[memberName1], [memberName2]
	[memberName3] = [value], [memberName4], [...]
	[memberNameN]

[enumName].[memberNameX]
```

#### Example

```qinp
enum Color:
	red, green, blue,
	black = 5, white, yellow		\\ The comma after the last member of a row is optional
	cyan = 3, magenta = -1, orange

var<Color> col = Color.red
```