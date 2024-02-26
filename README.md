# Penguin Language

<!--TOC-->

- [Goals](#goals)
- [Features](#features)
- [Development](#development)
- [Example](#example)
    - [Basic arithmetic](#basic-arithmetic)
    - [Strings](#strings)
    - [Arrays](#arrays)
    - [Loops](#loops)
- [Grammar](#grammar)

<!--/TOC-->

![](https://github.com/p-lang/Images/interpreter.gif)

## Goals

Feature set similar to Python with optional type-safety and brackets.

## Features

- Dynamically typed
- Ignores whitespace
- Basic mutable types

## Development

- [x] Lexer
- [x] AST
- [x] Variables
- [x] Arithmetic
- [x] `if`, `else`
- [x] `while`
- [x] Unary operators
- [x] Index operator
- [x] Negative numbers
- [x] Compound operators
- [x] Built-in function call
- [x] Printing
- [x] Custom function definitions
- [x] Custom function call
- [x] Stack frames
- [ ] `format` print
- [ ] ++ and -- operators
- [ ] `else if`
- [ ] `and`, `or`
- [ ] `break` in `while` statements
- [ ] Function type hints
- [ ] Function type checking
- [ ] Imports

## Example

### Basic arithmetic

```c
var1 = 1;
var2 = 3.14159;
var3 = var1 + var2;
>>> 4.14159;
```

### Strings

```c
my_string = "This is a very long test string.";
my_string[0];
>>> "T"
my_string[-1];
>>> "."
```

### Arrays

```c
my_array = [1,2,3]
my_array[-1]
>>> 3

my_variant_array = [false, 1, "two", 3.0];
my_array[0]
>>> false
my_array[1]
>>> 1
```

### Loops

```c
my_array = [0,1,2,3,4,5];
i = 0;
while (i < 6)
{
	my_array[i]
	>>> 0, 1, 2 ...
	i += 1;
}

my_long_string = "abcdefghijklmnopqrstuvwxyz";
i = 0;
my_new_string = "";
while (i < 10)
{
	my_new_string += my_long_string[i];
	i += 1;
}
my_new_string
>>> "abcdefghij"
```

## Grammar

| Expression | Grammar                                           |
|------------|---------------------------------------------------|
| Body       | ```Expr*```                                       |
| Expr       | ```Equality \| Assignment ;```                    |
| Assignment | ```Name ( = \| += \| -= \| *= \| /= ) Expr```     |
| Equality   | ```Comparison ( != \| == ) Comparison```          |
| Comparison | ```Sum ( < \| > \| <= \| >= ) Sum```              |
| Sum        | ```Product ( + \| - ) Product```                  |
| Product    | ```Unary ( * \| / ) Unary```                      |
| Unary      | ```( ! \| - ) Value```                            |
| Value      | ```Number \| String \| Bool \| Name \| ( ... )``` |
