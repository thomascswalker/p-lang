// Read the next token at the specified index.
// i (int): The current index.

def next(idx)
{
    return index_of(source, idx);
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
    append(tokens, r);
    i += 1;
}

// Print the tokens
print(tokens);
