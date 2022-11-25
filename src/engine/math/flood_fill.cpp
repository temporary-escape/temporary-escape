#include "flood_fill.hpp"
#include <unordered_set>

using namespace Engine;

void FloodFill::operator()(const std::function<void(size_t, size_t)>& yield) const {
    const auto getNearestPoint = [&](const Vector2& pos) -> size_t {
        const Vector2* found = &positions.at(0);
        for (size_t i = 1; i < positions.size(); i++) {
            const auto a = glm::distance(pos, positions.at(i));
            const auto b = glm::distance(pos, *found);

            if (a < b) {
                found = &positions.at(i);
            }
        }

        assert(found != nullptr);
        return found - positions.data();
    };

    // Find the nearest start for each region
    std::unordered_map<size_t, std::vector<size_t>> progress;
    for (const auto& [id, pos] : startingPoints) {
        const auto positionIdx = getNearestPoint(pos);

        auto it = progress.insert(std::make_pair(id, std::vector<size_t>{})).first;
        it->second.push_back(positionIdx);
    }

    std::unordered_set<size_t> processed;

    while (true) {
        std::vector<size_t> toRemove;

        // For each unique ID we want to flood fill
        for (auto& [id, pointsIndexes] : progress) {
            std::vector<size_t> temp;
            std::swap(pointsIndexes, temp);

            // For each point from which we want to move forward
            for (auto& pointIndex : temp) {
                const auto& connectionsFromPoint = connections.at(pointIndex);

                // For each connection from the point
                for (const auto& otherIndex : connectionsFromPoint) {
                    // Was this point already processed?
                    if (const auto it = processed.find(otherIndex); it != processed.end()) {
                        continue;
                    }
                    processed.insert(otherIndex);
                    pointsIndexes.push_back(otherIndex);

                    yield(otherIndex, id);
                }
            }

            if (pointsIndexes.empty()) {
                toRemove.push_back(id);
            }
        }

        for (const auto& id : toRemove) {
            progress.erase(id);
        }

        if (progress.empty()) {
            break;
        }
    }
}
