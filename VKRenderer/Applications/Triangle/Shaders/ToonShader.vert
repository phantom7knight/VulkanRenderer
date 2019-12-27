

cbuffer uniformBlock1 : register(b0)
{
	float4x4	vp;
    float4x4	modelMatrix;
    float4		color;

    // Point Light Information
    float3		lightPosition;
    float3		lightColor;
	
	//Camera
	float3		CameraPos;
	

};

//Constant Buffer
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
	
	
};

VSOutput main(VSInput input)
{
	VSOutput result;

	//Genetate complete MVP matrix
	
	float4x4 mvp_matrix = mul(vp,modelMatrix);
	
	//FragPos
	result.fragPos = mul(modelMatrix,input.Position);
	
	
	//Equivalent to gl_Position in OpenGL
	result.Position = mul(mvp_matrix,input.Position);
	
	//Light Color for PS
	result.lightColor = lightColor;
	
	//Obj Color for PS
	result.Color = color;
	
	
	//Normals for all the vertices
    result.normals = normalize(mul(modelMatrix, float4(input.Normal.xyz, 0.0f))); // Assume uniform scaling

	//Camera Position for PS
	result.cameraPos = CameraPos;

	//Light Position
	result.lightPos = lightPosition;
	
	
	return result;
	
}
