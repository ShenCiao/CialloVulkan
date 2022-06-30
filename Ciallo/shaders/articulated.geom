#version 460

layout (lines) in;
layout (triangle_strip, max_vertices = 4) out;

layout (location = 0) in float width[];
layout (location = 0) out vec2 point0;
layout (location = 1) out vec2 point1;

void main(){
    vec2 p0 = gl_in[0].gl_Position.xy;
    vec2 p1 = gl_in[1].gl_Position.xy;
    point0 = p0;
    point1 = p1;

    vec2 v = normalize(p1 - p0); // normalized vector
    vec2 n = vec2(-v.y, v.x); // norm

    vec2 vertPos;
    // emit p0 side left vertex
    vertPos = p0 + ( +n*width[0] - v*width[0]);
    gl_Position = vec4(vertPos, 0.0, 1.0);
    EmitVertex();
    // emit p0 side right vertex
    vertPos = p0 + ( -n*width[0] - v*width[0]);
    gl_Position = vec4(vertPos, 0.0, 1.0);
    EmitVertex();
    // emit p1 side left vertex
    vertPos = p1 + ( +n*width[1] + v*width[1]);
    gl_Position = vec4(vertPos, 0.0, 1.0);
    EmitVertex();
    // emit p1 side right vertex
    vertPos = p1 + ( -n*width[1] + v*width[1]);
    gl_Position = vec4(vertPos, 0.0, 1.0);
    EmitVertex();
}