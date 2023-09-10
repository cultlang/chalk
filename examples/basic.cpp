#include <iostream>

#include "chalk/chalk.hpp"
#include "chalk/runtime.hpp"

namespace test
{
    class TestA
    {
        int hi;

        void hello(int) {}

        int lets_go(int a) { return a; }
        int lets_go(float a) { return a; }

        public:
            static constexpr auto reflect(chalk::Reflector auto ref) {
                // this allows for some interesting tricks when working around member functions
                using Self = decltype(ref)::Reflectee;
                using namespace chalk;

                ref % name("test::TestA");

                ref % mem(&Self::hi)
                    % name("hi");

                ref % func(&Self::hello)
                    % name("hello");

                // you could also chain funcs into an overload set
                ref % func<Self, int, int>(&Self::lets_go)
                    % name("lets_go_i");
                ref % func<Self, int, float>(&Self::lets_go)
                    % name("lets_go_f");

                return ref;
            }
    };

    class TestB
        : public TestA
    {
        float myValue;

        public:
            static constexpr auto reflect(chalk::Reflector auto ref) {
                using Self = decltype(ref)::Reflectee;
                using namespace chalk;

                ref % base<TestA>()
                    % name("test::TestB");

                ref % mem(&Self::myValue)
                    % name("myValue");

                return ref;
            }
    };
}

int main()
{
    // you would want some sort of repository here so that base<TestA> could linkup to the reflection of TestA
    std::vector<std::shared_ptr<chalk::SimpleReflection>> reflections;
    reflections.push_back(chalk::reflect<test::TestA, chalk::RuntimeReflector>());
    reflections.push_back(chalk::reflect<test::TestB, chalk::RuntimeReflector>());

    for (auto r : reflections) {
        std::cout << r->name << " " << r->type_name << "\n";
        for (auto e : r->entries) {
            std::cout << "  " << e.kind << " " << e.type_name << " " << e.name << " " << e.ptr << "\n";
        }
    }
}
