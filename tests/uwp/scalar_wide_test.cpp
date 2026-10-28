// ROM-free correctness tests for portable Xbox widescreen vector operations.
#include <cassert>
#include <bit>
#include <cmath>
#include <cstdint>
#include "melonDS/src/ScalarWideVector.h"

int main()
{
    static_assert(sizeof(VF) == 16 && sizeof(VI) == 16);
    VF a{};
    for (int i = 0; i < 4; ++i) a[i] = float(i - 2);
    VI negative = a < 0;
    assert(negative[0] == -1 && negative[1] == -1 && negative[2] == 0 && negative[3] == 0);

    VF masked = (VF)((VI)a & negative);
    assert(std::bit_cast<std::uint32_t>(masked[0]) == std::bit_cast<std::uint32_t>(a[0]));
    assert(std::bit_cast<std::uint32_t>(masked[2]) == 0);

    VI ints = WideNumericConvert<VI>(a);
    assert(ints[0] == -2 && ints[1] == -1 && ints[2] == 0 && ints[3] == 1);

    VI z = (ints + 3) * 2;
    assert(z[0] == 2 && z[1] == 4 && z[2] == 6 && z[3] == 8);
    VI active = (z >= 6) & (z < 9);
    assert(active[0] == 0 && active[1] == 0 && active[2] == -1 && active[3] == -1);
    assert((z >> 1)[3] == 4);

    VF q = 1.0f / (a + 3.0f);
    assert(std::abs(q[0] - 1.0f) < 0.000001f);
    assert(std::abs(q[3] - 0.25f) < 0.000001f);
    return 0;
}
