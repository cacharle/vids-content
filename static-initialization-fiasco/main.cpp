#include <cstdio>
#include "foo.h"

int main() {
    std::printf("foo=%d bar=%d\n", g_foo.value, g_bar.derived);
    return 0;
}
