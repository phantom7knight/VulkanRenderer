#version 450
#extension GL_ARB_separate_shader_objects : enable


vec2 positions[3] = vec2[](
	vec2(0.0,-0.5),
	vec2(0.5,0.5),
	vec2(-0.5,0.5)
	);
	
	
layout(location = 0) in vec3 aPos;

layout(location = 1) in vec3 aNormal;

layout(location = 2) in vec2 aTexCoords;
	
	
void main()
{
	gl_Position = vec4( aPos ,1.0);
}