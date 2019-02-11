#version 330 core
out vec4 color;

//FIXED: THE FRAMEBUFFER REQUIRES VEC4 COLOR

in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
in vec4 shadow_coord;

uniform vec3 campos;
uniform float lfactor;
uniform vec3 lpos;

uniform sampler2D tex;
uniform sampler2D shadow_map_tex;

void main()
{

// Shadow calculations
// OLD
//float visibility = 1.0;

//if (texture(shadow_map_tex, shadow_coord.xy).z < shadow_coord.z) {
//    visibility = 0.5;
//}
// NEW
// The perspective divide
vec3 proj_coords = shadow_coord.xyz;// / shadow_coord.w;
proj_coords = proj_coords * 0.5 + 0.5;

float closestDepth = texture(shadow_map_tex, proj_coords.xy).r;
float currentDepth = proj_coords.z;

float shadow = currentDepth > closestDepth ? 1.0 : 0.0;


float other_light_factor = 1 - lfactor;

shadow *= lfactor;

//vec3 lightColor = vec3(1.0);

vec3 n = normalize(vertex_normal);
//vec3 lp = vec3(0,500,0);
//vec3 ld = normalize(lp - vertex_pos);
vec3 ld = normalize(lpos - vertex_pos);

float diffuse = dot(n,ld);
//float diffuse = max(dot(ld, n), 0.0);
//vec3 diffuse = diff * lightColor;

diffuse = clamp(diffuse, 0, 1);

vec2 vertex_tex_correct = vec2(vertex_tex.x, 1 - vertex_tex.y);

vec3 tcol = texture(tex, vertex_tex_correct).rgb;
//vec3 tcol = texture(tex, vertex_tex).rgb;

// Diffuse and ambient lighting
//color.rgb = tcol*diffuse*0.7*visibility + tcol*0.3;
color.rgb = tcol*diffuse*0.7*(1-shadow) + tcol*0.3;
//color.rgb = tcol*diffuse*0.7 + tcol*0.3;

vec3 cd = normalize(campos - vertex_pos);
vec3 h = normalize(cd+ld);
float spec = dot(n,h);
spec = pow(spec,10);

spec = clamp(spec,0,1);


// Specular lighting
//color.rgb += vec3(1,1,1)*spec*(10*other_light_factor + 1);//*visibility;
color.rgb += vec3(1,1,1)*spec*(10*other_light_factor + 1) * (1-shadow);//*visibility;
color.a = 1;
//color *= lfactor;
}
