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
	float ObjRoughness;
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
	float diff = max(dot(normalize(Normals),LightDir),0.0);
	vec3 DiffLight =  diff * LightColor;

	//Specular Lighting
	vec3 ViewDir = normalize(camPos - vertPos.xyz);
	vec3 reflectDir = reflect(-LightDir,normalize(Normals));
	float spec = pow(max(dot(ViewDir,reflectDir),0.0),specIntensity);
	vec3 SpecLight = spec * LightColor;

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
	int method = 2;
	
	vec4 diffusePart = vec4(1.);
	vec4 specularPart = vec4(1.);

	// Sample the textures
	vec3 AlbedoSamplerOutput = texture(albedoTexture, TexCoords).rgb;
	float roughness = texture(roughnessTexture, TexCoords).r;
	float metallicness = texture(metallicTexture, TexCoords).r;
	
	diffusePart = vec4(AlbedoSamplerOutput, 1.0) / PI;

	//Light position
	vec3 lightPos = light_ubo.lightPosition;
	vec3 LightDir = normalize(lightPos - vertPos.xyz);
	
	//View  Position
	vec3 camPos	= light_ubo.camPosition;
	vec3 ViewDir = normalize(camPos - vertPos.xyz);
		
	vec3 F0 = mix(vec3(0.04f), AlbedoSamplerOutput, metallicness);

	// H = (l+v) / ||l+v||
	vec3 H = normalize(LightDir + ViewDir);

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
	
	
	//float denom_here = (4 * NL * NV)* GF ;
	float denom_here = (4 * NL * NV) ;
	
	if(denom_here <= 0)
		denom_here = 1.0f;
	
	specularPart = vec4(( N * G * F ) / denom_here,1.0f);
	
	vec3 kS = F.rgb;
	
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallicness;

	result = (vec4(kD,1.0f) * diffusePart + specularPart) * NL;

	result += diffusePart * vec4(vec3(0.03f), 1.0f);
	
	// vec3 specular = G * F / (4 * NV * NL);
	// vec3 diffuse = diffusePart.xyz;
	
	// result = vec4(diffuse + specular, 1.0f);
	
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