#version 460

layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec3[] inColor;
layout(location = 0) out vec3 fragColor;

void main(){
    float width = 0.05;
    vec4 v01 = gl_in[1].gl_Position - gl_in[0].gl_Position;
    vec2 nv = normalize(v01.xy);
    vec2 n = vec2(-nv.y, nv.x);
    vec2 p0 = gl_in[0].gl_Position.xy;
    vec2 p1 = gl_in[1].gl_Position.xy;

    // Vertex 0
    gl_Position = vec4(p0 + n*width - nv*width, 0.0, 1.0);
    fragColor = inColor[0];
    EmitVertex();
    // Vertex 1
    gl_Position = vec4(p0 - n*width - nv*width, 0.0, 1.0);
    fragColor = inColor[0];
    EmitVertex();
    // Vertex 2
    gl_Position = vec4(p1 + n*width + nv*width, 0.0, 1.0);
    fragColor = inColor[1];
    EmitVertex();
    // Vertex 3
    gl_Position = vec4(p1 - n*width + nv*width, 0.0, 1.0);
    fragColor = inColor[1];
    EmitVertex();

    EndPrimitive();
}