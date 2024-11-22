#include <fpng.h>

namespace {
__attribute__((constructor)) void init() { fpng::fpng_init(); }
} // namespace
