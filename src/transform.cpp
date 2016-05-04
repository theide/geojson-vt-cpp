#include <mapbox/geojsonvt/transform.hpp>

#include <cmath>

namespace mapbox {
namespace geojsonvt {

inline int16_t trans(double p, uint16_t extent, uint32_t z2, uint32_t px) {
    return std::round(extent * (p * z2 - px));
}

TilePoint Transform::transformPoint(
    const ProjectedPoint& p, uint16_t extent, uint32_t z2, uint32_t tx, uint32_t ty) {
    int16_t x = trans(p.x, extent, z2,tx);
    int16_t y = trans(p.y, extent, z2,ty);
    return TilePoint(x, y);
}

const Tile& Transform::transformTile(Tile& tile, uint16_t extent) {
    if (tile.transformed) {
        return tile;
    }

    const uint32_t z2 = tile.z2;
    const uint32_t tx = tile.tx;
    const uint32_t ty = tile.ty;

    for (auto& feature : tile.features) {
        const auto& geom = feature.geometry;
        const auto type = feature.type;

        if (type == TileFeatureType::Point) {
            auto& tileGeom = feature.tileGeometry.get<TilePoints>();
            auto const& projected_points = geom.get<ProjectedPoints>();
            tileGeom.reserve(projected_points.size());
            for (const auto& pt : projected_points) {
                tileGeom.emplace_back(trans(pt.x,extent,z2,tx),trans(pt.y,extent,z2,ty));
            }

        } else {
            feature.tileGeometry.set<TileRings>();
            auto& tileGeom = feature.tileGeometry.get<TileRings>();
            for (const auto& r : geom.get<ProjectedRings>()) {
                TilePoints ring;
                ring.reserve(r.points.size());
                for (const auto& pt : r.points) {
                    ring.emplace_back(trans(pt.x,extent,z2,tx),trans(pt.y,extent,z2,ty));
                }
                tileGeom.push_back(std::move(ring));
            }
        }
    }

    tile.transformed = true;

    return tile;
}

} // namespace geojsonvt
} // namespace mapbox
