#version 460

layout (location = 0) in vec2 pos;
layout (location = 1) in float pressure;

layout(location = 0) out float outWidth;

//TODO: add 2d camera's MVP

layout (set = 0, binding = 0) uniform Width{
    // vec2 pos0; 
    // vec2 pos1;
    float value;
} width;

// // linear equaltion for testing.
// float multiplier(vec2 p0, vec2 p1, float x){
//     float m = (p1.y - p0.y)/(p1.x - p0.x);
//     float c = p0.y - p0.x * (p1.y - p0.y)/(p1.x - p0.x);
//     return m*x + c;
// }

void main() {
    gl_Position = vec4(pos, 0.0, 1.0);
    // width = w.value*multiplier(w.pos0, w.pos1, pressure);
    outWidth = width.value;
}
