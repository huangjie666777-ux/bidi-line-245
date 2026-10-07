#include "unicode_text.hpp"

#include <hb-ot.h>

namespace bidi_line245 {

DecodedLine decode_utf8_line(const std::string& text) {
    DecodedLine result;
    std::size_t i = 0;
    while (i < text.size()) {
        unsigned char first = static_cast<unsigned char>(text[i]);
        std::size_t length = 0;
        uint32_t cp = 0;
        if ((first & 0x80) == 0) {
            length = 1;
            cp = first;
        } else if ((first & 0xE0) == 0xC0) {
            length = 2;
            cp = first & 0x1F;
        } else if ((first & 0xF0) == 0xE0) {
            length = 3;
            cp = first & 0x0F;
        } else if ((first & 0xF8) == 0xF0) {
            length = 4;
            cp = first & 0x07;
        } else {
            throw std::invalid_argument("input is not valid UTF-8");
        }

        if (i + length > text.size()) {
            throw std::invalid_argument("input is not valid UTF-8");
        }
        for (std::size_t k = 1; k < length; ++k) {
            unsigned char trail = static_cast<unsigned char>(text[i + k]);
            if ((trail & 0xC0) != 0x80) {
                throw std::invalid_argument("input is not valid UTF-8");
            }
            cp = (cp << 6) | (trail & 0x3F);
        }

        if ((length == 2 && cp < 0x80) ||
            (length == 3 && cp < 0x800) ||
            (length == 4 && cp < 0x10000) ||
            cp > 0x10FFFF || (cp >= 0xD800 && cp <= 0xDFFF)) {
            throw std::invalid_argument("input is not valid UTF-8");
        }
        if (cp == U'\n' || cp == U'\r' || cp == 0x0085 ||
            cp == 0x2028 || cp == 0x2029) {
            throw std::invalid_argument("input must be a single line");
        }

        CodePoint item;
        item.value = cp;
        item.byte_begin = i;
        item.byte_end = i + length;
        item.script = resolve_script(cp);
        result.code_points.push_back(item);
        if (result.code_points.size() > max_code_points) {
            throw std::length_error("input exceeds 4096 code points");
        }
        i += length;
    }
    result.effective_script = resolve_effective_scripts(result.code_points);
    return result;
}

void validate_byte_range(const std::string& text, std::size_t begin, std::size_t end) {
    if (end < begin || begin > text.size() || end > text.size()) {
        throw std::out_of_range("selection byte range is out of bounds or inverted");
    }
    auto boundary = [&](std::size_t pos) {
        if (pos == text.size()) return true;
        return (static_cast<unsigned char>(text[pos]) & 0xC0) != 0x80;
    };
    if (!boundary(begin) || !boundary(end)) {
        throw std::invalid_argument("selection endpoint is not on a code point boundary");
    }
}

bool is_invisible_bidi_control(uint32_t cp) {
    return cp == 0x200E || cp == 0x200F ||
           (cp >= 0x202A && cp <= 0x202E) ||
           (cp >= 0x2066 && cp <= 0x2069) ||
           cp == 0xFEFF;
}

hb_script_t resolve_script(uint32_t cp) {
    hb_unicode_funcs_t* funcs = hb_unicode_funcs_get_default();
    return static_cast<hb_script_t>(hb_unicode_script(funcs, cp));
}

std::vector<hb_script_t> resolve_effective_scripts(const std::vector<CodePoint>& cps) {
    std::vector<hb_script_t> script(cps.size(), HB_SCRIPT_COMMON);
    hb_script_t current = HB_SCRIPT_COMMON;
    for (std::size_t i = 0; i < cps.size(); ++i) {
        hb_script_t s = cps[i].script;
        if (s != HB_SCRIPT_COMMON && s != HB_SCRIPT_INHERITED &&
            s != HB_SCRIPT_UNKNOWN) {
            current = s;
        }
        script[i] = current;
    }
    current = HB_SCRIPT_COMMON;
    for (std::size_t i = cps.size(); i-- > 0;) {
        hb_script_t s = cps[i].script;
        if (s != HB_SCRIPT_COMMON && s != HB_SCRIPT_INHERITED &&
            s != HB_SCRIPT_UNKNOWN) {
            current = s;
        }
        if (script[i] == HB_SCRIPT_COMMON) {
            script[i] = current;
        }
    }
    return script;
}

}  // namespace bidi_line245
