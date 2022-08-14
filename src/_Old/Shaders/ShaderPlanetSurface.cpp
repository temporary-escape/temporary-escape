#include "ShaderPlanetSurface.hpp"

using namespace Engine;

ShaderPlanetSurface::ShaderPlanetSurface(const Config& config) : Shader("ShaderPlanetSurface") {
    addFragmentShader(config.shadersPath / "planet-surface.frag");
    addVertexShader(config.shadersPath / "planet-surface.vert");
    link();
    use();

    setUniform("surfaceTexture", int(Samplers::SurfaceTexture));

    cameraUniformIndex = getUniformBlockIndex("Camera");
    uniformBlockBinding(cameraUniformIndex, Bindings::Camera);

    directionalLightsIndex = getUniformBlockIndex("DirectionalLights");
    uniformBlockBinding(directionalLightsIndex, Bindings::DirectionalLights);

    modelMatrixUniform = getUniformLocation("modelMatrix");
    normalMatrixUniform = getUniformLocation("normalMatrix");
}

void ShaderPlanetSurface::setModelMatrix(const Matrix4& matrix) const {
    setUniform(modelMatrixUniform, matrix);
}

void ShaderPlanetSurface::setNormalMatrix(const Matrix3& matrix) const {
    setUniform(normalMatrixUniform, matrix);
}

void ShaderPlanetSurface::bindCameraUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::Camera);
}

void ShaderPlanetSurface::bindSurfaceTexture(const Texture& texture) const {
    texture.bind(int(Samplers::SurfaceTexture));
}

void ShaderPlanetSurface::bindDirectionalLightsUniform(const VertexBuffer& ubo) const {
    ubo.bindBufferBase(Bindings::DirectionalLights);
}
