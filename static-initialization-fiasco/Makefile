CXXFLAGS = -O0 -Wall

# Link order determines global constructor order across TUs.
# good: foo.o first -> Foo() runs before Bar() -> bar.derived == 42
# bad:  bar.o first -> Bar() reads g_foo.value before Foo() ran -> garbage

good: main.cpp foo.cpp bar.cpp foo.h
	$(CXX) $(CXXFLAGS) -o $@ foo.cpp bar.cpp main.cpp

bad: main.cpp foo.cpp bar.cpp foo.h
	$(CXX) $(CXXFLAGS) -o $@ bar.cpp foo.cpp main.cpp

clean:
	rm -f good bad

.PHONY: clean
