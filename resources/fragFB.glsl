#version 330 core
//layout(location = 0) out float fragmentdepth;
//out float fragmentdepth;
//out vec4 color;

//#version 410 core


//in vec2 fragTex;

//uniform float lfactor;

//uniform sampler2D tex;

void main()
{
    //float other_lfactor = 1 - lfactor;
    // lfactor = 0.5 or 1.0
    // other_l_factor = 0.5 or 0.0
    
    
    //vec3 texturecolor = texture(tex, fragTex,0).rgb;
    //vec3 texturecolorLOWRES = texture(tex, fragTex,6*other_lfactor).rgb;
    //vec3 texturecolorLOWRES = texture(tex, fragTex,4).rgb;
	//texturecolor.r = pow(texturecolor.r, 4*lfactor);
	//texturecolor.g = pow(texturecolor.g, 2*lfactor + 1);
	//texturecolor.b = pow(texturecolor.b, (other_lfactor*1) + 2);
    /*texturecolor.r = pow(texturecolor.r,4);
    texturecolor.g = pow(texturecolor.g,2);
    texturecolor.b = pow(texturecolor.b,1);*/
	//color.rgb = texturecolor+texturecolorLOWRES;
	//color.a=1;
    
    // Not really needed, OpenGL does it anyway
    //fragmentdepth = gl_FragCoord.z;
    
    //Store positions (vertex_pos) not colors. -Christian
}
