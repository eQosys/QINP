# Control Flow

## Overview
 - [if-elif-else](#if-elif-else)

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