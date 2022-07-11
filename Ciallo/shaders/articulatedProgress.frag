#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in flat vec2 p0;
layout(location = 2) in flat vec2 p1;
layout(location = 3) in vec2 p;
layout(location = 4) in flat float width;

layout(location = 0) out vec4 outColor;

void main() {
    
    vec2 L = normalize(p1 - p0);
    vec2 H = vec2(-L.y, L.x);

    vec2 pLH = vec2(dot(p-p0, L), dot(p-p0, H))/width;
    vec2 p0LH = vec2(0, 0);
    vec2 p1LH = vec2(length(p1-p0)/width, 0);

    float d0LH = distance(pLH, p0LH);
    float d1LH = distance(pLH, p1LH);

    if((pLH.x < 0 && d0LH > 1.0)){
        discard;
    }
    if((pLH.x > p1LH.x && d1LH > 1.0)){
        discard;
    }

    outColor = vec4(fragColor, 1.0);
}
