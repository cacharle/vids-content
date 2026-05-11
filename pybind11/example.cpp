#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <string>
#include <vector>

namespace py = pybind11;

int add(int a, int b) {
    return a + b;
}

std::vector<int> primes_below(int n) {
    std::vector<int> out;
    for (int i = 2; i < n; ++i) {
        bool prime = true;
        for (int j = 2; j * j <= i; ++j) {
            if (i % j == 0) { prime = false; break; }
        }
        if (prime) out.push_back(i);
    }
    return out;
}

struct Greeter {
    std::string name;
    explicit Greeter(std::string n) : name(std::move(n)) {}
    std::string greet() const { return "hello, " + name; }
};

PYBIND11_MODULE(example, m) {
    m.doc() = "tiny pybind11 demo";

    m.def("add", &add, "add two ints", py::arg("a"), py::arg("b"));
    m.def("primes_below", &primes_below, "primes strictly below n");

    py::class_<Greeter>(m, "Greeter")
        .def(py::init<std::string>(), py::arg("name"))
        .def("greet", &Greeter::greet)
        .def_readwrite("name", &Greeter::name);
}
