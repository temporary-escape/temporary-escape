in vec2 v_texCoords;

layout(location = 0) out vec4 o_color;

uniform sampler2D depthTexture;
uniform sampler2D baseColorRoughnessTexture;
uniform sampler2D normalMetallicTexture;
uniform sampler2D emissiveAmbientTexture;

uniform ivec2 viewport;
uniform int mode;

void main() {
    float depth = texture(depthTexture, v_texCoords).r;
    if (depth > 0.9999) {
        discard;
    }

    vec4 baseColorRoughness = texture(baseColorRoughnessTexture, v_texCoords);
    vec3 baseColor = baseColorRoughness.rgb;
    float roughness = baseColorRoughness.a;
    vec4 normalMetallic = texture(normalMetallicTexture, v_texCoords);
    vec3 normal = normalMetallic.rgb;
    float metallic = normalMetallic.a;
    vec4 emissiveAmbient = texture(emissiveAmbientTexture, v_texCoords);
    vec3 emissive = emissiveAmbient.rgb;
    float ambient = emissiveAmbient.a;

    if (mode == 1) {
        o_color = vec4(baseColor, 1.0);
    } else if (mode == 2) {
        o_color = vec4(normal * 0.5 + 0.5, 1.0);
    } else if (mode == 3) {
        o_color = vec4(emissive, 1.0);
    } else if (mode == 4) {
        o_color = vec4(metallic, metallic, metallic, 1.0);
    } else if (mode == 5) {
        o_color = vec4(roughness, roughness, roughness, 1.0);
    } else if (mode == 6) {
        o_color = vec4(ambient, ambient, ambient, 1.0);
    } else {
        o_color = vec4(depth, depth, depth, 1.0);
    }
}