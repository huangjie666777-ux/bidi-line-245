#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include <hb.h>

namespace bidi_line245 {

struct CodePoint {
    uint32_t value = 0;
    std::size_t byte_begin = 0;
    std::size_t byte_end = 0;
    hb_script_t script = HB_SCRIPT_COMMON;
};

struct DecodedLine {
    std::vector<CodePoint> code_points;
    std::vector<hb_script_t> effective_script;
};

constexpr std::size_t max_code_points = 4096;

DecodedLine decode_utf8_line(const std::string& text);
void validate_byte_range(const std::string& text, std::size_t begin, std::size_t end);
bool is_invisible_bidi_control(uint32_t cp);
hb_script_t resolve_script(uint32_t cp);
std::vector<hb_script_t> resolve_effective_scripts(const std::vector<CodePoint>& cps);

}  // namespace bidi_line245
