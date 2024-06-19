#version 330 core
layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inColor;

uniform mat4 mvp;

out vec3 color;
out vec3 position;

void main() {
    color = inColor;
    position = inPos;
    gl_Position = mvp * vec4(inPos, 1.0);
}
