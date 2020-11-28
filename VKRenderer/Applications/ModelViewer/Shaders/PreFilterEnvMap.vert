#version 450

layout(push_constant) uniform PreFilterPushBlock {
	layout (offset = 0) mat4 mvp;
} pushConst;

layout(location = 0) in 	vec3 aPos;
	
layout(location = 1) in 	vec3 aNormal;
	
layout(location = 2) in 	vec2 aTexCoords;

layout(location = 3) in 	vec2 aTangents;

layout(location = 4) in 	vec2 aBiTangents;

layout(location = 0) out 	vec3 vertPos;

void main() 
{
	vertPos = aPos;
	gl_Position = pushConst.mvp * vec4(aPos.xyz, 1.0);
}
