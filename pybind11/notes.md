# pybind11

Installation -> show webpage

## Basic example of a function

```cpp
int add(int a, int b) {
    return a + b;
}

PYBIND11_MODULE(example, m) {
    m.def("add", &add);
}
```

## Documentation & named arguments

```cpp
    m.doc() = "foo";
    m.def("add", &add, "adds stuff", py::arg("a"), py::arg("b"));
```

## Default arguments

```cpp
int add(int a, int b = 3) {
    m.def("add", &add, py::arg("a"), py::arg("b") = 3);
```

## More complex types

```cpp
std::vector<std::string> keys(const std::map<std::string, int> &m) {
    std::vector<std::string> v;
    for (auto [k, _] : m)
        v.push_back(k);
    return v;
}
```

## Overload

```cpp
double add(double a, double b) {
    return 42 * a + b;
}

    m.def("add", py::overload_cast<int, int>(&add));
    m.def("add", py::overload_cast<double, double>(&add));
```

## Enum

```cpp
enum Color {
    Red,
    Green,
    Blue,
};

    py::native_enum<Color>(m, "Color", "enum.Enum")
        .value("Red", Red)
        .value("Green", Green)
        .value("Blue", Blue)
        .export_values()
        .finalize();
```

> Also works with `enum class`

## Class/struct

```cpp
struct Foo {
    Foo(const std::string &name) : name(name) {}
    std::string scream() const { return name + ": AAAAAAAAAAAAH!"; }
    std::string name;
};

    py::class_<Foo>(m, "Foo")
        .def(py::init<const std::string&>())
        .def("scream", &Foo::scream);
```

### Getter/setter

```cpp
    .def_readonly("name", &Foo::name)
    .def_readwrite("name", &Foo::name)
```


## Exceptions

```cpp
void will_throw() {
    throw std::runtime_error("hello there");
}
```
