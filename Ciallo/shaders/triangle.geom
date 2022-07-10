#version 460

layout(triangles) in;
layout(triangle_strip, max_vertices = 4) out;

layout(location = 0) in vec3[] inColor;
layout(location = 0) out vec3 fragColor;

void main(){
    vec4 v01 = gl_in[1].gl_Position - gl_in[0].gl_Position;
    vec4 v02 = gl_in[2].gl_Position - gl_in[0].gl_Position;

    float scale = 0.5;
    // vertex 0
    gl_Position = gl_in[0].gl_Position;
    fragColor = inColor[0];
    EmitVertex();
    // vertex 1
    gl_Position = gl_in[0].gl_Position + scale*v01;
    fragColor = inColor[1];
    EmitVertex();
    // vertex 2
    gl_Position = gl_in[0].gl_Position + scale*v02;
    fragColor = inColor[2];
    EmitVertex();
    // vertex 3
    gl_Position = vec4(gl_in[0].gl_Position.x, -gl_in[0].gl_Position.y , 0.0, 1.0);
    fragColor = vec3(0.0, 0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}