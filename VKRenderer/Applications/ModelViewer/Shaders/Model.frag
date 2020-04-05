#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out 	vec4 OutColor;

layout(location = 0) in 	vec4 vertPos;
layout(location = 1) in		vec2 TexCoords;
layout(location = 2) in		vec3 Normals;

//take the sampler data
layout(binding = 1) uniform sampler2D samplerTexture1;

void main()
{
	//remove the hard coded values
	vec3 ambLightColor	= vec3(0.8,0.8,0.8);
	vec3 LightColor		= vec3(1.0,1.0,1.0);
	vec3 objColor		= vec3(0.8,0.3,0.2);
	vec3 camPos			= vec3(0.0, 0.0, -10.5);
	float specIntensity = 4;

	vec4 samplerOutput = texture(samplerTexture1, TexCoords);

	//Simple Lighting
	//===================

	//Ambient Lighting
	vec3 AmbLight = ambLightColor * objColor;

	//Diffuse Lighting
	vec3 lightPos = vec3(0.0, -100.0, 0.0);
	
	vec3 LightDir = normalize(lightPos - vertPos.xyz);
	float diff = max(dot(normalize(Normals),LightDir),0.0);
	vec3 DiffLight =  diff * LightColor;

	//Specular Lighting
	vec3 ViewDir = normalize(camPos - vertPos.xyz);
	vec3 reflectDir = reflect(-LightDir,normalize(Normals));
	float spec = pow(max(dot(ViewDir,reflectDir),0.0),specIntensity);
	vec3 SpecLight = spec * LightColor;



	vec4 lightCalcs = vec4((AmbLight + DiffLight + SpecLight) * objColor, 1.0f);

	OutColor = lightCalcs;
	//OutColor = samplerOutput;
}