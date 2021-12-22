layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in float g_brightness[];
in vec4 g_color[];

out vec2 v_coords;
out float v_brightness;
out vec4 v_color;

uniform mat4 projectionMatrix;
uniform vec2 particleSize;

void main (void) {
    vec4 P = gl_in[0].gl_Position;

    v_brightness = g_brightness[0];
    v_color = g_color[0];

    // a: left-bottom 
    vec2 va = P.xy + vec2(-1.0, -1.0) * particleSize;
    gl_Position = projectionMatrix * vec4(va, P.zw);
    v_coords = vec2(-1.0, -1.0);
    EmitVertex();  
    
    // b: left-top
    vec2 vb = P.xy + vec2(-1.0, 1.0) * particleSize;
    gl_Position = projectionMatrix * vec4(vb, P.zw);
    v_coords = vec2(-1.0, 1.0);
    EmitVertex();  
    
    // d: right-bottom
    vec2 vd = P.xy + vec2(1.0, -1.0) * particleSize;
    gl_Position = projectionMatrix * vec4(vd, P.zw);
    v_coords = vec2(1.0, -1.0);
    EmitVertex();  

    // c: right-top
    vec2 vc = P.xy + vec2(1.0, 1.0) * particleSize;
    gl_Position = projectionMatrix * vec4(vc, P.zw);
    v_coords = vec2(1.0, 1.0);
    EmitVertex();  

    EndPrimitive();  
}   
