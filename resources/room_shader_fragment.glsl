#version 330 core
out vec4 color;

in vec3 vertex_pos;
in vec3 vertex_normal;
in vec4 shadow_coord;

uniform vec3 campos;
uniform float lfactor;
uniform vec3 lpos;

uniform sampler2D shadow_map_tex;

void main()
{
    // NEW
    vec3 proj_coords = shadow_coord.xyz;
    proj_coords = proj_coords * 0.5 + 0.5;
    
    float closestDepth = texture(shadow_map_tex, proj_coords.xy).r;
    float currentDepth = proj_coords.z;
    
    float shadow = currentDepth > closestDepth ? 1.0 : 0.5;
    
    
    vec3 n = normalize(vertex_normal);
    //vec3 lp = vec3(0,500,0);
    vec3 ld = normalize(lpos - vertex_pos);
    float diffuse = dot(n,ld);
    
    diffuse = clamp(diffuse, 0, 1);
    
    // Diffuse and ambient lighting
    color.rgb = vec3(1.0,1.0,1.0)*diffuse*0.7*(1-shadow) + vec3(1.0,1.0,1.0)*0.3;

    vec3 cd = normalize(campos - vertex_pos);
    vec3 h = normalize(cd+ld);
    float spec = dot(n,h);
    spec = pow(spec,10);
    
    spec = clamp(spec,0,1);
    
    // Specular lighting
    color.rgb += vec3(1,1,1)*spec*(1-shadow);

    color *= lfactor;
    color.a = 1;
}
