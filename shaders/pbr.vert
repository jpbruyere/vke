#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
//instance
layout (location = 3) in int  inMatId;
layout (location = 4) in mat4 inModel;
layout (location = 8) in vec4 inColor;

layout (binding = 0) uniform UBO
{
	mat4 projection;
	mat4 model;
	mat4 view;
	vec3 camPos;
} ubo;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec2 outUV;
layout (location = 3) out int outMatId;
layout (location = 4) out vec4 outColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	vec3 locPos = vec3(inModel * vec4(inPos, 1.0));
	outWorldPos = locPos;
	outNormal = mat3(inModel) * inNormal;
	outColor = inColor;
	outUV = inUV;
	//outUV.t = 1.0 - inUV.t;
	outMatId = inMatId;
	gl_Position =  ubo.projection * ubo.view * vec4(outWorldPos, 1.0);
}
