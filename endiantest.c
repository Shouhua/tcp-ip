/*
    This file is used to test the endian of your cpu, and we also have some other method to do it.
for example:
const int endian = 1;
#define is_bigendian() ((*(char *)&endian) == 0)
#define is_littlendian() ((*(char *)&endian) == 1)
*/
#include <stdio.h>
#include <stdlib.h>
static union 
{
    char c[4];
    unsigned long mylong;
} endian_test = {{ 'l', '?', '?', 'b'}};
#define ENDIANNESS ((char) endian_test.mylong)
 
int main(int argc, char **argv)
{
    char m = (char)ENDIANNESS;
    printf("This machine is in %s mode.\n", &m);
    return 0;
}
