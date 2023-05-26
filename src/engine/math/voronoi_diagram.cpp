#include "voronoi_diagram.hpp"
#define JC_VORONOI_IMPLEMENTATION
#define JC_VORONOI_CLIP_IMPLEMENTATION
#include <jc_voronoi_clip.h>

using namespace Engine;

static Vector2 pointToVec(const jcv_point& point) {
    return {point.x, point.y};
}

static jcv_point vecToPoint(const Vector2& vec) {
    return {vec.x, vec.y};
}

VoronoiResult Engine::computeVoronoiDiagram(const std::vector<Vector2>& points, const std::vector<Vector2>& clip) {
    VoronoiResult result{};

    result.cells.resize(points.size());

    jcv_diagram diagram;
    memset(&diagram, 0, sizeof(jcv_diagram));
    auto deref = std::shared_ptr<jcv_diagram>(&diagram, [](jcv_diagram* d) { jcv_diagram_free(d); });

    jcv_clipping_polygon polygon;
    jcv_clipper clipper{};
    jcv_rect rect;
    auto jpoints = std::unique_ptr<jcv_point[]>(new jcv_point[points.size()]);

    Vector2 min;
    Vector2 max;

    for (size_t i = 0; i < points.size(); i++) {
        jpoints[i].x = points[i].x;
        jpoints[i].y = points[i].y;

        min.x = std::min(points[i].x, min.x);
        min.y = std::min(points[i].y, min.y);
        max.x = std::max(points[i].x, max.x);
        max.y = std::max(points[i].y, max.y);
    }

    min -= 10.0f;
    max += 10.0f;

    std::vector<jcv_point> clipPoints;

    if (!clip.empty()) {
        clipPoints.resize(clip.size());
        for (size_t i = 0; i < clip.size(); i++) {
            clipPoints[i] = vecToPoint(clip[i]);
        }
    } else {
        clipPoints = {
            {min.x, min.y},
            {max.x, min.y},
            {max.x, max.y},
            {min.x, max.y},
        };
    }

    rect.min.x = min.x;
    rect.min.y = min.y;
    rect.max.x = max.x;
    rect.max.y = max.y;

    polygon.num_points = clipPoints.size();
    polygon.points = clipPoints.data();

    clipper.test_fn = jcv_clip_polygon_test_point;
    clipper.clip_fn = jcv_clip_polygon_clip_edge;
    clipper.fill_fn = jcv_clip_polygon_fill_gaps;
    clipper.ctx = &polygon;

    jcv_diagram_generate(points.size(), jpoints.get(), nullptr, &clipper, &diagram);

    const jcv_site* sites = jcv_diagram_get_sites(&diagram);
    for (int i = 0; i < diagram.numsites; ++i) {
        const jcv_site* site = &sites[i];

        const auto idx = static_cast<size_t>(site->index);
        auto& dst = result.cells[idx];

        const jcv_graphedge* e = site->edges;
        while (e) {
            // draw_triangle(site->p, e->pos[0], e->pos[1]);
            auto& triangle = dst.emplace_back();
            triangle[0] = pointToVec(site->p);
            triangle[1] = pointToVec(e->pos[0]);
            triangle[2] = pointToVec(e->pos[1]);
            e = e->next;
        }
    }

    return result;
}
