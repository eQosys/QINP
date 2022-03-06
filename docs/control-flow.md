# Control Flow

## Overview
 - [if-elif-else](#if-elif-else)
 - [while loop](#while-loop)
 - [do-while loop](#do-while-loop)

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
u32 i = 0
while ++i < 10:
	print(i)	\\ Prints the numbers 1 to 9
	print("\n")
```

---

### do-while loop

Do-while loops are used to execute a block of code at least once, and then repeat the loop as long as the condition evaluates to true.

#### Usage
```qinp
do:
	[body]
while [condition]
```

#### Example
```qinp
u32 i = 0
do:
	print(i)	\\ Prints the numbers 0 to 9
	print("\n")
while ++i < 10