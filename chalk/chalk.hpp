#pragma once
#include <concepts>
#include <string>

namespace chalk
{

    struct name {
        std::string_view value;
    };

    template<typename T>
    struct base {
    };

    template<typename T, typename TMem>
    struct mem {
        TMem T::*value;
    };

    template<typename T, typename FRet, typename... FArgs>
    struct func {
        FRet (T::*value)(FArgs...);
    };


    template<typename C>
    concept FuncReflector
        = requires(C r, name n) {
            { r % n } -> std::convertible_to<C>;
        };

    template<typename C>
    concept MemReflector
        = requires(C r, name n) {
            { r % n } -> std::convertible_to<C>;
        };

    template<typename C>
    concept Reflector
        = requires {
            typename C::Reflectee;
        } and requires (C r) {
            { r.reify() } -> std::convertible_to<typename C::Result>;
        } and requires(C r, func<typename C::Reflectee, void> f) {
            { r % f } -> FuncReflector;
        };

    template<typename TReflect, template<typename> class TReflector>
        requires Reflector<TReflector<TReflect>>
    constexpr auto reflect() -> typename TReflector<TReflect>::Result {
        TReflector<TReflect> reflector;
        reflector = TReflect::reflect(reflector);
        return reflector.reify();
    }

    class ReflectBranchNop {
        public:
            // used to ignore things you don't care about
            constexpr auto& operator% (auto) {
                return *this;
            };
    };

    // would be better with deducing this, but for now it's CRTP
    template<typename TActual>
    class ReflectorBase {
        public:
            // used to ignore things you don't care about
            constexpr auto operator% (auto) {
                return ReflectBranchNop {};
            };
    };
}
