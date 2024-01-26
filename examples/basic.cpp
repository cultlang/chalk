#include <iostream>

#include "chalk/chalk.hpp"
#include "chalk/reflection.hpp"

namespace test
{
    class TestA
    {
        int hi;

        void hello(int) {}

        int lets_go(int a) { return a; }
        int lets_go(float a) { return a; }

        public:
            static constexpr void outline(chalk::Outliner auto& out) {
                using namespace chalk;
                using Self = linee<decltype(out)>;

                out % name("test::TestA");

                out % mem(&Self::hi)
                    % name("hi");

                out % func(&Self::hello)
                    % name("hello");

                // you could also chain funcs into an overload set
                out % func<Self, int, int>(&Self::lets_go)
                    % name("lets_go_i");
                out % func<Self, int, float>(&Self::lets_go)
                    % name("lets_go_f");
            }
    };

    class TestB
        : public TestA
    {
        float myValue;

        public:
            static constexpr void outline(chalk::Outliner auto& out) {
                using namespace chalk;
                using Self = linee<decltype(out)>;

                out % base<TestA>()
                    % name("test::TestB");

                out % mem(&Self::myValue)
                    % name("myValue");
            }
    };
}

int main()
{
    // you would want some sort of repository here so that base<TestA> could linkup to the reflection of TestA
    std::vector<std::shared_ptr<chalk::reflection::SimpleReflection>> reflections;
    reflections.push_back(chalk::outline<test::TestA, chalk::reflection::ReflectionOutliner>());
    reflections.push_back(chalk::outline<test::TestB, chalk::reflection::ReflectionOutliner>());

    chalk::reflection::Repository repo;

    for (auto r : reflections) {
        std::cout << r->name << " " << r->type_name << "\n";
        for (auto e : r->entries) {
            std::cout << "  " << e.kind << " " << e.type_name << " " << e.name << " " << e.ptr << "\n";
        }
    }
}
