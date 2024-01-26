#pragma once
#include <concepts>
#include <string>

namespace chalk
{
    namespace tags
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
    }

    template<typename C>
    concept FuncOutliner
        = requires(C r, tags::name n) {
            { r % n } -> std::convertible_to<C>;
        };

    template<typename C>
    concept MemOutliner
        = requires(C r, tags::name n) {
            { r % n } -> std::convertible_to<C>;
        };

    template<typename C>
    concept Outliner
        = requires {
            typename C::Linee;
            typename C::Result;
        } and requires (C r) {
            { r.reify() } -> std::convertible_to<typename C::Result>;
        } and requires(C r, tags::func<typename C::Linee, void> f) {
            { r % f } -> FuncOutliner;
        };

    template<typename TOutRef, Outliner TOutliner = std::remove_cvref_t<TOutRef>>
    using linee = typename TOutliner::Linee;



    template<typename TTarget, class TOutliner>
        requires Outliner<TOutliner>
    constexpr void outline(TOutliner* outliner) {
        if constexpr ( requires { TTarget::outline(*outliner); } ) {
            TTarget::outline(*outliner);
        } else {
            static_assert(requires { TTarget::outline(*outliner); }, "TTarget has no valid outliner.");
        }
    }

    template<typename TTarget, template<typename> class TTOutliner>
        requires Outliner<TTOutliner<TTarget>>
    constexpr auto outline() -> typename TTOutliner<TTarget>::Result {
        TTOutliner<TTarget> outliner;
        outline<TTarget>(&outliner);
        return outliner.reify();
    }


    class OutlineBranchNop {
        public:
            // used to ignore things you don't care about
            constexpr auto& operator% (auto) {
                return *this;
            };
    };

    // would be better with deducing this, but for now it's CRTP
    template<typename TActual>
    class OutlinerBase {
        public:
            // used to ignore things you don't care about
            constexpr auto operator% (auto) {
                return OutlineBranchNop {};
            };
    };
}
