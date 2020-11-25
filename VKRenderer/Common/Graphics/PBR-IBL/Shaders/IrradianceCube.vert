#version 450

layout(push_constant) uniform PushConsts {
	layout (offset = 0) mat4 mvp;
} pushConsts;

layout (location = 0) out vec3 outUVW;

layout(location = 0) in 	vec3 aPos;
	
layout(location = 1) in 	vec3 aNormal;
	
layout(location = 2) in 	vec2 aTexCoords;

layout(location = 3) in 	vec2 aTangents;

layout(location = 4) in 	vec2 aBiTangents;

layout(location = 0) out 	vec4 vertPos;

// out gl_PerVertex {
	// vec4 gl_Position;
// };

void main() 
{
	vertPos = aPos;
	gl_Position = pushConsts.mvp * vec4(aPos.xyz, 1.0);
}
