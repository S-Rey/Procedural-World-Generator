#version 330 core

in vec2 uv;

out vec3 color;

uniform float p[512];

uniform int n_octaves;



float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}
float lerp(float t, float a, float b) {
    return a + t * (b - a);
}
float grad(int hash, float x, float y, float z) {
      int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
      float u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
             v = h<4 ? y : h==12||h==14 ? x : z;
      return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}


float noise(float x, float y, float z) {
      int X = int(floor(x)) & 255;                 // FIND UNIT CUBE THAT
      int Y = int(floor(y)) & 255;                 // CONTAINS POINT.
      int Z = int(floor(z)) & 255;
      x -= floor(x);                                // FIND RELATIVE X,Y,Z
      y -= floor(y);                                // OF POINT IN CUBE.
      z -= floor(z);
      float u = fade(x);                               // COMPUTE FADE CURVES
      float v = fade(y);                               // FOR EACH OF X,Y,Z.
      float w = fade(z);

       // THE 8 CUBE CORNERS,

      int A  = int(p[X  ] +Y);
      int AA = int(p[A  ] +Z);
      int AB = int(p[A+1] +Z);
      int B  = int(p[X+1] +Y);
      int BA = int(p[B  ] +Z);
      int BB = int(p[B+1] +Z);


      return lerp(w, lerp(v, lerp(u, grad(int(p[AA  ]), x  , y  , z   ),  // AND ADD
                                     grad(int(p[BA  ]), x-1, y  , z   )), // BLENDED
                             lerp(u, grad(int(p[AB  ]), x  , y-1, z   ),  // RESULTS
                                     grad(int(p[BB  ]), x-1, y-1, z   ))),// FROM  8
                     lerp(v, lerp(u, grad(int(p[AA+1]), x  , y  , z-1 ),  // CORNERS
                                     grad(int(p[BA+1]), x-1, y  , z-1 )), // OF CUBE
                             lerp(u, grad(int(p[AB+1]), x  , y-1, z-1 ),
                                     grad(int(p[BB+1]), x-1, y-1, z-1 ))));
}




void main() {

    float perlinC = 0;

    for(int i = 1; i <= n_octaves; ++i) {
        perlinC += (noise(uv.x * float(i), uv.y* float(i), i*i) + 0.5) / (float(i * i)*2.0);// / float(i); //float(n_octaves);
    }
    color = vec3(perlinC);

}
