#version 330

in float h;
in vec3 light_dir;
in vec3 view_dir;
in vec3 point;
in vec2 fposition;
in vec2 uv;

out vec3 color;

uniform bool d;
uniform sampler2D tex;
uniform sampler2D tex_perlin;

uniform sampler2D snow;
uniform sampler2D rock;
uniform sampler2D grass;
uniform sampler2D sand;

vec3 normal(vec2 coordinates)
{
    float delta = 0.0000001;
    vec3 normal = vec3(0.0,0.0,1.0);
    // dont use edge coordinates
    if(coordinates != vec2(0.0, 0.0) ||
            coordinates != vec2(1.0, 1.0)) {

        normal = vec3(0.0, 0.0, 0.0);
        // compute the delta h for x coordinates
        float dhx = length(texture(tex_perlin, coordinates).rgb) -
                length(texture(tex_perlin, coordinates + vec2(delta, 0.0)).rgb);
        float alphax = tan (dhx / delta);

        normal += vec3(cos(alphax), 1.0, sin(alphax));

        // compute the delta h for y coordinates
        float dhy = length(texture(tex_perlin, coordinates).rgb) -
                length(texture(tex_perlin, coordinates + vec2(0.0, delta)).rgb);
        float alphay = tan(dhy / delta);

        normal += vec3(1.0, cos(alphay), sin(alphay));
        normal = normalize(normal);
    }

    return normal;
}

vec3 normal2(vec2 coordinates)
{
    float delta = 0.03;
    vec3 normal = vec3(0.0, 0.0, 1.0);

    float dhx = length(texture(tex_perlin, coordinates+ vec2(-delta, 0.0)).rgb) -
            length(texture(tex_perlin, coordinates + vec2(delta, 0.0)).rgb);
    float dhy = length(texture(tex_perlin, coordinates+ vec2(0.0, -delta)).rgb) -
            length(texture(tex_perlin, coordinates + vec2(0.0, delta)).rgb);

    vec3 vx = vec3(delta*2, 0.0, dhx);
    vec3 vy = vec3(0.0, delta*2, dhy);

    normal = normalize(cross(vx, vy));

    return normal;

}

void main() {

    vec3 Ld = vec3(1.0f, 1.0f, 1.0f);
    vec3 kd = vec3(0.9f, 0.5f, 0.5f);
    vec3 La = vec3(0.5f, 0.5, 0.5);
    vec3 ka = vec3(0.9f, 0.5f, 0.5f);
    vec3 l = normalize(light_dir);
    vec3 n;
    
    float grass_factor;
    float rock_factor;
    float snow_factor;
    
    vec2 position = fposition * 2; // better for the rendering of the textures
    
    if(h < 0.4) {
        color = texture(sand, position).rgb;
    } else if (h >= 0.4 && h < 0.6) {
        grass_factor = 5*h - 2;
        color = mix(texture(sand, position).rgb, texture(grass, position).rgb, vec3(grass_factor));
    } else if(h >= 0.6 && h < 0.7) {
        color = texture(grass, position).rgb;
    } else if(h >= 0.7 && h < 0.9) {
        rock_factor = 5*h - 3.5;
        color = mix(texture(grass, position).rgb, texture(rock, position).rgb, vec3(rock_factor));
    } else if(h >= 0.9 && h < 0.95) {
        color = texture(rock, position).rgb;
    } else if(h >= 0.95 && h < 1.15) {
        snow_factor = 5*h - 4.75;
        color = mix(texture(rock, position).rgb, texture(snow, position).rgb, vec3(snow_factor));
    } else {
        color = texture(snow, position).rgb;
    }

    vec3 vcolor = vec3(0.0, 0.0, 0.0);

    n = normal2(uv);
    float lambert = dot(n,l);
    if (lambert > 0) {
        vcolor += Ld*kd*lambert;
    }
    
    // discard the moutains that are below the water level in order to take the right reflects for the moutains
    if(d) {
        if (h < 0.45) {
            discard;
        }
    }
 
    color *= (length(vcolor) + 0.3); // 0.3 is for the ambient shading
}
