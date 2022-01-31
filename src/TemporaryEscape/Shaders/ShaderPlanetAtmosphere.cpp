#include "ShaderPlanetAtmosphere.hpp"

using namespace Engine;

ShaderPlanetAtmosphere::ShaderPlanetAtmosphere(const Config& config) : Shader("ShaderPlanetAtmosphere") {
    addFragmentShader(config.shadersPath / "planet-atmosphere.frag");
    addVertexShader(config.shadersPath / "planet-atmosphere.vert");
    link();
    use();

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    directionalLightsIndex = getUniformBlockIndex("DirectionalLights");
    uniformBlockBinding(directionalLightsIndex, Bindings::DirectionalLights);

    modelMatrixUniform = getUniformLocation("modelMatrix");
    normalMatrixUniform = getUniformLocation("normalMatrix");
}

void ShaderPlanetAtmosphere::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderPlanetAtmosphere::setNormalMatrix(const Matrix3& matrix) const {
    setUniform(normalMatrixUniform, matrix);
}

void ShaderPlanetAtmosphere::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

void ShaderPlanetAtmosphere::bindDirectionalLightsUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::DirectionalLights);
}
