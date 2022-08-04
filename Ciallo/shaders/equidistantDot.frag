#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in flat vec2 p0;
layout(location = 3) in vec2 p;
layout(location = 4) in float width;

layout(location = 0) out vec4 outColor;

void main() {
    
    float d = distance(p0, p);
    if( d > width){
        discard;
    }

    outColor = vec4(fragColor);
}
