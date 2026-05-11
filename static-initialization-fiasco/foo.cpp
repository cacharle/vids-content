#include "foo.h"

Foo::Foo() {
    value = 21;
}

int Foo::get() const {
    return value;
}

Foo g_foo;
