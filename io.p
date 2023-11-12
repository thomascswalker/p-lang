// Print the given argument
def print_test(arg)
{
    print(arg);
}

// Read the file
source = read_file(".\simple.p");
print(source);

i = 0;
tokens = [];

// for (c : source)
while (i < size_of(source))
{
    c = index_of(source, i);
    append(tokens, c);
    i += 1;
}

print_test(tokens);