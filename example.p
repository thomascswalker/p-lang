array = [1,2,3];
append(array, 4);
print(array);
append(array, 5);
print(array);

i = 0;
a = 0;
b = 1;

print("Here's the fibbonaci sequence:");
fib_array = [];
while (i < 30)
{
    append(fib_array, a);
    temp = a + b;
    a = b;
    b = temp;
    i += 1;
}
print(fib_array);