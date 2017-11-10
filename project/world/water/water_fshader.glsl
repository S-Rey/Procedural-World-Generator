#version 330

in vec2 uv;
in vec4 gl_FragCoord;

out vec4 color;

uniform sampler2D tex;
uniform sampler2D tex_mirror;

void main() {
    
    vec2 window = textureSize(tex_mirror, 0);
    
    float _u = gl_FragCoord.x / window.x;
    float _v = gl_FragCoord.y / window.y;
    
    vec3 color2 = mix(vec3(0.62, 0.82, 0.87), vec3(texture(tex_mirror, vec2(_u, 1.0 - _v))), vec3(.5));
    
    color = vec4(color2.xyz, 0.8); //alpha à 0.8 afin d'avoir de l'eau un peu transparente
}
