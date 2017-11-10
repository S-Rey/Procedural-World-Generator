#version 330 core

in vec3 vpoint;

out vec2 uv;
out float perlinC;


void main() {
    uv = vpoint.xy * 3.0;
    gl_Position = vec4(vpoint, 1.0);
}
