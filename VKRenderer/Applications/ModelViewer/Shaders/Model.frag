#version 450
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.14159265

layout(location = 0) out 	vec4 OutColor;

layout(location = 0) in 	vec4 vertPos;
layout(location = 1) in		vec2 TexCoords;
layout(location = 2) in		vec3 Normals;

//take the sampler data
layout(binding = 1) uniform sampler2D samplerTexture1;

layout(binding = 2) uniform LightInfoUBO
{
	vec3 lightColor;
	int specularIntensity;
	vec3 lightPosition;
	vec3 camPosition;
}light_ubo;


void CalculatePhong(inout vec4 result)
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



	result = vec4((AmbLight + DiffLight + SpecLight) * objColor, 1.0f);

}

float NormalDistributionFunction(float roughness, float NH)
{
	float result = 0.0f;

	float numerator = (roughness * roughness);

	float denominator = ( (NH * NH) * (numerator - 1) + 1);

	result = numerator / (PI  * denominator * denominator);

	return result;
}

float SubGeometricFunction(float k, vec3 Normal, vec3 inputVector)
{
	float res = 0.0f;

	float NV = dot(normalize(Normal), inputVector);
	
	res = NV / ((NV * (1-k)) + k);

	return res;
}

float GeometricFunction(float roughness, vec3 lightDir, vec3 viewDir, vec3 Normal)
{
	float result = 0.0f;

	float k = ((roughness + 1) * (roughness + 1)) / 8;

	float G1 = SubGeometricFunction(k,Normal,lightDir);

	float G2 = SubGeometricFunction(k,Normal,viewDir);

	result = G1 * G2;

	return result;
}

vec3 FresnelFunction(vec3 viewDir, vec3 HalfwayVec, vec3 SpecularColor)
{
	vec3 result = vec3(1.);

	float VH = dot(viewDir, HalfwayVec);

	float crazyCalc = pow(2, (-5.55473*VH) - (-6.98316*VH));

	result = SpecularColor + (1 - SpecularColor) * crazyCalc;

	return result;
}


//Source : https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
void CalculateBRDF(inout vec4 result)
{
	vec4 diffusePart = vec4(1.);
	vec4 specularPart = vec4(1.);

	vec3 diffAlbedoColor = vec3(0.5,0.5,0.5);
	vec3 specularColor = vec3(0.3,0.4,0.5);

	diffusePart = vec4(diffAlbedoColor, 1.0);
	diffusePart /= PI;

	//Light position
	vec3 lightPos = light_ubo.lightPosition;
	vec3 LightDir = normalize(lightPos - vertPos.xyz);
	
	//View  Position
	vec3 camPos	= light_ubo.camPosition;
	vec3 ViewDir = normalize(camPos - vertPos.xyz);
	

	// H = (l+v) / ||l+v||

	vec3 H = vec3(lightPos + camPos) / length(lightPos + camPos);

	float NH = dot(normalize(Normals), H);

	float roughness = light_ubo.specularIntensity;

	float NDF = NormalDistributionFunction(roughness,NH);

	float GF = GeometricFunction(roughness,LightDir,ViewDir,normalize(Normals));

	vec4 FF = vec4(FresnelFunction(ViewDir,H,specularColor),1.0);

	
	float NL = dot(normalize(Normals), LightDir);

	float NV = dot(normalize(Normals), ViewDir);
	

	specularPart = (( NDF * FF ) / (4 * NL * NV) )* GF;



	result = diffusePart + specularPart;

}

void main()
{
	int num = 1;

	switch(num)
	{
		case 0:
			CalculatePhong(OutColor);
			break;
		case 1:
			CalculateBRDF(OutColor);
			break;
	}



	//vec4 samplerOutput = texture(samplerTexture1, TexCoords);
	//OutColor = samplerOutput;
}