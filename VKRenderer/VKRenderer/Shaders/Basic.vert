#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform UniformBufferObject
{
	mat4 ModelMatrix;
	mat4 ViewMatrix;
	mat4 ProjectionMatrix;
}ubo;




layout(location = 0) in 	vec3 aPos;
	
layout(location = 1) in 	vec3 aNormal;
	
layout(location = 2) in 	vec2 aTexCoords;

layout(location = 0) out 	vec3 fragColor;
	
void main()
{
	gl_Position = ubo.ProjectionMatrix * ubo.ViewMatrix * ubo.ModelMatrix * vec4( aPos, 1.0);
	fragColor = vec3(1.0,0.5,0.222);
}