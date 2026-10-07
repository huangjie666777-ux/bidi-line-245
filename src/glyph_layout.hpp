#pragma once

#include <string>

#include "../include/bidi_line245/bidi_line245.hpp"
#include "bidi_analysis.hpp"
#include "unicode_text.hpp"

namespace bidi_line245 {

struct RunLayout {
    std::size_t run_index = 0;
    std::vector<Glyph> glyphs;
    std::vector<HorizontalRange> cluster_boxes;
    double width = 0.0;
};

class FontShaper {
public:
    explicit FontShaper(const std::string& font_path);
    ~FontShaper();

    FontShaper(const FontShaper&) = delete;
    FontShaper& operator=(const FontShaper&) = delete;

    LineLayout layout(const DecodedLine& decoded, const BidiAnalysis& bidi, double font_size_px) const;

private:
    RunLayout shape_run(const DecodedLine& decoded, const BidiAnalysis& bidi,
                        const BidiRun& run, double scale) const;
    hb_font_t* font_ = nullptr;
};

}  // namespace bidi_line245
