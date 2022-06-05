#include "Random.hpp"
#include "../Utils/Random.hpp"

using namespace Engine;

std::vector<Vector2> Engine::randomCirclePositions(std::mt19937_64& rng, float maxRadius, size_t maxCount,
                                                   float minDistance) {
    std::vector<Vector2> positions;
    auto tries = 20;

    while (tries > 0 && positions.size() < maxCount) {
        tries--;

        auto randomPos = Vector2{randomReal<float>(rng, 0.0f, maxRadius), 0.0f};
        randomPos = glm::rotate(randomPos, glm::radians(randomReal<float>(rng, 0.0f, 360.0f)));

        auto accepted = true;

        for (const auto& pos : positions) {
            if (glm::distance(randomPos, pos) < minDistance) {
                accepted = false;
                break;
            }
        }

        if (!accepted) {
            continue;
        }

        positions.push_back(randomPos);
        tries = 20;
    }

    return positions;
}
