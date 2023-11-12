// Read the next token at the specified index.
// i (int): The current index.
def next(i)
{
    c = index_of(source, i);
    append(tokens, c);
    return c;
}

// Read the file
source = read_file(".\simple.p");
print(source);

// Parse the tokens
i = 0;
tokens = [];
while (i < size_of(source))
{
    r = next(i);
    print(r);
    i += 1;
}

// Print the tokens
print(tokens);