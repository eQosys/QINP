# Lambdas

Lambdas are a way to define short (single statement) anonymous functions. They are defined using the `lambda` keyword, followed by a return type and a list of parameters. The body of the lambda is defined by the next statement followed by a semicolon.

## Overview
 - [Usage](#usage)
 - [Examples](#examples)

---

## Usage
```qinp
lambda<`return-type`>(`parameter-list`): `statement`;
```

---

## Examples

```qinp
const sum = lambda<i32>(i32 a, i32 b): return a + b;
```

