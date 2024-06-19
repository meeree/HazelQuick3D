#version 330 core
in vec3 normal;
in vec3 position;

uniform vec3 lightPos1;

out vec4 fragColor;

void main() {
    vec3 lightVec1 = normalize(lightPos1 - position);
    float d1 = abs(dot(lightVec1, normal)); // Note the abs means normal sign does not matter.
    vec3 c1 = vec3(1);  

    // Retrieve bottom left coordinate of sprite from sprite sheet.
    vec3 ambient = 0.8 * vec3(1.0);
    fragColor = vec4(ambient + d1 * c1, 1.0);
    fragColor = vec4(normal, 1.0);
}
