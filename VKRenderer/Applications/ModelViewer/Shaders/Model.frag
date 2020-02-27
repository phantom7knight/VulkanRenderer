#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out 	vec4 OutColor;
layout(location = 0) in 	vec3 fragColor;
layout(location = 1) in		vec2 TexCoords;

//take the sampler data
layout(binding = 1) uniform sampler2D samplerTexture1;

void main()
{
	vec4 samplerOutput = texture(samplerTexture1, TexCoords);

	OutColor = samplerOutput;
}