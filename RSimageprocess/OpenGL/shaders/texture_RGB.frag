#version 410 core

in vec2 TexCoord;
out vec4 color;
uniform sampler2D textureSampler;

void main() {
    color = texture(textureSampler, TexCoord);
    if (color.r == 0 && color.g == 0 && color.b == 0)
        color = vec4(0.0,0.0,0.0,0.0);
}
