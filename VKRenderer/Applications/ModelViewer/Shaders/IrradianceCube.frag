#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 vertPos;
layout(binding = 0) uniform samplerCube EnvMap;

layout(push_constant) uniform PushConst
{
	layout(offset = 64)	float delta_Phi;
	layout(offset = 68)	float delta_Theta;
}pushConst;

#define PI 3.14159265

void main()
{
	vec3 N = normalize(vertPos);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 right = normalize(cross(up, N));
	up = cross(N, right);
	
	const float TWO_PI = PI * 2.0;
	const float HALF_PI = PI * 0.5;
	
	vec3 color = vec3(0.0f);
	uint sampleCount = 0;
	
	for(float phi = 0.0; phi < TWO_PI; phi += pushConst.delta_Phi)
	{
		for(float theta = 0.0; theta < HALF_PI; theta += pushConst.delta_Theta)
		{
			vec3 tempVec = cos(phi) * right + sin(phi) * up;
			vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
			color += texture(EnvMap, sampleVector).rgb * cos(theta) * sin(theta);
			sampleCount++;
		}		
	}
	
	//outColor = vec4(PI * color / float(sampleCount), 1.0);
	outColor = vec4(1.0,0.0,0.0,1.0);
}