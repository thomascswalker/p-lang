array = [1,2,3];
append(array, 4);
print(array);
append(array, 5);
print(array);

i = 0;
a = 0;
b = 1;

print("Here's the fibbonaci sequence:");
while (i < 10)
{
    print(i, a);
    temp = a + b;
    a = b;
    b = temp;
    i += 1;
}