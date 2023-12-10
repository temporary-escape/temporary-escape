#version 450

layout(location = 0) in VS_OUT {
    vec4 color;
    vec4 texCoords;
} ps_in;

layout(push_constant) uniform Constants {
    mat4 mvp;
} constants;

layout(binding = 0) uniform sampler2D texSampler[16];

layout(location = 0) out vec4 outColor;

void main() {
    int mode = int(floor(ps_in.texCoords.z));
    int index = int(floor(ps_in.texCoords.w));

    if (mode == 1) {
        vec4 raw = texture(texSampler[index], ps_in.texCoords.xy);
        outColor = raw * ps_in.color;
    } else if (mode == 2) {
        vec4 raw = texture(texSampler[index], ps_in.texCoords.xy);
        outColor = ps_in.color * vec4(1.0, 1.0, 1.0, raw.r);
    } else {
        outColor = ps_in.color;
    }
}
