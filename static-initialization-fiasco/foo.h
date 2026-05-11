#pragma once

struct Foo {
    int value;
    Foo();
    int get() const;
};

extern Foo g_foo;

struct Bar {
    int derived;
    Bar();
};

extern Bar g_bar;
