from __future__ import annotations

from dataclasses import dataclass
from typing import List
import json


@dataclass
class Token:
    type: str
    content: str


class Lexer:
    source: str = ""
    pos: int = 0
    c: str = ""

    def __init__(self, in_source: str):
        self.source = self.read_source(in_source)
        self.c = self.source[0]

    def read_source(self, filename: str) -> str:
        with open(filename, "r") as f:
            source = f.read()
        return source

    def next(self):
        self.pos += 1
        if self.pos < len(self.source):
            self.c = self.source[self.pos]
        else:
            self.c = "\0"

    def lex(self) -> List[Token]:
        self.pos = 0
        tokens = []

        while self.pos < len(self.source):
            # Ignore whitespace
            if self.c in (" ", "\t", "\n"):
                self.next()
                continue

            # Numbers
            if self.c.isdigit():
                value = ""
                while self.c.isdigit():
                    value += self.c
                    self.next()
                t = Token("Number", value)
                tokens.append(t)
                continue

            t = Token(self.c, self.c)
            tokens.append(t)
            self.next()

        return tokens


class Parser:
    tokens: List[Token]
    pos: int
    size: int

    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0
        self.size = len(tokens)

    def next(self) -> Token:
        token = self.tokens[self.pos]
        self.pos += 1
        return token

    def match(self, value: str) -> bool:
        if self.pos == self.size:
            return False
        return self.tokens[self.pos].type == value

    def parse_literal(self) -> dict:
        token = self.next()
        return {"type": "Literal", "value": token.content}

    def parse_multiplicative_expr(self):
        expr = self.parse_literal()
        while self.match("*") or self.match("/"):
            expr = {
                "type": "BinOp",
                "operator": self.next().type,
                "left": expr,
                "right": self.parse_literal(),
            }
        return expr

    def parse_additive_expr(self):
        expr = self.parse_multiplicative_expr()
        while self.match("+") or self.match("-"):
            expr = {
                "type": "BinOp",
                "operator": self.next().type,
                "left": expr,
                "right": self.parse_multiplicative_expr(),
            }
        return expr

    def parse(self):
        return self.parse_additive_expr()

    def eval_binop(self, expr: dict) -> int:
        # Evaluate left
        ltype = expr["left"]["type"]
        if ltype == "BinOp":
            lvalue = self.eval_binop(expr["left"])
        elif ltype == "Literal":
            lvalue = expr["left"]["value"]
        else:
            raise Exception(f"Invalid Expr: {expr['type']}")

        # Evaluate right
        rtype = expr["right"]["type"]
        if rtype == "BinOp":
            rvalue = self.eval_binop(expr["right"])
        elif rtype == "Literal":
            rvalue = expr["right"]["value"]
        else:
            raise Exception(f"Invalid Expr: {expr['type']}")

        # Convert to numbers
        lvalue = int(lvalue)
        rvalue = int(rvalue)

        # Execute the operator
        op = expr["operator"]
        if op == "+":
            return lvalue + rvalue
        elif op == "-":
            return lvalue - rvalue
        elif op == "*":
            return lvalue * rvalue
        elif op == "/":
            return lvalue / rvalue
        else:
            raise Exception(f"Invalid operator: {op}")

    def eval(self, expr: dict) -> int:
        if expr["type"] == "BinOp":
            return self.eval_binop(expr)
        else:
            raise Exception("Unable to evaluate.")


if __name__ == "__main__":
    lexer = Lexer("hello.p")
    tokens = lexer.lex()
    parser = Parser(tokens)
    tree = parser.parse()
    print(json.dumps(tree, indent=4))
    print(parser.eval(tree))
