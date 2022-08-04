#version 460

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec4 inColor;
layout(location = 2) in float inWidth;

layout(location = 0) out vec4 outColor;
layout(location = 1) out float outWidth;

//TDOO:: MVP
void main() {
    gl_Position = vec4(inPos, 0.0, 1.0);
    outColor = inColor;
    outWidth = inWidth;
}
