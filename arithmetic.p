a = 1;
b = 2;
c = a + b;
result = c == 3;
print(result);

a = 5;
b = 10;
c = a * b;
result = c == 50;
print(result);

a = 0;
b = 1;
i = 0;
while (i < 20)
{
    t = b;
    b += a;
    a = t;
    i += 1;
}
result = a == 6765;
print(result);

a = 5 + 10 * 20 + 30 / 40;
b = 5 + 10 * 20 + 30 / 40.0;
result = a == 205;
print(result);
result = b == 205.75;
print(result);