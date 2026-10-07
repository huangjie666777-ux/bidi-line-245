#pragma once

#include <cstdint>
#include <vector>

#include <fribidi.h>

#include "../include/bidi_line245/bidi_line245.hpp"
#include "unicode_text.hpp"

namespace bidi_line245 {

struct BidiRun {
    std::size_t logical_begin = 0;
    std::size_t logical_end = 0;
    bool rtl = false;
    uint8_t level = 0;
    hb_script_t script = HB_SCRIPT_COMMON;
};

struct VisualItem {
    std::size_t run_index = 0;
    std::size_t visual_begin = 0;
    std::size_t visual_end = 0;
    bool rtl = false;
};

struct BidiAnalysis {
    std::vector<uint8_t> levels;
    std::vector<uint32_t> shaped_code_points;
    std::vector<std::size_t> shaped_logical_index;
    std::vector<FriBidiStrIndex> visual_to_logical;
    std::vector<BidiRun> runs;
    std::vector<VisualItem> visual_items;
    uint8_t base_level = 0;
};

BidiAnalysis analyze_bidi(const DecodedLine& decoded, BaseDirection direction);

}  // namespace bidi_line245
