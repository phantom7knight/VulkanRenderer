#version 450
#extension GL_ARB_separate_shader_objects : enable

#define PI 3.14159265

layout(location = 0) out 	vec4 OutColor;

layout(location = 0) in 	vec4 vertPos;
layout(location = 1) in		vec2 TexCoords;
layout(location = 2) in		vec3 Normals;

//take the sampler data
layout(binding = 1) uniform sampler2D albedoTexture;

layout(binding = 2) uniform LightInfoUBO
{
	vec3 lightColor;
	int specularIntensity;
	vec3 lightPosition;
	int lightModel;
	vec3 camPosition;
	int lightIntensity;
}light_ubo;

layout(binding = 3) uniform sampler2D metallicTexture;

layout(binding = 4) uniform sampler2D roughnessTexture;

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
	
	float diffuse = max(dot(normalize(Normals),LightDir),0.0);
	
	vec3 DiffLight =  diffuse * LightColor;

	//Specular Lighting
	vec3 ViewDir = normalize(camPos - vertPos.xyz);
	vec3 reflectDir = reflect(-LightDir,normalize(Normals));
	float spec = pow(max(dot(ViewDir,reflectDir),0.0),64);
	vec3 SpecLight = specIntensity * spec * LightColor;

	vec3 AlbedoSamplerOutput = texture(albedoTexture, TexCoords).rgb;

	result = vec4((AmbLight + DiffLight + SpecLight ) * objColor * AlbedoSamplerOutput, 1.0f);
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
	
	res = NV / ((NV * (1.0-k)) + k);

	return res;
}

float GeometricFunction(float roughness, vec3 lightDir, vec3 viewDir, vec3 Normal)
{
	float result = 0.0f;

	float k = ((roughness + 1.0) * (roughness + 1.0)) / 8.0;

	float G1 = SubGeometricFunction(k,Normal,lightDir);

	float G2 = SubGeometricFunction(k,Normal,viewDir);

	result = G1 * G2;

	return result;
}

vec3 FresnelFunction(vec3 viewDir, vec3 HalfwayVec, vec3 F0)
{
	vec3 result = vec3(1.0);

	float VH = dot(viewDir, HalfwayVec);

	float Fc = pow(1.0 - VH, 5.0);

	result = F0 + (1 - F0) * Fc;

	return result;
}

float NDF(float roughness, float NH)
{
	float numerator = (roughness + 2) * (pow(NH, roughness));
	float denominator = 2 * PI;
	
	return numerator / denominator;
}

float Geometric(float LH)
{
	return (1 / pow(LH, 2));
}

//F (L , H ) = Ks + (1−Ks)(1−L⋅H )^5
vec3 FresnelSchlickApproximation(float LH, vec3 Ks)
{
	return (Ks + (1 - Ks) * (pow(1-LH, 5)));
}

//Source : https://cdn2.unrealengine.com/Resources/files/2013SiggraphPresentationsNotes-26915738.pdf
void CalculateBRDF(inout vec4 result)
{
	int method = 1;
	
	vec3 lightColor = vec3(1000.0,1000.0,1000.0) * vec3(light_ubo.lightIntensity);
	
	vec3 Lo = vec3(0.0);

	// Sample the textures
	vec3 AlbedoSamplerOutput = pow(texture(albedoTexture, TexCoords).rgb, vec3(2.2f));
	float roughness = texture(roughnessTexture, TexCoords).r;
	float metallicness = texture(metallicTexture, TexCoords).r;
	
	//diffusePart = vec4(AlbedoSamplerOutput, 1.0) / PI;

	//Light position & Direction
	vec3 lightPos = light_ubo.lightPosition;
	vec3 LightDir = normalize(lightPos - vertPos.xyz);
	
	//View  Position & Direction
	vec3 camPos	= light_ubo.camPosition;
	vec3 ViewDir = normalize(camPos - vertPos.xyz);
		
	vec3 F0 = mix(vec3(0.04f), AlbedoSamplerOutput, metallicness);

	// Halfway Vector: H = (l+v) / ||l+v||
	vec3 H = normalize(LightDir + ViewDir);
	
	float distance = length(lightPos - vertPos.xyz);
	float attenuation = 1.0f/ (distance * distance);
	vec3 radiance = lightColor * attenuation;

	float NH = max(dot(normalize(Normals), H), 0.0f);
	float NL = max(dot(normalize(Normals), LightDir), 0.0f);
	float NV = max(dot(normalize(Normals), ViewDir), 0.0f);
	float LH = max(dot(LightDir, H), 0.0f);
	
	float N, G;
	vec3 F;

	if(method == 1)
	{
		N = NormalDistributionFunction(roughness,NH);

		G = GeometricFunction(roughness,LightDir,ViewDir,normalize(Normals));

		F = FresnelFunction(ViewDir,H,F0);
	}
	else
	{
		N = NDF(roughness, NH);
		G = Geometric(LH);
		F = FresnelSchlickApproximation(LH,F0);		
	}
	
	vec3 numerator = N * G * F;
	float denominator = 4 * max(NV, 0.0) * max(NL, 0.0) + 0.001;
	vec3 specular = numerator / denominator;
	
	vec3 kS = F;
	vec3 kD = 1.0f - kS;
	
	Lo = (kD * AlbedoSamplerOutput / PI + specular) * radiance * NL;
	
	vec3 ambient = vec3(0.03) * AlbedoSamplerOutput;
	
	result = vec4(ambient + Lo, 1.0f);
	
	// Tonemapping
	result /= (result + vec4(1.0));
	
	// Gamma Correct
	result = pow(result, vec4(1.0/2.2));
	
}

void main()
{
	int num = light_ubo.lightModel;

	switch(num)
	{
		case 0:
			CalculatePhong(OutColor);
			break;
		case 1:
			CalculateBRDF(OutColor);
			break;
	}

	//vec4 samplerOutput = texture(albedoTexture, TexCoords);
	//OutColor = samplerOutput;
}