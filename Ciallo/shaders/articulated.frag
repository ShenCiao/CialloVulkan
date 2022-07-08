#version 460

layout(location = 0) in vec2 p0;
layout(location = 1) in vec2 p1;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform Color{
    vec4 value;
} color;

void main() {
    outColor = vec4(0.0, 0.0, 0.0, 1.0);
}
