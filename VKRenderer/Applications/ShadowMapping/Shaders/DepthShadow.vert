#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform DepthCalcUBO
{
	mat4 mvp;
	vec3 lightPos;
}ubo;

layout(location = 0) in 	vec3 aPos;


void main()
{
	
	gl_Position	= ubo.mvp * vec4(aPos, 1.0);

}