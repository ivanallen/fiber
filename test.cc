#include "fiber.h"
#include <iostream>

void bar() {
    std::cout << "bar 1" << std::endl;
    Fiber::yield(); // step into foo
    std::cout << "bar 2" << std::endl;
}

void foo() {
    std::cout << "foo 1" << std::endl;
    Fiber::yield(); // step into main
    std::cout << "foo 2" << std::endl;

    auto fiber = std::make_shared<Fiber>(bar);
    fiber->resume(); // step into bar
    std::cout << "foo 3" << std::endl;
    std::cout << "fiber " << fiber->id()
        << " state:" << fiber->state() << std::endl;
}

int main() {
    auto fiber = std::make_shared<Fiber>(foo);
    std::cout << "m1" << std::endl;
    fiber->resume(); // step into foo
    std::cout << "m2" << std::endl;
    fiber->resume(); // step info foo
    std::cout << "fiber " << fiber->id()
        << " state:" << fiber->state() << std::endl;
    return 0;
}
