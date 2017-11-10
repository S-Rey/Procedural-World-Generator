#version 330

in vec2 position;
in vec3 vpoint;

out float h;
out vec3 light_dir;
out vec3 view_dir;
out vec3 point;
out vec2 fposition;
out vec2 uv;

uniform mat4 MVP;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform float time;
uniform sampler2D tex;
uniform sampler2D tex_perlin;



void main() {
     uv = (position + vec2(1.0, 1.0)) / 2.0;

    // height of each pixel deducted from the perlin noise
    h = length(texture(tex_perlin, uv).rgb);
    vec3 pos_3d = vec3(position.x , h/3.0 - 0.15 , position.y) * 30.0 ;

    
    fposition = position;

    mat4 MV = view;
    vec4 vpoint_mv = MV * vec4(vpoint, 1.0);


    gl_Position = MVP * vec4(pos_3d, 1.0);
    vec3 light_pos = vec3(0.0f, 0.0f, 0.0f);

    light_dir = light_pos - vpoint_mv.xyz;
    light_dir = vec3(1.0,1.0, 1.0);
    view_dir = -vpoint_mv.xyz;

    point = vpoint;

}
