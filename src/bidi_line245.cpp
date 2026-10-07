#include "../include/bidi_line245/bidi_line245.hpp"

#include <stdexcept>

#include "bidi_analysis.hpp"
#include "glyph_layout.hpp"
#include "selection.hpp"
#include "unicode_text.hpp"

namespace bidi_line245 {

LineShaper::LineShaper(std::string font_path) : font_path_(std::move(font_path)) {}
LineShaper::~LineShaper() = default;

LineLayout LineShaper::shape_line(const std::string& utf8_text,
                                  double font_size_px,
                                  BaseDirection base_direction) const {
    DecodedLine decoded = decode_utf8_line(utf8_text);
    BidiAnalysis bidi = analyze_bidi(decoded, base_direction);
    FontShaper font(font_path_);
    return font.layout(decoded, bidi, font_size_px);
}

std::vector<HorizontalRange> LineShaper::selection_boxes(const LineLayout& layout,
                                                         const std::string& utf8_text,
                                                         std::size_t begin,
                                                         std::size_t end) const {
    validate_byte_range(utf8_text, begin, end);
    if (begin == end) return {};
    DecodedLine decoded = decode_utf8_line(utf8_text);
    return map_selection(decoded, layout, begin, end);
}

std::vector<HorizontalRange> selection_boxes(const LineLayout& layout,
                                             const std::string& utf8_text,
                                             std::size_t begin,
                                             std::size_t end) {
    LineShaper shaper;
    return shaper.selection_boxes(layout, utf8_text, begin, end);
}

}  // namespace bidi_line245
