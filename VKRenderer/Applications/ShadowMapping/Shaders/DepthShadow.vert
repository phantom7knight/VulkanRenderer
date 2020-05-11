#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(binding = 0) uniform DepthCalcUBO
{
	mat4 mvp;
}ubo;




layout(location = 0) in 	vec3 aPos;


layout(location = 0) out 	vec4 vertPos;
	
void main()
{
	vertPos		= ubo.mvp * vec4(aPos, 1.0);

	gl_Position = vertPos;
}