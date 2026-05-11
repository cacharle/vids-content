#include "foo.h"

Bar::Bar() {
    /* depends on g_foo already being constructed */
    derived = g_foo.get() * 2;
}

Bar g_bar;
