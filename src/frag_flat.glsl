#version 330 core
in vec3 color;
in vec3 position;

out vec4 fragColor;

void main() {
    fragColor = vec4(color, 1.0);
}
