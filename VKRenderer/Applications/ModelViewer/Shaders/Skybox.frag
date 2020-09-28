#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform samplerCube skyboxCubeTexture;

out vec4 outColor;

vec2 GenerateCubeMap(vec3 )
{
	return 
}

void main()
{
	vec4 result = vec4(0.0f);
	
	vec2 uv = 
	
	vec3 sampleCubeMap = texture(skyboxCubeTexture, uv);
	
	outColor = result;
}