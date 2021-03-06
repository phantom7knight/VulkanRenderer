#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform ModelUBO
{
	mat4 ModelMatrix;
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
}ubo;




layout(location = 0) in 	vec3 aPos;
	
layout(location = 1) in 	vec3 aNormal;
	
layout(location = 2) in 	vec2 aTexCoords;

layout(location = 3) in 	vec2 aTangents;

layout(location = 4) in 	vec2 aBiTangents;



layout(location = 0) out 	vec4 vertPos;

layout(location = 1) out	vec2 TexCoords;

layout(location = 2) out	vec3 Normals;
	
void main()
{
	vertPos		= ubo.ModelMatrix * vec4(aPos, 1.0);
	TexCoords	= aTexCoords;
	Normals		= mat3(transpose(inverse(ubo.ModelMatrix))) * aNormal;
	gl_Position = ubo.ProjectionMatrix * ubo.ViewMatrix * vec4(vertPos.xyz,1.0f);
}