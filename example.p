my_string = "This is a long and complex string meant to be parsed.";
a = my_string[5];

my_array = [1,[1,2,3],"three",4,5];
b = my_array[3];

b += 1 + 2 * 3;

new_string = "";

max_count = 40;
while (b < max_count)
{
    b += 1;
    new_string += my_string[b];
}
