#version 410 core

in vec3 fragColor;
out vec4 color;
uniform float transparent;

void main() {
    color = vec4(fragColor.xyz, 1.0f);
}
