#version 410 core

in vec2 TexCoord;
out vec4 color;
uniform sampler2D image;

void main() {
    color = texture(image, TexCoord);
}
