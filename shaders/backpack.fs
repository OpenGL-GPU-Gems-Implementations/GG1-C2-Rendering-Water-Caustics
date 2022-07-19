#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;
in vec3 CPosition;

uniform sampler2D texture_diffuse1;

void main() {
    // directional light
    vec3 lightDir = vec3(1, 5, 1);
    float diff = dot(lightDir, Normal);
    FragColor = texture(texture_diffuse1, TexCoords);//* diff;
}