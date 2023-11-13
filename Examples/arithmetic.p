/*
Tests for basic arithmetic.
*/

// Addition
a = 1;
b = 2;
c = a + b;
printf("Addition: 1 + 2 = {}", c);
print("=========");

// Multiplication
a = 5;
b = 10;
c = a * b;
printf("Multiplication: 5 * 10 = {}", c);
print("=========");

// Order of operations
a = 5 + 10 * 20 + 30 / 40;
b = 5 + 10 * 20 + 30 / 40.0;
result = a == 205;
print("a == 205");
printf("Order of operations: {}", a);
print(result);
result = b == 205.75;
print("b == 205.75");
print(result);
print("=========");

// Fibonacci sequence
a = 0;
b = 1;
i = 0; // iterator
limit = 20;
while (i < limit)
{
    t = b;
    b += a;
    a = t;
    print(a);
    i += 1;
}
printf("Fibonnaci: {} iterations, ends with {}", limit, a);
print("=========");

// Implicit casting
a = 5;
b = 10.0;
c = a + b;
result = c == 15.0;
print("Implicit casting")
print(result)
print("=========");
