#version 330 core
out vec4 color;

in vec3 vertex_pos;
in vec3 vertex_normal;

uniform vec3 campos;
uniform float lfactor;

void main()
{
    float other_light_factor = 1 - lfactor;

    vec3 n = normalize(vertex_normal);
    vec3 lp = vec3(0,500,0);
    vec3 ld = normalize(lp - vertex_pos);
    //float diffuse = dot(n,ld);
    
    //diffuse = clamp(diffuse, 0, 1);
    
    //vec3 tcol = texture(tex, vertex_tex).rgb;
    
    // Diffuse and ambient lighting
    //color = vec3(1,0,0)*diffuse*0.7 + vec3(1,0,0)*0.3;
    color.rgb = vec3(0.9,0.9,0.9);

    vec3 cd = normalize(campos - vertex_pos);
    vec3 h = normalize(cd+ld);
    float spec = dot(n,h);
    spec = pow(spec,10);
    
    spec = clamp(spec,0,1);
    
    // Specular lighting
    color.rgb += vec3(1,1,1)*spec;
    //color *= other_light_factor;
    
    color.a = 1;
}
