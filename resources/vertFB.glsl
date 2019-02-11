#version 330 core
layout(location = 0) in vec3 vertPos;
//layout(location = 1) in vec2 vertTex;

//#version  410 core

//out vec2 fragTex;

uniform mat4 DepthMVP;
uniform mat4 M;

void main()
{
gl_Position =  DepthMVP * M * vec4(vertPos.xyz, 1);
//gl_Position =  vertPos;
//fragTex = vertTex;
}
