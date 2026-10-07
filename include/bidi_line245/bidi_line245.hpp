#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace bidi_line245 {

enum class BaseDirection {
    Auto,
    Ltr,
    Rtl
};

struct Glyph {
    uint32_t glyph_id = 0;
    std::size_t cluster_begin = 0;
    std::size_t cluster_end = 0;
    double baseline_x = 0.0;
    double baseline_y = 0.0;
    double x_advance = 0.0;
    double y_advance = 0.0;
    double x_offset = 0.0;
    double y_offset = 0.0;
    uint8_t bidi_level = 0;
    bool rtl = false;
};

struct HorizontalRange {
    double begin = 0.0;
    double end = 0.0;
};

struct LineLayout {
    std::vector<Glyph> glyphs;
    std::vector<HorizontalRange> cluster_boxes;
    double width = 0.0;
    double base_level = 0;
};

class LineShaper {
public:
    explicit LineShaper(std::string font_path = "fonts/DejaVuSans.ttf");
    ~LineShaper();

    LineShaper(const LineShaper&) = delete;
    LineShaper& operator=(const LineShaper&) = delete;

    LineLayout shape_line(const std::string& utf8_text,
                          double font_size_px,
                          BaseDirection base_direction = BaseDirection::Auto) const;

    std::vector<HorizontalRange> selection_boxes(const LineLayout& layout,
                                                 const std::string& utf8_text,
                                                 std::size_t begin,
                                                 std::size_t end) const;

private:
    std::string font_path_;
};

std::vector<HorizontalRange> selection_boxes(const LineLayout& layout,
                                             const std::string& utf8_text,
                                             std::size_t begin,
                                             std::size_t end);

}  // namespace bidi_line245
