// Run the fibonnaci sequence until some upper limit
// Functions can be defined with 'function', 'func', 'fn', or 'def'

def fib(a, b)
{
    sum = a + b;
    a = b;
    b = sum;
}

def fib_to(limit)
{
    a = 0;
    b = 1;
    while (b < limit)
    {
        print(a);
        fib(a, b);
    }
}

fib_to(1000);
print(a);
