# Penguin Language

## Goals
Feature set similar to Python with optional type-safety and brackets.

## Features
- Dynamically typed
- Ignores whitespace
- Basic mutable types

## TODO:
- [x] Variables
- [x] Arithmetic
- [x] `if`, `else`
- [x] `while`
- [x] Unary operators (`++`, `--`, `[]`, etc.)
- [ ] Compound operators (`+=`, `-=`, etc.)
- [ ] Negative numbers
- [ ] `else if`
- [ ] `and`, `or`
- [ ] Printing
- [ ] `break`
- [ ] Function definitions
- [ ] Function call
- [ ] Type method call (`list.size()`)

## Grammar

| Expression | Grammar |
| --- | --- |
| Body       | ```Expr*``` |
| Expr       | ```Equality | Assignment ;``` |
| Assignment | ```Name ( = | += | -= | *= | /= ) Expr``` |
| Equality   | ```Comparison ( != | == ) Comparison``` |
| Comparison | ```Sum ( < | > | <= | >= ) Sum``` |
| Sum        | ```Product ( + | - ) Product``` |
| Product    | ```Unary ( * | / ) Unary``` |
| Unary      | ```( ! | - ) Value``` |
| Value      | ```Number | String | Bool | Name | ( ... )``` |
