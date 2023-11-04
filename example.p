i = 0;
a = 0;
b = 1;

fib = [];
append(fib, a);
while (i < 10)
{
    temp = a + b;
    a = b;
    b = temp;
    append(fib, a);

    i += 1;
}
print(fib);