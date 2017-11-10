#version 330

in vec2 position;
in vec2 vtexcoord;

out vec2 uv;

uniform mat4 MVP;
uniform float time;

void main() {

    float height = 0.0f; //hauteur de la plaque d'eau
    vec3 pos_3d = vec3(position.x, height, position.y) * 30.0;
    
    gl_Position = MVP * vec4(pos_3d, 1.0);
    uv = vtexcoord;
}
