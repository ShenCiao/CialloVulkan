#version 460

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec3[] inColor;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out flat vec2 p0;
layout(location = 2) out flat vec2 p1;
layout(location = 3) out vec2 p;
layout(location = 4) out flat float width;

void main(){
    float w = 0.05;
    vec4 v01 = gl_in[1].gl_Position - gl_in[0].gl_Position;
    vec2 nv = normalize(v01.xy);
    vec2 n = vec2(-nv.y, nv.x);
    p0 = gl_in[0].gl_Position.xy;
    p1 = gl_in[1].gl_Position.xy;

    // Vertex 0
    width = w;
    p0 = gl_in[0].gl_Position.xy;
    p1 = gl_in[1].gl_Position.xy;
    gl_Position = vec4(p0 + n*w - nv*w, 0.0, 1.0);
    p = gl_Position.xy;
    fragColor = inColor[0];
    EmitVertex();
    // Vertex 1
    width = w;
    p0 = gl_in[0].gl_Position.xy;
    p1 = gl_in[1].gl_Position.xy;
    gl_Position = vec4(p0 - n*w - nv*w, 0.0, 1.0);
    p = gl_Position.xy;
    fragColor = inColor[0];
    EmitVertex();
    // Vertex 2
    width = w;
    p0 = gl_in[0].gl_Position.xy;
    p1 = gl_in[1].gl_Position.xy;
    gl_Position = vec4(p1 + n*w + nv*w, 0.0, 1.0);
    p = gl_Position.xy;
    fragColor = inColor[1];
    EmitVertex();
    // Vertex 3
    width = w;
    p0 = gl_in[0].gl_Position.xy;
    p1 = gl_in[1].gl_Position.xy;
    gl_Position = vec4(p1 - n*w + nv*w, 0.0, 1.0);
    p = gl_Position.xy;
    fragColor = inColor[1];
    EmitVertex();

    EndPrimitive();
}