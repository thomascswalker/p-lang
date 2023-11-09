/*
Tests for basic arithmetic.
*/

// Addition
a = 1;
b = 2;
c = a + b;
result = c == 3;
print("Addition");
print(result);

// Multiplication
a = 5;
b = 10;
c = a * b;
result = c == 50;
print("Multiplication");
print(result);

// Order of operations
a = 5 + 10 * 20 + 30 / 40;
b = 5 + 10 * 20 + 30 / 40.0;
result = a == 205;
print("Order of operations");
print("a == 205");
print(result);
result = b == 205.75;
print("b == 205.75");
print(result);

// Fibonacci sequence
a = 0;
b = 1;
i = 0; // iterator
while (i < 20)
{
    t = b;
    b += a;
    a = t;
    i += 1;
}
result = a == 6765;
print("Fibonacci");
print("a == 6765");
print(result);

// Implicit casting
a = 5;
b = 10.0;
c = a + b;
result = c == 15.0;
print("Implicit casting")
print(result)