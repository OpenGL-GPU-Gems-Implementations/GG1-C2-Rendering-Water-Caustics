#version 430 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;
in vec3 CPosition;

uniform sampler2D texture_diffuse1;
uniform sampler2D normal;
uniform sampler2D refractions;

void main() {
    // directional light
    vec3 lightDir = vec3(1, 5, 1);
    float diff = dot(lightDir, Normal);
    vec4 p0 = texture(normal, Position.xz/25 + vec2(25, 25));
    vec2 tp = refract(vec3(0, 1, 0), normalize(p0.xyz), 1.33).xy;
    vec4 p1 = texture(refractions, tp);
    //if (p1.x < 0.1) {
    //    p1 = vec4(1, 1, 1, 1);
    //}
    FragColor = p1;
    FragColor = p1 + texture(texture_diffuse1, TexCoords) * 0.5;
    //FragColor = vec4(tp.x, tp.y, 0, 1);
    //FragColor = max(vec4(0), sqrt(2*p1-1.5)) + texture(texture_diffuse1, TexCoords) * 0.5;
    //mix(max(vec4(0), sqrt(2*p1-1.5)), texture(texture_diffuse1, TexCoords), 0.5);//* diff;
}