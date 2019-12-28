
struct VSOutput {
	float4 Position 		: SV_POSITION;
	float4 Color 			: COLOR;
    float3 lightColor 		: LIGHTCOLOR;
	float4 normals 			: NORMALS;
	float4 fragPos 			: FRAGPOS;
	float3 cameraPos 		: CAMPOS;
	float3 lightPos 		: LIGHTPOS;
	
};

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
	
	//Toon Shading
	float ndotL = dot(lightDir,normalize(input.normals));
	
	// Hard-transitions 
	//float ToonIntensity = ndotL > 0 ? 1 : 0;

	//Smooth-transitions
	float smoothToonIntensity = smoothstep(0,0.01,ndotL);
	
	float3 light_here = input.lightColor * smoothToonIntensity;
	
	//Rim Lighting
	float RimLightAmt = 0.65;
	float3 rimLightColor = float3(1.0,0.32,0.23);
	float3 rimlight = 1 - dot(input.normals , viewDir);
	float rimIntesity = smoothstep(RimLightAmt - 0.01,RimLightAmt + 0.01 , rimlight);
	float3 rim = rimLightColor * rimIntesity;
	
	
    
	input.Color = float4(light_here + diffuse + ambient + specular + rim, 1.0);
	
	return input.Color;
	
	
	
	
	
}