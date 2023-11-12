def is_whitespace(ch, result)
{
    if (ch == " ")
    {
        result = true;
    }
    else
    {
        result = false;
    }
}

// Read the file
source = "";
read_file(".\simple.p", source);

// Get the size of the file
size = 0;
size_of(source, size);

// Get each character in the file
c = "";
i = 0;
tokens = [];
while (i < size)
{
    // iterable, index, out
    index_of(source, i, c);

    r = false;
    is_whitespace(c, r);
    if (r == false)
    {
        append(tokens, c);
    }
    else
    {
    }
    i += 1;
}

print(tokens);