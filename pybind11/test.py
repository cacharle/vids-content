import example

assert example.add(2, 3) == 5
assert example.add(a=10, b=-4) == 6
assert example.primes_below(20) == [2, 3, 5, 7, 11, 13, 17, 19]

g = example.Greeter("world")
assert g.greet() == "hello, world"
g.name = "pybind11"
assert g.greet() == "hello, pybind11"

print("ok:", example.__doc__)
print(" ", example.add(2, 3))
print(" ", example.primes_below(30))
print(" ", g.greet())
