#version 410 core

in vec2 TexCoord;
out vec4 color;
uniform sampler2D textureSampler;

void main() {
    float intensity = texture(textureSampler, TexCoord).r;
    if (intensity > 0)
        color = vec4(intensity, intensity, intensity, 1.0);
    else
        color = vec4(0.0,0.0,0.0,0.0);
}
