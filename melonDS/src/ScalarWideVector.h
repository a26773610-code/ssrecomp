// Portable four-lane fallback for the SoulSilver wide 3D rasterizer.
// MSVC cannot compile GCC's vector_size / __builtin_convertvector extensions.
// Mask comparisons preserve GCC semantics: true = all bits set (-1).
// This is a correctness-first scalar implementation, to benchmark on Xbox.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>

template<typename T>
struct ScalarWideVec {
    T v[4]{};
    ScalarWideVec() = default;
    template<typename U, typename=std::enable_if_t<std::is_arithmetic_v<U>>>
    ScalarWideVec(U scalar) { for (int i=0;i<4;i++) v[i] = static_cast<T>(scalar); }
    template<typename U>
    explicit ScalarWideVec(const ScalarWideVec<U>& x) {
        static_assert(sizeof(T) == sizeof(U), "bit reinterpret needs equal lane size");
        static_assert(sizeof(ScalarWideVec<T>) == sizeof(ScalarWideVec<U>));
        std::memcpy(v, x.v, sizeof(v));
    }
    T& operator[](int i) { return v[i]; }
    const T& operator[](int i) const { return v[i]; }
    ScalarWideVec& operator+=(const ScalarWideVec& x) { for(int i=0;i<4;i++) v[i] += x[i]; return *this; }
    ScalarWideVec& operator-=(const ScalarWideVec& x) { for(int i=0;i<4;i++) v[i] -= x[i]; return *this; }
    ScalarWideVec& operator*=(const ScalarWideVec& x) { for(int i=0;i<4;i++) v[i] *= x[i]; return *this; }
    ScalarWideVec& operator/=(const ScalarWideVec& x) { for(int i=0;i<4;i++) v[i] /= x[i]; return *this; }
    ScalarWideVec& operator&=(const ScalarWideVec& x) { static_assert(std::is_integral_v<T>); for(int i=0;i<4;i++) v[i] &= x[i]; return *this; }
    ScalarWideVec& operator|=(const ScalarWideVec& x) { static_assert(std::is_integral_v<T>); for(int i=0;i<4;i++) v[i] |= x[i]; return *this; }
};
using VF = ScalarWideVec<float>;
using VI = ScalarWideVec<std::int32_t>;

#define SSR_WIDE_BIN(OP) \
template<typename T> inline ScalarWideVec<T> operator OP (ScalarWideVec<T> a, const ScalarWideVec<T>& b) { for(int i=0;i<4;i++) a[i] = static_cast<T>(a[i] OP b[i]); return a; } \
template<typename T, typename U, typename=std::enable_if_t<std::is_arithmetic_v<U>>> inline ScalarWideVec<T> operator OP (ScalarWideVec<T> a, U b) { for(int i=0;i<4;i++) a[i] = static_cast<T>(a[i] OP static_cast<T>(b)); return a; } \
template<typename T, typename U, typename=std::enable_if_t<std::is_arithmetic_v<U>>> inline ScalarWideVec<T> operator OP (U a, ScalarWideVec<T> b) { for(int i=0;i<4;i++) b[i] = static_cast<T>(static_cast<T>(a) OP b[i]); return b; }
SSR_WIDE_BIN(+)
SSR_WIDE_BIN(-)
SSR_WIDE_BIN(*)
SSR_WIDE_BIN(/)
#undef SSR_WIDE_BIN

#define SSR_WIDE_BITS(OP) \
inline VI operator OP (VI a, VI b) { for(int i=0;i<4;i++) a[i] = a[i] OP b[i]; return a; } \
inline VI operator OP (VI a, int b) { for(int i=0;i<4;i++) a[i] = a[i] OP b; return a; } \
inline VI operator OP (int a, VI b) { for(int i=0;i<4;i++) b[i] = a OP b[i]; return b; }
SSR_WIDE_BITS(&)
SSR_WIDE_BITS(|)
SSR_WIDE_BITS(^)
SSR_WIDE_BITS(>>)
SSR_WIDE_BITS(<<)
#undef SSR_WIDE_BITS
inline VI operator~(VI a) { for(int i=0;i<4;i++) a[i] = ~a[i]; return a; }

#define SSR_WIDE_CMP(OP) \
template<typename T> inline VI operator OP (const ScalarWideVec<T>& a, const ScalarWideVec<T>& b) { VI out; for(int i=0;i<4;i++) out[i] = (a[i] OP b[i]) ? -1 : 0; return out; } \
template<typename T, typename U, typename=std::enable_if_t<std::is_arithmetic_v<U>>> inline VI operator OP (const ScalarWideVec<T>& a, U b) { VI out; for(int i=0;i<4;i++) out[i] = (a[i] OP static_cast<T>(b)) ? -1 : 0; return out; } \
template<typename T, typename U, typename=std::enable_if_t<std::is_arithmetic_v<U>>> inline VI operator OP (U a, const ScalarWideVec<T>& b) { VI out; for(int i=0;i<4;i++) out[i] = (static_cast<T>(a) OP b[i]) ? -1 : 0; return out; }
SSR_WIDE_CMP(<)
SSR_WIDE_CMP(<=)
SSR_WIDE_CMP(>)
SSR_WIDE_CMP(>=)
SSR_WIDE_CMP(==)
SSR_WIDE_CMP(!=)
#undef SSR_WIDE_CMP

template<typename To, typename From>
inline To WideNumericConvert(const From& source) {
    To out;
    for(int i=0;i<4;i++) out[i] = static_cast<std::remove_reference_t<decltype(out[i])>>(source[i]);
    return out;
}
