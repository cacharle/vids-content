#include <boost/decimal.hpp>
#include <iostream>

int main()
{
    using boost::decimal::decimal64_t;

    decimal64_t a {1, -1}; // 0.1
    decimal64_t b {2, -1}; // 0.2
    decimal64_t c = a + b;

    std::cout << "0.1 + 0.2 = " << c << '\n';
    std::cout << "exactly 0.3? " << std::boolalpha << (c == decimal64_t{3, -1}) << '\n';

    return 0;
}
