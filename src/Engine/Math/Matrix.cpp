#include "Matrix.hpp"

using namespace Engine;

Matrix4 Engine::withoutScale(const Matrix4& mat) {
    float scaleX = glm::length(glm::vec3(mat[0][0], mat[0][1], mat[0][2]));
    float scaleY = glm::length(glm::vec3(mat[1][0], mat[1][1], mat[1][2]));
    float scaleZ = glm::length(glm::vec3(mat[2][0], mat[2][1], mat[2][2]));
    return glm::scale(mat, glm::vec3(1.0f / scaleX, 1.0f / scaleY, 1.0f / scaleZ));
}
