#ifndef IS_LEGAL
#define IS_LEGAL

namespace nest {
template<bool L>
struct IsLegal {};

template<>
struct IsLegal<true> {typedef void* is_legal;};
}

#endif //IS_LEGAL
