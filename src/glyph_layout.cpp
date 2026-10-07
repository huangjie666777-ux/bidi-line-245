#include "glyph_layout.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <hb-ot.h>

namespace bidi_line245 {
namespace {
double px_from_hb(hb_position_t value, double font_size_px) {
    return static_cast<double>(value) * font_size_px / 64.0;
}
}

FontShaper::FontShaper(const std::string& font_path) {
    hb_blob_t* blob = hb_blob_create_from_file(font_path.c_str());
    if (!hb_blob_get_length(blob)) {
        hb_blob_destroy(blob);
        throw std::runtime_error("cannot open font: " + font_path);
    }
    hb_face_t* face = hb_face_create(blob, 0);
    hb_blob_destroy(blob);
    if (hb_face_get_glyph_count(face) == 0) {
        hb_face_destroy(face);
        throw std::runtime_error("invalid font: " + font_path);
    }
    font_ = hb_font_create(face);
    hb_face_destroy(face);
    hb_ot_font_set_funcs(font_);
}

FontShaper::~FontShaper() {
    if (font_) hb_font_destroy(font_);
}

RunLayout FontShaper::shape_run(const DecodedLine& decoded, const BidiAnalysis& bidi,
                                const BidiRun& run, double font_size_px) const {
    hb_buffer_t* buffer = hb_buffer_create();
    std::vector<std::size_t> visible_logical;
    std::vector<uint32_t> code_points;
    for (std::size_t i = run.logical_begin; i < run.logical_end; ++i) {
        if (!is_invisible_bidi_control(decoded.code_points[i].value)) {
            visible_logical.push_back(i);
            auto it = std::find(bidi.shaped_logical_index.begin(),
                                bidi.shaped_logical_index.end(), i);
            std::size_t shaped_index = static_cast<std::size_t>(
                it - bidi.shaped_logical_index.begin());
            uint32_t cp = bidi.shaped_code_points[shaped_index];
            code_points.push_back(cp);
        }
    }

    hb_buffer_set_direction(buffer, run.rtl ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
    hb_buffer_set_script(buffer, run.script);
    hb_buffer_set_cluster_level(buffer, HB_BUFFER_CLUSTER_LEVEL_MONOTONE_CHARACTERS);
    hb_buffer_add_utf32(buffer, code_points.data(),
                        static_cast<int>(code_points.size()), 0,
                        static_cast<int>(code_points.size()));
    unsigned input_count = hb_buffer_get_length(buffer);
    hb_glyph_info_t* input_info = hb_buffer_get_glyph_infos(buffer, &input_count);
    for (unsigned i = 0; i < input_count; ++i) input_info[i].cluster = i;
    hb_shape(font_, buffer, nullptr, 0);

    unsigned glyph_count = 0;
    hb_glyph_info_t* infos = hb_buffer_get_glyph_infos(buffer, &glyph_count);
    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buffer, &glyph_count);
    for (unsigned g = 0; g < glyph_count; ++g) {
        if (infos[g].codepoint == 0) {
            hb_buffer_destroy(buffer);
            throw std::domain_error("font is missing a glyph for input");
        }
    }

    std::vector<std::size_t> cluster_starts;
    for (unsigned g = 0; g < glyph_count; ++g) {
        std::size_t start = static_cast<std::size_t>(infos[g].cluster);
        if (cluster_starts.empty() || cluster_starts.back() != start) {
            cluster_starts.push_back(start);
        }
    }

    std::vector<std::size_t> cluster_starts_logical = cluster_starts;
    std::sort(cluster_starts_logical.begin(), cluster_starts_logical.end());
    std::vector<std::size_t> cluster_to_ordinal(visible_logical.size(), 0);
    for (std::size_t ordinal = 0; ordinal < cluster_starts_logical.size(); ++ordinal) {
        std::size_t next = (ordinal + 1 < cluster_starts_logical.size())
                               ? cluster_starts_logical[ordinal + 1]
                               : visible_logical.size();
        for (std::size_t k = cluster_starts_logical[ordinal]; k < next; ++k) {
            cluster_to_ordinal[k] = ordinal;
        }
    }
    std::vector<std::size_t> cluster_byte_begin(cluster_starts_logical.size());
    std::vector<std::size_t> cluster_byte_end(cluster_starts_logical.size());
    for (std::size_t ordinal = 0; ordinal < cluster_starts_logical.size(); ++ordinal) {
        std::size_t start = cluster_starts_logical[ordinal];
        std::size_t next = (ordinal + 1 < cluster_starts_logical.size())
                               ? cluster_starts_logical[ordinal + 1]
                               : visible_logical.size();
        cluster_byte_begin[ordinal] = decoded.code_points[visible_logical[start]].byte_begin;
        cluster_byte_end[ordinal] = (next < visible_logical.size())
                                        ? decoded.code_points[visible_logical[next]].byte_begin
                                        : decoded.code_points[run.logical_end - 1].byte_end;
    }

    RunLayout result;
    double pen = 0.0;
    for (unsigned g = 0; g < glyph_count; ++g) {
        std::size_t visible_index = static_cast<std::size_t>(infos[g].cluster);
        if (visible_index >= visible_logical.size()) {
            hb_buffer_destroy(buffer);
            throw std::runtime_error("HarfBuzz returned an invalid cluster");
        }
        std::size_t ordinal = cluster_to_ordinal[visible_index];
        double advance = std::abs(px_from_hb(positions[g].x_advance, font_size_px));
        double x_offset = px_from_hb(positions[g].x_offset, font_size_px);
        double y_offset = -px_from_hb(positions[g].y_offset, font_size_px);
        if (run.rtl) {
            x_offset = -x_offset;
            pen -= advance;
        } else {
            pen += advance;
        }

        Glyph glyph;
        glyph.glyph_id = infos[g].codepoint;
        glyph.cluster_begin = cluster_byte_begin[ordinal];
        glyph.cluster_end = cluster_byte_end[ordinal];
        glyph.baseline_x = pen + x_offset;
        glyph.baseline_y = 0.0;
        glyph.x_advance = advance;
        glyph.y_advance = 0.0;
        glyph.x_offset = x_offset;
        glyph.y_offset = y_offset;
        glyph.bidi_level = run.level;
        glyph.rtl = run.rtl;
        result.glyphs.push_back(glyph);
    }
    result.width = 0.0;
    for (const Glyph& glyph : result.glyphs) result.width += glyph.x_advance;
    for (Glyph& glyph : result.glyphs) {
        if (run.rtl) glyph.baseline_x += result.width;
    }

    result.cluster_boxes.assign(run.logical_end - run.logical_begin, HorizontalRange{});
    double cluster_pen = 0.0;
    for (std::size_t ci = 0; ci < cluster_starts_logical.size(); ++ci) {
        std::size_t start = cluster_starts_logical[ci];
        std::size_t next = (ci + 1 < cluster_starts_logical.size())
                               ? cluster_starts_logical[ci + 1]
                               : visible_logical.size();
        double cluster_width = 0.0;
        for (unsigned g = 0; g < glyph_count; ++g) {
            std::size_t current = static_cast<std::size_t>(infos[g].cluster);
            if (current >= start && current < next) cluster_width += result.glyphs[g].x_advance;
        }
        double left = run.rtl ? result.width - cluster_pen - cluster_width : cluster_pen;
        HorizontalRange box{left, left + cluster_width};
        for (std::size_t k = start; k < next; ++k) {
            result.cluster_boxes[visible_logical[k] - run.logical_begin] = box;
        }
        cluster_pen += cluster_width;
    }

    hb_buffer_destroy(buffer);
    return result;
}

LineLayout FontShaper::layout(const DecodedLine& decoded, const BidiAnalysis& bidi,
                               double font_size_px) const {
    if (!(font_size_px > 0.0) || !std::isfinite(font_size_px)) {
        throw std::invalid_argument("font size must be a positive finite pixel size");
    }
    int scale = static_cast<int>(std::lround(font_size_px * 64.0));
    hb_font_set_scale(font_, scale, scale);

    LineLayout line;
    line.base_level = bidi.base_level;
    line.cluster_boxes.resize(decoded.code_points.size());
    std::vector<RunLayout> runs;
    runs.reserve(bidi.runs.size());
    for (std::size_t i = 0; i < bidi.runs.size(); ++i) {
        RunLayout run_layout = shape_run(decoded, bidi, bidi.runs[i], font_size_px);
        run_layout.run_index = i;
        runs.push_back(std::move(run_layout));
    }

    double cursor = 0.0;
    for (const VisualItem& item : bidi.visual_items) {
        const BidiRun& run = bidi.runs[item.run_index];
        RunLayout& shaped = runs[item.run_index];
        for (Glyph glyph : shaped.glyphs) {
            glyph.baseline_x += cursor;
            line.glyphs.push_back(glyph);
        }
        for (std::size_t i = run.logical_begin; i < run.logical_end; ++i) {
            if (!is_invisible_bidi_control(decoded.code_points[i].value)) {
                const HorizontalRange& local =
                    shaped.cluster_boxes[i - run.logical_begin];
                line.cluster_boxes[i] =
                    HorizontalRange{cursor + local.begin, cursor + local.end};
            }
        }
        cursor += shaped.width;
    }
    line.width = cursor;
    return line;
}

}  // namespace bidi_line245
