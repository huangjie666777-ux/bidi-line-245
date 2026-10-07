#include "selection.hpp"

#include <algorithm>

namespace bidi_line245 {

std::vector<HorizontalRange> map_selection(const DecodedLine& decoded,
                                           const LineLayout& layout,
                                           std::size_t begin,
                                           std::size_t end) {
    std::vector<HorizontalRange> selected;
    for (std::size_t i = 0; i < decoded.code_points.size(); ++i) {
        const CodePoint& cp = decoded.code_points[i];
        if (cp.byte_begin < end && cp.byte_end > begin &&
            cp.byte_end > cp.byte_begin) {
            const HorizontalRange& box = layout.cluster_boxes[i];
            if (box.end > box.begin) selected.push_back(box);
        }
    }
    std::sort(selected.begin(), selected.end(),
              [](const HorizontalRange& a, const HorizontalRange& b) {
                  if (a.begin != b.begin) return a.begin < b.begin;
                  return a.end < b.end;
              });

    constexpr double epsilon = 0.0005;
    std::vector<HorizontalRange> merged;
    for (const HorizontalRange& range : selected) {
        if (merged.empty() || range.begin > merged.back().end + epsilon) {
            merged.push_back(range);
        } else if (range.end > merged.back().end) {
            merged.back().end = range.end;
        }
    }
    return merged;
}

}  // namespace bidi_line245
