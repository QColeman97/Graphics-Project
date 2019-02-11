#version 330 core
out vec4 color;

//FIXED: THE FRAMEBUFFER REQUIRES VEC4 COLOR

in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;

uniform sampler2D shadow_map_tex;

void main()
{

//vec2 vertex_tex_correct = vec2(vertex_tex.x, 1 - vertex_tex.y);

vec3 tcol = texture(shadow_map_tex, vertex_tex).rgb;

// Diffuse and ambient lighting
//color.rgb = tcol*diffuse*0.7*visibility + tcol*0.3;

color.rgb = tcol;
//color.rgb = vec3(vertex_tex, 0);

color.a = 1;
//color *= lfactor;
}
