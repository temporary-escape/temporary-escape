in VS_OUT {
    vec2 texCoords;
} vsOut;

layout(location = 0) out vec4 o_color;

uniform sampler2D inputTexture;
uniform bool horizontal;
uniform int radius;
uniform float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform float weight[32 + 1];

/*void main() {
    vec2 texOffset = 1.0 / textureSize(inputTexture, 0);// gets size of single texel
    vec3 result = texture(inputTexture, vsOut.texCoords).rgb * weights[0];// current fragment's contribution
    if (horizontal)
    {
        for (int i = 1; i < 5 + 1; ++i)
        {
            result += texture(inputTexture, vsOut.texCoords + vec2(texOffset.x * i, 0.0)).rgb * weights[i];
            result += texture(inputTexture, vsOut.texCoords - vec2(texOffset.x * i, 0.0)).rgb * weights[i];
        }
    }
    else
    {
        for (int i = 1; i < 5 + 1; ++i)
        {
            result += texture(inputTexture, vsOut.texCoords + vec2(0.0, texOffset.y * i)).rgb * weights[i];
            result += texture(inputTexture, vsOut.texCoords - vec2(0.0, texOffset.y * i)).rgb * weights[i];
        }
    }
    o_color = result;
}*/

float calcGauss(float x, float sigma) {
    if (sigma <= 0.0)
    return 0.0;
    return exp(-(x*x) / (2.0 * sigma)) / (2.0 * 3.14157 * sigma);
}

void main() {
    const float radius = 32.0;

    float sigma = 0.7;
    vec2 u_dir = vec2(0.0, 1.0);
    if (horizontal) {
        u_dir = vec2(1.0, 0.0);
    }
    vec2 texC     = vsOut.texCoords;
    vec3 texCol   = texture(inputTexture, texC).rgb;
    vec3 gaussCol = texCol.rgb;
    vec2 step     = u_dir / textureSize(inputTexture, 0);

    float sum = 1.0;
    for (int i = 1; i <= radius; ++ i)
    {
        float weight = calcGauss(float(i) / radius, sigma * 0.5);

        sum += weight * 2.0;

        texCol    = texture(inputTexture, texC + step * float(i)).rgb;
        gaussCol += texCol.rgb * weight;

        texCol    = texture(inputTexture, texC - step * float(i)).rgb;
        gaussCol += texCol.rgb * weight;
    }
    gaussCol.rgb = clamp(gaussCol.rgb / sum, 0.0, 1.0);
    o_color = vec4(gaussCol.rgb, 1.0);
}
