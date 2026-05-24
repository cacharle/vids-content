#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/native_enum.h>

#include <string>
#include <vector>

namespace py = pybind11;

int add(int a, int b = 3) {
    return a + b;
}

double add(double a, double b) {
    return 42 * a + b;
}

void will_throw() {
    throw std::runtime_error("hello there");
}

std::vector<std::string> keys(const std::map<std::string, int> &m) {
    std::vector<std::string> v;
    for (auto [k, _] : m)
        v.push_back(k);
    return v;
}

enum Color {
    Red,
    Green,
    Blue,
};

struct Foo {
    Foo(const std::string &name) : name(name) {}
    std::string scream() const { return name + ": AAAAAAAAAAAAH!"; }
    std::string name;
};

PYBIND11_MODULE(example, m) {
    m.def("add", py::overload_cast<int, int>(&add), py::arg("a"), py::arg("b") = 3);
    m.def("add", py::overload_cast<double, double>(&add));
    m.def("keys", &keys);
    m.def("will_throw", &will_throw);

    py::native_enum<Color>(m, "Color", "enum.Enum")
        .value("Red", Red)
        .value("Green", Green)
        .value("Blue", Blue)
        .export_values()
        .finalize();

    py::class_<Foo>(m, "Foo")
        .def(py::init<const std::string&>())
        .def_readwrite("name", &Foo::name)
        .def("scream", &Foo::scream);
}
