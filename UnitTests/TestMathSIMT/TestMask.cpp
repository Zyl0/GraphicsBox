#include <catch2/catch_all.hpp>
#include <MathSimt/Types.h>
#include <type_traits>

using namespace Math::Simt;

TEMPLATE_TEST_CASE("Mask members and operators", "[Mask]", 
    (std::integral_constant<size_t, 2>),
    (std::integral_constant<size_t, 4>),
    (std::integral_constant<size_t, 8>),
    (std::integral_constant<size_t, 16>),
    (std::integral_constant<size_t, 32>)
    )
{
    constexpr size_t ThreadCount = TestType::value;
    using MaskType = Mask<ThreadCount>;
    using Type = typename MaskType::Type;

    SECTION("Types and Constants")
    {
        STATIC_REQUIRE(MaskType::kThreadCount == ThreadCount);
        STATIC_REQUIRE(MaskType::Size() == ThreadCount);

        if constexpr (ThreadCount <= 8)
        {
            STATIC_REQUIRE(std::is_same_v<Type, uint8_t>);
        }
        else if constexpr (ThreadCount <= 16)
        {
            STATIC_REQUIRE(std::is_same_v<Type, uint16_t>);
        }
        else if constexpr (ThreadCount <= 32)
        {
            STATIC_REQUIRE(std::is_same_v<Type, uint32_t>);
        }
        else
        {
            STATIC_REQUIRE(std::is_same_v<Type, uint64_t>);
        }
    }

    SECTION("Constructors and defaults")
    {
        MaskType m1;
        REQUIRE(m1.bits == 0);
        REQUIRE(m1.None() == true);
        REQUIRE(m1.Any() == false);
        REQUIRE(m1.All() == false);
        REQUIRE(m1.Count() == 0);

        MaskType m2(3); // binary 011
        REQUIRE(m2.bits == 3);
        REQUIRE(m2.None() == false);
        REQUIRE(m2.Any() == true);
        if constexpr (ThreadCount == 2)
        {
            REQUIRE(m2.All() == true);
            REQUIRE(m2.Count() == 2);
        }
        else
        {
            REQUIRE(m2.All() == false);
            REQUIRE(m2.Count() == 2);
        }
    }

    SECTION("Proxy and operator[]")
    {
        MaskType m;
        m[0] = true;
        REQUIRE(m.bits == 1);
        m[1] = true;
        REQUIRE(m.bits == 3);
        m[0] = false;
        REQUIRE(m.bits == 2);
        
        const MaskType cm(2); // binary 010
        REQUIRE(cm[0] == false);
        REQUIRE(cm[1] == true);
    }

    SECTION("Bitwise operators")
    {
        MaskType m1(3); // 011
        MaskType m2(6); // 110

        MaskType mand = m1 & m2;
        REQUIRE(mand.bits == 2); // 010

        MaskType mor = m1 | m2;
        REQUIRE(mor.bits == 7); // 111

        MaskType mxor = m1 ^ m2;
        REQUIRE(mxor.bits == 5); // 101

        MaskType minv = ~m1;
        Type expected_minv_bits = (~Type(3)) & MaskType::FullMask();
        REQUIRE(minv.bits == expected_minv_bits);

        MaskType mnot = !m1;
        REQUIRE(mnot.bits == expected_minv_bits);
    }
    
    SECTION("Compound assignment operators")
    {
        MaskType m(3); // 011
        
        m &= MaskType(2); // 010
        REQUIRE(m.bits == 2);
        
        m |= MaskType(4); // 110
        REQUIRE(m.bits == 6);
        
        m ^= MaskType(2); // 100
        REQUIRE(m.bits == 4);
    }

    SECTION("All, Any, None, Count")
    {
        MaskType m_full(MaskType::FullMask());
        REQUIRE(m_full.All() == true);
        REQUIRE(m_full.Any() == true);
        REQUIRE(m_full.None() == false);
        REQUIRE(m_full.Count() == ThreadCount);

        MaskType m_empty(0);
        REQUIRE(m_empty.All() == false);
        REQUIRE(m_empty.Any() == false);
        REQUIRE(m_empty.None() == true);
        REQUIRE(m_empty.Count() == 0);

        MaskType m_partial(1);
        REQUIRE(m_partial.All() == false);
        REQUIRE(m_partial.Any() == true);
        REQUIRE(m_partial.None() == false);
        REQUIRE(m_partial.Count() == 1);
    }
}
