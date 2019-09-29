

struct VSOutput {
	float4 Position 		: SV_POSITION;
	float4 Color 			: COLOR;
    float3 lightColor 		: LIGHTCOLOR;
	float4 normals 			: NORMALS;
	float4 fragPos 			: FRAGPOS;
	float3 cameraPos 		: CAMPOS;
	float3 lightPos 		: LIGHTPOS;
	float2 texCoords		: TEX_COORDS;
	float  deltaTime		: TIME;
	float  dissolveAmt		: DISSOLVEAMT;
	
};


SamplerState 	NoiseSampler 	: register(s1);
Texture2D 		NoiseTex1 		: register(t2);


float4 main(VSOutput input) : SV_TARGET
{
    
	//===================================================
	//Light Calculations
	//===================================================
	
	float lightIntensity = 1.0f;
    float quadraticCoeff = 1.2;
    float ambientCoeff = 0.4;
	int speculatIntensity = 256;

    float3 lightDir;

    if (input.Color.w == 0) // Special case for Sun, so that it is lit from its top
        lightDir = float3(0.0f, 1.0f, 0.0f);
    else
        lightDir = normalize(input.lightPos - input.fragPos.xyz);

    float distance = length(lightDir);
    float attenuation = 1.0 / (quadraticCoeff * distance * distance);
    float intensity = lightIntensity * attenuation;
    
	//Diffuse + Ambient
    float3 baseColor = input.Color.xyz;
    float3 blendedColor = mul(input.lightColor * baseColor, lightIntensity);
	
	//Diffuse light calculation
    float3 diffuse = mul(blendedColor, max(dot(input.normals.xyz, lightDir), 0.0));
   
   //Ambient light calculation
	float3 ambient = mul(baseColor, ambientCoeff);
	
	//Specular light calculation
	float3 viewDir = normalize(input.cameraPos - input.fragPos.xyz);
    float3 H = normalize(lightDir + viewDir);
	float NdotH = dot(input.normals,H);
	float  specValue = pow(saturate(NdotH),speculatIntensity);
	float specularsmooth = smoothstep(0.005,0.01,specValue);
	float3 specular = input.lightColor * specularsmooth;
	
	
	input.Color = float4(diffuse + ambient + specular, 1.0);
	
	
	
	//Dematerializing Part
	
	//sample noise texture
	float Samplednoise  = NoiseTex1.Sample(NoiseSampler,input.texCoords).r;
	
	
	clip(Samplednoise - (input.dissolveAmt * input.deltaTime));
	
	
	return input.Color;
	
	
	
	
	
}