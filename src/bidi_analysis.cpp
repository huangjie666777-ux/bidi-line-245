#include "bidi_analysis.hpp"

#include <stdexcept>

namespace bidi_line245 {
namespace {
FriBidiParType resolve_base(const std::vector<FriBidiCharType>& types,
                            BaseDirection direction) {
    if (direction == BaseDirection::Ltr) return FRIBIDI_PAR_LTR;
    if (direction == BaseDirection::Rtl) return FRIBIDI_PAR_RTL;
    return fribidi_get_par_direction(types.data(),
                                     static_cast<FriBidiStrIndex>(types.size()));
}

}  // namespace

BidiAnalysis analyze_bidi(const DecodedLine& decoded, BaseDirection direction) {
    const std::size_t count = decoded.code_points.size();
    BidiAnalysis out;
    if (count == 0) return out;

    std::vector<FriBidiChar> logical(count);
    std::vector<FriBidiCharType> types(count);
    std::vector<FriBidiBracketType> brackets(count);
    std::vector<FriBidiLevel> levels(count);
    for (std::size_t i = 0; i < count; ++i) {
        logical[i] = decoded.code_points[i].value;
    }
    fribidi_get_bidi_types(logical.data(), static_cast<FriBidiStrIndex>(count), types.data());
    fribidi_get_bracket_types(logical.data(), static_cast<FriBidiStrIndex>(count),
                              types.data(), brackets.data());

    FriBidiParType base = resolve_base(types, direction);
    FriBidiParType resolved_base = base;
    if (!fribidi_get_par_embedding_levels_ex(
            types.data(), brackets.data(), static_cast<FriBidiStrIndex>(count),
            &resolved_base, levels.data())) {
        throw std::runtime_error("FriBidi level analysis failed");
    }
    out.base_level = static_cast<uint8_t>(resolved_base & 1);
    out.levels.assign(levels.begin(), levels.end());

    std::vector<FriBidiChar> mirrored = logical;
    fribidi_shape(FRIBIDI_FLAG_SHAPE_MIRRORING, levels.data(),
                  static_cast<FriBidiStrIndex>(count), nullptr, mirrored.data());
    for (std::size_t i = 0; i < count; ++i) {
        if (!is_invisible_bidi_control(decoded.code_points[i].value)) {
            out.shaped_code_points.push_back(mirrored[i]);
            out.shaped_logical_index.push_back(i);
        }
    }

    std::vector<FriBidiStrIndex> map(count);
    for (std::size_t i = 0; i < count; ++i) map[i] = static_cast<FriBidiStrIndex>(i);
    if (!fribidi_reorder_line(FRIBIDI_FLAGS_DEFAULT, types.data(),
                              static_cast<FriBidiStrIndex>(count), 0, resolved_base,
                              levels.data(), nullptr, map.data())) {
        throw std::runtime_error("FriBidi line reorder failed");
    }
    out.visual_to_logical.assign(map.begin(), map.end());

    for (std::size_t i = 0; i < out.shaped_logical_index.size(); ++i) {
        std::size_t logical_index = out.shaped_logical_index[i];
        bool has_previous_visible = i > 0 && out.shaped_logical_index[i - 1] + 1 == logical_index;
        if (out.runs.empty() || !has_previous_visible) {
            BidiRun run;
            run.logical_begin = logical_index;
            run.logical_end = logical_index + 1;
            run.level = out.levels[logical_index];
            run.rtl = (run.level & 1U) != 0;
            run.script = decoded.effective_script[logical_index];
            out.runs.push_back(run);
        } else {
            BidiRun& last = out.runs.back();
            uint8_t level = out.levels[logical_index];
            hb_script_t script = decoded.effective_script[logical_index];
            if (has_previous_visible && logical_index == last.logical_end && level == last.level &&
                script == last.script) {
                ++last.logical_end;
            } else {
                BidiRun run;
                run.logical_begin = logical_index;
                run.logical_end = logical_index + 1;
                run.level = level;
                run.rtl = (level & 1U) != 0;
                run.script = script;
                out.runs.push_back(run);
            }
        }
    }

    for (std::size_t v = 0; v < count; ++v) {
        FriBidiStrIndex li = map[v];
        if (li < 0 || is_invisible_bidi_control(decoded.code_points[li].value)) continue;
        std::size_t logical_index = static_cast<std::size_t>(li);
        std::size_t run_index = 0;
        while (run_index < out.runs.size() &&
               !(logical_index >= out.runs[run_index].logical_begin &&
                 logical_index < out.runs[run_index].logical_end)) {
            ++run_index;
        }
        if (run_index == out.runs.size()) throw std::runtime_error("internal bidi run lookup failed");

        if (out.visual_items.empty() || out.visual_items.back().run_index != run_index) {
            VisualItem item;
            item.run_index = run_index;
            item.visual_begin = v;
            item.visual_end = v + 1;
            item.rtl = out.runs[run_index].rtl;
            out.visual_items.push_back(item);
        } else {
            ++out.visual_items.back().visual_end;
        }
    }
    return out;
}

}  // namespace bidi_line245
