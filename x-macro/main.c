#include <stdio.h>

#define COLORS \
    X(Red,   0xff0000)    \
    X(Green, 0x00ff00)   \
    X(Blue,  0x0000ff)

#define X(c, _) Color##c,
enum Color {
    COLORS
};
#undef X

void print_color(enum Color color)
{
    switch (color) {
#define X(c, _) case Color##c: printf("%s\n", #c); break;
COLORS
#undef X
    }
}

int color_hex(enum Color color)
{
    switch (color) {
#define X(c, v) case Color##c: return v;
COLORS
#undef X
    }
}


int main()
{
    enum Color c = ColorBlue;
    print_color(c);
    printf("value: %06x\n", color_hex(c));

    return 0;
}
