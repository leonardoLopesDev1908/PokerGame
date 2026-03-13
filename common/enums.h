#pragma once

enum class Stage : uint8_t
{
        PreFlop,
        Flop,
        Turn,
        River,
        CardChecking,
        Ending
};

enum class Hands : uint8_t
{
        HighCard,
        Pair,
        TwoPair,
        ThreeOfaKind,
        Straight,
        Flush,
        FullHouse,
        FourOfaKind,
        StraighFlush,
        RoyalFlush
};

