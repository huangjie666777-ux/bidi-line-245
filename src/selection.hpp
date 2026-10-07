#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../include/bidi_line245/bidi_line245.hpp"
#include "unicode_text.hpp"

namespace bidi_line245 {

std::vector<HorizontalRange> map_selection(const DecodedLine& decoded,
                                           const LineLayout& layout,
                                           std::size_t begin,
                                           std::size_t end);

}  // namespace bidi_line245
