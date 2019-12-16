
#define PI 3.1415926535

cbuffer uniformBlock2 : register(b0)
{
	float4x4	vp;
    float4x4	modelMatrix;
    float4		color;

	float4		LightColor_DISSAMT;
	
	//Camera + DT
	float4		CameraPosDT;
	
	// Point Light Information
    float4		lightPosition;
	
	
};


struct VSInput
{
    float4 Position : POSITION;
    float4 Normal	: NORMAL;
};

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

VSOutput main(VSInput input)//, uint InstanceID : SV_InstanceID
{
	VSOutput result;

	//Genetate complete MVP matrix
	
	float4x4 mvp_matrix = mul(vp,modelMatrix);
	
	//FragPos
	result.fragPos = mul(modelMatrix,input.Position);
	
	
	//Equivalent to gl_Position in OpenGL
	result.Position = mul(mvp_matrix,input.Position);
	
	//Light Color for PS
	result.lightColor.xyz =  LightColor_DISSAMT.xyz;
	
	//Obj Color for PS
	result.Color = color;
	
	
	//Normals for all the vertices
    result.normals = normalize(mul(modelMatrix, float4(input.Normal.xyz, 0.0f))); // Assume uniform scaling

	//Camera Position for PS
	result.cameraPos = CameraPosDT.xyz;

	//Light Position
	result.lightPos.xyz = lightPosition;
	
	//Generate Texture-Coords for Sphere
	
	float theta = atan2(input.Normal.y,input.Normal.x);
	float phi 	= acos(input.Normal.z);
	
	result.texCoords = float2(theta / (2 * PI) + 0.5 , phi / PI);
	
	//Time
	result.deltaTime = CameraPosDT.w;
	
	//Dissolve Amt
	result.dissolveAmt =  LightColor_DISSAMT.w;
	
	return result;
	
}
