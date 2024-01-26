#pragma once
#include <concepts>

namespace chalk
{
    template<typename TActual, typename... TTypePack>
    struct MixChain;

    template<typename TActual, typename TType, typename... TTypePack>
    struct MixChain<TActual, TType, TTypePack...>
        : public MixChain<TActual, TTypePack...>
    {
        using Actual = TActual;
        using Link = MixChain<TActual, TTypePack...>;
    };
    template<typename TActual, typename TType>
    struct MixChain<TActual, TType>
    {
        using Actual = TActual;
    };
}
