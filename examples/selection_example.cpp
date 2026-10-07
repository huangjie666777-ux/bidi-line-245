#include <bidi_line245/bidi_line245.hpp>

#include <iomanip>
#include <iostream>
#include <string>

int main() {
    namespace b = bidi_line245;
    b::LineShaper shaper("fonts/DejaVuSans.ttf");
    const std::string text = u8"A \xD8\xB9 B";
    b::LineLayout line = shaper.shape_line(text, 32.0, b::BaseDirection::Ltr);

    std::size_t first_begin = 0;
    std::size_t first_end = 1;
    std::size_t last_begin = text.size() - 1;
    std::size_t last_end = text.size();
    auto ranges = shaper.selection_boxes(line, text, first_begin, first_end);
    auto second = shaper.selection_boxes(line, text, last_begin, last_end);
    ranges.insert(ranges.end(), second.begin(), second.end());

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "select the two Latin letters around an Arabic run:\n";
    for (b::HorizontalRange range : ranges) {
        std::cout << "[" << range.begin << ", " << range.end << ")\n";
    }
}
