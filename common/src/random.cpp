#include <lit/common/random.hpp>

RandomGen::RandomGen(uint64_t seed) { rng.seed(seed); }

uint64_t RandomGen::get() {
    return rng();
}

int64_t RandomGen::get_int(int64_t max) {
    return static_cast<int64_t>(get() >> 1) % (max + 1);
}

int64_t RandomGen::get_int(int64_t min, int64_t max) {
    return static_cast<int64_t>(get() >> 1) % (max - min + 1) + min;
}

double RandomGen::get_double() {
    return static_cast<double>(get()) / static_cast<double>(std::numeric_limits<uint64_t>::max());
}

double RandomGen::get_double(double min, double max) {
    return get_double() * (max - min) + min;
}