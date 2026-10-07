#include <bidi_line245/bidi_line245.hpp>

#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace bidi_line245;

namespace {
void check(bool condition, const std::string& message) {
    if (!condition) throw std::logic_error(message);
}

void expect_throws(const std::string& name, const std::function<void()>& fn) {
    try {
        fn();
    } catch (const std::exception&) {
        return;
    }
    throw std::logic_error(name + " did not throw");
}

bool near(double a, double b) { return std::fabs(a - b) < 0.01; }
}  // namespace

int main() {
    LineShaper shaper("fonts/DejaVuSans.ttf");

    LineLayout empty = shaper.shape_line("", 16.0);
    check(empty.glyphs.empty() && near(empty.width, 0.0), "empty line must return empty");

    expect_throws("invalid UTF-8", [&] { shaper.shape_line("A\xFF", 16.0); });
    expect_throws("newline", [&] { shaper.shape_line("A\n", 16.0); });
    expect_throws("zero size", [&] { shaper.shape_line("A", 0.0); });
    expect_throws("negative size", [&] { shaper.shape_line("A", -1.0); });
    expect_throws("private-use missing glyph", [&] { shaper.shape_line("\xEE\x80\x80", 16.0); });
    expect_throws("overlong input", [&] {
        shaper.shape_line(std::string(4097, 'A'), 16.0);
    });

    const std::string lam_alif = u8"\xD9\x84\xD8\xA7";
    LineLayout ligature = shaper.shape_line(lam_alif, 28.0, BaseDirection::Rtl);
    check(ligature.glyphs.size() < 2, "Arabic Laam-Aleef should ligature");
    check(ligature.glyphs.at(0).cluster_begin == 0 &&
              ligature.glyphs.at(0).cluster_end == lam_alif.size(),
          "ligature cluster must cover all merged bytes");

    const std::string combined = u8"A\xCC\x81";
    LineLayout marks = shaper.shape_line(combined, 24.0);
    check(marks.cluster_boxes.at(0).begin == marks.cluster_boxes.at(1).begin &&
              marks.cluster_boxes.at(0).end == marks.cluster_boxes.at(1).end,
          "combining mark shares base cluster box");
    auto mark_select = shaper.selection_boxes(marks, combined, 1, 3);
    check(mark_select.size() == 1 && near(mark_select.at(0).begin, 0.0),
          "partial cluster selection selects whole cluster");

    const std::string isolated = u8"A\xE2\x81\xA6" u8"B\xD8\xB9\xE2\x81\xA9" u8"C";
    LineLayout isolates = shaper.shape_line(isolated, 20.0, BaseDirection::Auto);
    check(isolates.glyphs.size() == 4, "directional isolates produce no visible glyphs");

    const std::string joiner_text = u8"\xD8\xA7\xE2\x80\x8D\xD9\x84\xD8\xB9";
    LineLayout with_joiner = shaper.shape_line(joiner_text, 24.0, BaseDirection::Rtl);
    check(with_joiner.glyphs.size() >= 1, "joiners remain with visible shaping clusters");

    const std::string mixed = u8"A \xD8\xB9 B";
    LineLayout line = shaper.shape_line(mixed, 32.0, BaseDirection::Ltr);
    check(line.glyphs.size() == 4 || line.glyphs.size() == 5, "space and three scripts shape");
    auto both_latin = shaper.selection_boxes(line, mixed, 0, 1);
    auto trailing = shaper.selection_boxes(line, mixed, mixed.size() - 1, mixed.size());
    both_latin.insert(both_latin.end(), trailing.begin(), trailing.end());
    check(both_latin.size() == 2, "mixed selection returns disconnected visual ranges");
    check(both_latin.at(0).end < both_latin.at(1).begin,
          "selection must not fill unselected Arabic");
    auto empty_selection = shaper.selection_boxes(line, mixed, 0, 0);
    check(empty_selection.empty(), "empty logical selection returns empty");
    expect_throws("selection out of bounds", [&] {
        shaper.selection_boxes(line, mixed, 0, mixed.size() + 1);
    });
    expect_throws("selection inverted", [&] {
        shaper.selection_boxes(line, mixed, 2, 1);
    });
    expect_throws("selection inside code point", [&] {
        shaper.selection_boxes(line, mixed, 3, 4);
    });

    double summed = 0.0;
    for (const Glyph& glyph : line.glyphs) summed += glyph.x_advance;
    check(near(summed, line.width), "visible glyph advances sum to line width");

    LineLayout auto_rtl = shaper.shape_line(u8"\xD7\xA9" u8"A", 20.0, BaseDirection::Auto);
    check(auto_rtl.base_level == 1.0, "auto direction follows first strong character");
    LineLayout auto_ltr = shaper.shape_line(u8"A\xD7\xA9", 20.0, BaseDirection::Auto);
    check(auto_ltr.base_level == 0.0, "auto LTR follows first strong character");

    std::cout << "selftest passed\n";
}
