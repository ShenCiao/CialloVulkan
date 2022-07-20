#version 460

layout(location = 0) in vec2 inPos;
layout(location = 1) in float inPressure;

layout(location = 0) out vec3 outColor;

vec3 colorMap(float t){
    vec3 a = vec3(1.0, 0.0, 0.0);
    vec3 b = vec3(0.0, 1.0, 0.0);
    vec3 c = vec3(0.0, 0.0, 1.0);
    if(t < 0.5){
        return mix(a, b, t*2);
    }
    if(t >= 0.5){
        return mix(b, c, (t-0.5)*2);
    }
}

void main() {
    gl_Position = vec4(inPos, 0.0, 1.0);
    outColor = colorMap(inPressure);
}

