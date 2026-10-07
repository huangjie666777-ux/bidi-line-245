#include <bidi_line245/bidi_line245.hpp>

#include <iomanip>
#include <iostream>
#include <string>

int main() {
    namespace b = bidi_line245;
    b::LineShaper shaper("fonts/DejaVuSans.ttf");
    const std::string text =
        u8"Latin \xD7\xA2\xD7\x91\xD7\xA8\xD7\x99\xD7\xAA"
        u8" \xD8\xA7\xD9\x84\xD8\xB9\xD8\xB1\xD8\xA8 123";
    b::LineLayout line = shaper.shape_line(text, 28.0, b::BaseDirection::Auto);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "glyphs: " << line.glyphs.size() << ", width: " << line.width << "\n";
    for (const b::Glyph& g : line.glyphs) {
        std::cout << "id=" << g.glyph_id
                  << " bytes=[" << g.cluster_begin << "," << g.cluster_end << ")"
                  << " baseline=(" << g.baseline_x << "," << g.baseline_y << ")"
                  << " advance=" << g.x_advance
                  << " offset=(" << g.x_offset << "," << g.y_offset << ")"
                  << " level=" << static_cast<int>(g.bidi_level) << "\n";
    }
}
