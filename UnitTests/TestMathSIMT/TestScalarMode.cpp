#include <catch2/catch_all.hpp>
#include <MathSimt/Types.h>
#include <type_traits>

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Scalar constructors and methods", "[Scalar]",
    ((typename T, size_t N), T, N),
    (uint8_t, 1), (int32_t, 1), (uint32_t, 1), (float, 1),
    (uint8_t, 2), (int32_t, 2), (uint32_t, 2), (float, 2),
    (uint8_t, 4), (int32_t, 4), (uint32_t, 4), (float, 4),
    (uint8_t, 8), (int32_t, 8), (uint32_t, 8), (float, 8),
    (uint8_t, 16), (int32_t, 16), (uint32_t, 16), (float, 16),
    (uint8_t, 32), (int32_t, 32), (uint32_t, 32), (float, 32))
{
    using ScalarType = Scalar<T, N>;
    constexpr size_t ThreadCount = N;

    SECTION("Constants")
    {
        REQUIRE(ScalarType::kThreadCount == N);
        REQUIRE(ScalarType::kAlignment == sizeof(T) * N);
        REQUIRE(ScalarType::Size() == N);
    }

    SECTION("Constructors and Assignment")
    {
        ScalarType s1;
        s1.Zero();
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(s1[i] == T(0));
        }

        ScalarType s2(T(5));
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(s2[i] == T(5));
        }

        if constexpr (N >= 2) {
            ScalarType s3{T(1), T(2)};
            REQUIRE(s3[0] == T(1));
            REQUIRE(s3[1] == T(2));
            for (size_t i = 2; i < N; ++i) REQUIRE(s3[i] == T(0));
            
            ScalarType s4;
            s4 = {T(3), T(4)};
            REQUIRE(s4[0] == T(3));
            REQUIRE(s4[1] == T(4));
        }

        ScalarType s_copy(s2);
        for (size_t i = 0; i < N; ++i) REQUIRE(s_copy[i] == T(5));

        ScalarType s_assign;
        s_assign = T(7);
        for (size_t i = 0; i < N; ++i) REQUIRE(s_assign[i] == T(7));
    }

    SECTION("Arithmetic operators")
    {
        ScalarType a(T(6));
        ScalarType b(T(2));

        ScalarType c = a + b;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(8));

        c = a - b;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(4));

        c = a * b;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(12));

        c = a / b;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(3));

        c = a + T(2);
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(8));
        
        c = T(2) + a;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(8));

        c = a - T(2);
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(4));
        
        c = T(10) - b;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(8));

        c = a * T(2);
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(12));
        
        c = T(2) * a;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(12));

        c = a / T(2);
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(3));
        
        c = T(12) / b;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(6));

        ScalarType d(T(5));
        d += a;
        for (size_t i = 0; i < N; ++i) REQUIRE(d[i] == T(11));

        d -= b;
        for (size_t i = 0; i < N; ++i) REQUIRE(d[i] == T(9));

        d *= ScalarType(T(2));
        for (size_t i = 0; i < N; ++i) REQUIRE(d[i] == T(18));

        d /= ScalarType(T(3));
        for (size_t i = 0; i < N; ++i) REQUIRE(d[i] == T(6));

        d += T(2);
        for (size_t i = 0; i < N; ++i) REQUIRE(d[i] == T(8));

        d -= T(3);
        for (size_t i = 0; i < N; ++i) REQUIRE(d[i] == T(5));

        d *= T(4);
        for (size_t i = 0; i < N; ++i) REQUIRE(d[i] == T(20));

        d /= T(5);
        for (size_t i = 0; i < N; ++i) REQUIRE(d[i] == T(4));
        
        c = -a;
        for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == (T)(-T(6)));
    }

    SECTION("Comparison operators")
    {
        ScalarType a, b, c;
        for (size_t i = 0; i < N; ++i)
        {
            if (i % 3 == 0) {
                a[i] = T(2); b[i] = T(5); c[i] = T(2); // a < b, a == c
            } else if (i % 3 == 1) {
                a[i] = T(5); b[i] = T(2); c[i] = T(5); // a > b, a == c
            } else {
                a[i] = T(5); b[i] = T(5); c[i] = T(5); // a == b, a == c
            }
        }

        Mask<N> eq = (a == b);
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(eq[i] == (i % 3 == 2));
        }

        Mask<N> neq = (a != b);
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(neq[i] == (i % 3 != 2));
        }

        Mask<N> lt = (a < b);
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(lt[i] == (i % 3 == 0));
        }

        Mask<N> le = (a <= b);
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(le[i] == (i % 3 == 0 || i % 3 == 2));
        }

        Mask<N> gt = (a > b);
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(gt[i] == (i % 3 == 1));
        }

        Mask<N> ge = (a >= b);
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(ge[i] == (i % 3 == 1 || i % 3 == 2));
        }
        
        Mask<N> eq_c = (a == c);
        for (size_t i = 0; i < N; ++i) {
            REQUIRE(eq_c[i] == true);
        }
    }

    if constexpr (std::is_integral_v<T>)
    {
        SECTION("Integral Bitwise Operators")
        {
            ScalarType a(T(6)); // 110
            ScalarType b(T(3)); // 011

            ScalarType c = a & b;
            for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(2));

            c = a | b;
            for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(7));

            c = a ^ b;
            for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(5));

            c = ~a;
            for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == (T)(~T(6)));

            c = a << 1;
            for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(12));

            c = a >> 1;
            for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(3));
            
            c = a % ScalarType(T(4));
            for (size_t i = 0; i < N; ++i) REQUIRE(c[i] == T(2));
        }
    }

    SECTION("Load and Store")
    {
        alignas(ScalarType::kAlignment) T buffer[N * 2];
        for (size_t i = 0; i < N * 2; ++i) buffer[i] = T(i + 1);

        ScalarType s = ScalarType::Load(buffer);
        for (size_t i = 0; i < N; ++i) REQUIRE(s[i] == T(i + 1));

        ScalarType s_aligned = ScalarType::AlignedLoad(buffer + N);
        for (size_t i = 0; i < N; ++i) REQUIRE(s_aligned[i] == T(i + N + 1));

        Mask<N> m;
        for (size_t i = 0; i < N; ++i) m[i] = (i % 2 == 0);
        
        ScalarType s_mask = ScalarType::Load(buffer, m);
        for (size_t i = 0; i < N; ++i) {
            if (i % 2 == 0) REQUIRE(s_mask[i] == T(i + 1));
            else REQUIRE(s_mask[i] == T(0));
        }

        alignas(ScalarType::kAlignment) T out_buffer[N];
        for (size_t i = 0; i < N; ++i) out_buffer[i] = T(0);
        
        s.Store(out_buffer);
        for (size_t i = 0; i < N; ++i) REQUIRE(out_buffer[i] == T(i + 1));

        for (size_t i = 0; i < N; ++i) out_buffer[i] = T(0);
        s.AlignedStore(out_buffer, m);
        for (size_t i = 0; i < N; ++i) {
            if (i % 2 == 0) REQUIRE(out_buffer[i] == T(i + 1));
            else REQUIRE(out_buffer[i] == T(0));
        }
    }

    SECTION("Global Functions")
    {
        ScalarType a(T(1));
        ScalarType b(T(2));
        Mask<N> mask;
        for (size_t i = 0; i < N; ++i) mask[i] = (i % 2 == 0);

        ScalarType sel = Select(a, b, mask);
        for (size_t i = 0; i < N; ++i) {
            if (i % 2 == 0) REQUIRE(sel[i] == T(1));
            else REQUIRE(sel[i] == T(2));
        }

        if constexpr (N >= 2) {
            typename ScalarType::IndexerType indices;
            for (size_t i = 0; i < N; ++i) indices[i] = static_cast<int>(N - 1 - i);

            ScalarType var_a;
            for (size_t i = 0; i < N; ++i) var_a[i] = T(i);
            
            ScalarType p = Permute(var_a, indices);
            for (size_t i = 0; i < N; ++i) REQUIRE(p[i] == T(N - 1 - i));
            
            alignas(ScalarType::kAlignment) T buffer[N];
            for (size_t i = 0; i < N; ++i) buffer[i] = T(i * 10);

            // TODO find a way to get the automatic type from scalar type
            ScalarType g = Gather<typename  ScalarType::Type, ScalarType::kThreadCount>(buffer, indices);
            for (size_t i = 0; i < N; ++i) REQUIRE(g[i] == T((N - 1 - i) * 10));

            for (size_t i = 0; i < N; ++i) buffer[i] = T(0);
            Scatter(p, buffer, indices);
            for (size_t i = 0; i < N; ++i) {
                REQUIRE(buffer[i] == T(i));
            }

            ScalarType rot = Rotate(var_a, 1);
            for (size_t i = 0; i < N; ++i) {
                REQUIRE(rot[i] == T((i + 1) % N));
            }

            ScalarType shf = Shift(var_a, 1);
            for (int i = 0; i < N; ++i) {
                int expected = std::clamp(i + 1, 0, static_cast<int>(N));
                if (expected < N) {
                    REQUIRE(shf[i] == T(expected));
                }
            }

            ScalarType pk = Pack(var_a, mask);
            size_t p_idx = 0;
            for (size_t i = 0; i < N; ++i) {
                if (mask[i]) {
                    REQUIRE(pk[p_idx++] == T(i));
                }
            }

            ScalarType unpk = UnPack(pk, mask);
            p_idx = 0;
            for (size_t i = 0; i < N; ++i) {
                if (mask[i]) {
                    REQUIRE(unpk[i] == pk[p_idx++]);
                } else {
                    REQUIRE(unpk[i] == T(0));
                }
            }

            ScalarType splt = Split(var_a, mask);
            // Splitting separates valid vs invalid elements
            // first packed with mask, then packed with !mask
            size_t valid_cnt = 0;
            for (size_t i = 0; i < N; ++i) if (mask[i]) valid_cnt++;
            size_t v_idx = 0, nv_idx = valid_cnt;
            for (size_t i = 0; i < N; ++i) {
                if (mask[i]) {
                    REQUIRE(splt[v_idx++] == T(i));
                } else {
                    REQUIRE(splt[nv_idx++] == T(i));
                }
            }
        }
    }
}
