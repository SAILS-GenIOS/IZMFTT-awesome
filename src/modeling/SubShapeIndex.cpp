#include "SubShapeIndex.h"
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <cstdlib>

namespace SubShapeIndex {

int indexOf(const TopoDS_Shape& shape, const TopoDS_Shape& sub,
            TopAbs_ShapeEnum type) {
    if (shape.IsNull() || sub.IsNull()) return 0;
    try {
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(shape, type, map);
        return map.FindIndex(sub); // 0 when absent
    } catch (...) {
        return 0;
    }
}

TopoDS_Shape at(const TopoDS_Shape& shape, int index, TopAbs_ShapeEnum type) {
    if (shape.IsNull() || index <= 0) return {};
    try {
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(shape, type, map);
        if (index > map.Extent()) return {};
        return map.FindKey(index);
    } catch (...) {
        return {};
    }
}

std::string serialize(const TopoDS_Shape& shape,
                      const std::vector<TopoDS_Shape>& subs,
                      TopAbs_ShapeEnum type) {
    std::string out;
    if (shape.IsNull()) return out;
    try {
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(shape, type, map);
        for (const auto& s : subs) {
            int idx = map.FindIndex(s);
            if (idx <= 0) continue;
            if (!out.empty()) out += ',';
            out += std::to_string(idx);
        }
    } catch (...) {}
    return out;
}

std::vector<int> parse(const std::string& csv) {
    std::vector<int> out;
    size_t pos = 0;
    while (pos < csv.size()) {
        size_t end = csv.find(',', pos);
        if (end == std::string::npos) end = csv.size();
        int v = std::atoi(csv.substr(pos, end - pos).c_str());
        if (v > 0) out.push_back(v);
        pos = end + 1;
    }
    return out;
}

bool resolveAll(const TopoDS_Shape& shape, const std::vector<int>& indices,
                TopAbs_ShapeEnum type, std::vector<TopoDS_Shape>& out) {
    out.clear();
    if (shape.IsNull() || indices.empty()) return false;
    try {
        TopTools_IndexedMapOfShape map;
        TopExp::MapShapes(shape, type, map);
        for (int idx : indices) {
            if (idx <= 0 || idx > map.Extent()) { out.clear(); return false; }
            out.push_back(map.FindKey(idx));
        }
        return true;
    } catch (...) {
        out.clear();
        return false;
    }
}

} // namespace SubShapeIndex
