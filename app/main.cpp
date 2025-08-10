#include "lib/Derived.hpp"
#include <iostream>

int main()
{
    Lib::Derived d(3);
    std::cout << "Result = " << d.add(1, 2) << '\n';
}