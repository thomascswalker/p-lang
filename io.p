pos = 0;

ascii = ["a","b","c","d","e","f",
         "g","h","i","j","k","l",
         "m","n","o","p","q","r",
         "s","t","u","v","w","x",
         "y","z"];

def is_ascii(c)
{
    i = 0;
    result = false;
    while (i < size_of(ascii))
    {
        if (index_of(ascii, i) == c)
        {
            result = true;
        }
        i += 1;
    }
    return result;
}

numeric = ["1","2","3","4","5","6","7","8","9","0"];

def is_numeric(c)
{
    i = 0;
    result = false;
    while (i < size_of(numeric))
    {
        if (index_of(numeric, i) == c)
        {
            result = true;
        }
        i += 1;
    }
    return result;
}

def advance()
{
    pos += 1;
    print(pos);
}

def next()
{
    // Get the current character
    t = index_of(source, pos);

    // Increment the position
    advance();
    return t;
}

// Read the file
source = read_file(".\simple.p");

def parse()
{
    tokens = [];
    // Parse the tokens
    while (pos < size_of(source))
    {
        // Skip whitespace
        c = index_of(source, pos);
        if (c == " ")
        {
            advance();
        }
        else
        {
            r = next();
            append(tokens, r);
            advance();

            is_number = is_numeric(r);
            print(is_number);
        }
    }
    return tokens;
}

// Print the tokens
all_tokens = parse();
print(all_tokens);