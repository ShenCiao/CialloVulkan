#version 460

layout(location = 0) in vec2 inPos;
layout(location = 1) in float inPressure;

layout(location = 0) out vec3 outColor;

void main() {
    gl_Position = vec4(inPos, 0.0, 1.0);
    outColor = vec3(0.1529, 0.0275, 0.7608);
}
