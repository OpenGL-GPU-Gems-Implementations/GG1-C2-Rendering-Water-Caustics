#version 430 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec3 CPosition;

uniform samplerCube skybox;

void main() {
    // directional light
    /*vec3 lightDir = vec3(1, 5, 1);
    float diff = abs(dot(lightDir, Normal)) * 0.5 + 0.5;
    FragColor = vec4(Normal, 1);*/
    vec3 I = normalize(Position - CPosition);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1) * 0.7;
}