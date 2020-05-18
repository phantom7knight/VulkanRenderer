#version 450
#extension GL_ARB_separate_shader_objects : enable


layout(location = 0) out 	vec4 OutColor;

layout(location = 0) in 	vec4 vertPos;
layout(location = 1) in		vec2 TexCoords;
layout(location = 2) in		vec3 Normals;
layout(location = 3) in		vec4 LightSpaceVertPos;

//take the sampler data
layout(binding = 1) uniform sampler2D ShadowMap;

layout(binding = 2) uniform LightInfoUBO
{
	vec3 lightColor;
	int specularIntensity;
	vec3 lightPosition;
	vec3 camPosition;
}light_ubo;

float LinearizeDepth(float depth)
{
  float n = 1.0; // camera z near
  float f = 128.0; // camera z far
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void CalculatePhong(inout vec3 result, float shadowValue)
{
	
	//remove the hard coded values
	vec3 ambLightColor	= vec3(0.8,0.8,0.8);
	vec3 LightColor		= light_ubo.lightColor;
	vec3 objColor		= vec3(0.5,0.5,0.5);
	vec3 camPos			= light_ubo.camPosition;
	float specIntensity = light_ubo.specularIntensity;

	//Simple Lighting
	//===================

	//Ambient Lighting
	vec3 AmbLight = ambLightColor * objColor;

	//Diffuse Lighting
	vec3 lightPos = light_ubo.lightPosition;
	
	vec3 LightDir = normalize(lightPos - vertPos.xyz);
	float diff = max(dot(normalize(Normals),LightDir),0.0);
	vec3 DiffLight =  diff * LightColor;

	//Specular Lighting
	vec3 ViewDir = normalize(camPos - vertPos.xyz);
	vec3 reflectDir = reflect(-LightDir,normalize(Normals));
	float spec = pow(max(dot(ViewDir,reflectDir),0.0),specIntensity);
	vec3 SpecLight = spec * LightColor;


	if(shadowValue == 0.0f)
		result = (AmbLight) * objColor;
	else
		result = (AmbLight + DiffLight + SpecLight) * objColor;


}

float BasicShadowResult(vec4 LightSpaceVertPos)
{
	float result;

	//perspective divide of the LightSpaceVertPos
	vec3 projectedCoords = LightSpaceVertPos.xyz / LightSpaceVertPos.w;

	//bring to [0,1] from [-1,1]
	projectedCoords = projectedCoords * 0.5 + 0.5;

	//
	float lightDepth =  texture(ShadowMap, projectedCoords.xy).r;

	//
	float pixelDepth = projectedCoords.z;

	//check if the pixel is in light or outside

	result = pixelDepth >  lightDepth ? 1.0 : 0.0;

	return result;
}



void main()
{
	
	
	//calculate Shadow
	float shadowResult = BasicShadowResult(LightSpaceVertPos);

	vec3 lightResult;
	CalculatePhong(lightResult,shadowResult);


	OutColor = vec4(vec3(texture(ShadowMap, TexCoords).r),1.0);
	//OutColor = vec4(lightResult, 1.0);

}