#version 330 core
layout (location=0) in vec3 inPos;
layout (location=1) in vec3 inNormal;

uniform mat4 mvp;

out vec3 normal;
out vec3 position;

void main() {
    normal = inNormal;
    position = inPos;
    gl_Position = mvp * vec4(inPos, 1.0);
}
