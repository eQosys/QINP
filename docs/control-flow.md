# Control Flow

## Overview
 - [if-elif-else](#if-elif-else)
 - [while loop](#while-loop)

---

### if-elif-else

If-elif-else statements are used to execute different code depending on the value of a condition.
The conditions may be any expression that evaluates (possibly through implicit conversion) to a boolean.
The elif and else statements are optional.

#### Usage
```qinp
if [condition]:
	[body]
elif [condition]:
	[body]
else:
	[body]
```

#### Example:
```qinp
if true:
	print("Hello")
elif false:
	print("World")
else:
	print("!")
```

---

### while loop

While loops are used to execute a block of code repeatedly as long as a condition evaluates to true.

#### Usage
```qinp
while [condition]:
	[body]
```

#### Example
```qinp
while true:
	print("Hello")
```