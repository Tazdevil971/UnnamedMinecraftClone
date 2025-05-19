#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in float inSpecStrength;

layout(push_constant) uniform PushConstant { mat4 mvp; }
pushConstant;

void main() { gl_Position = pushConstant.mvp * vec4(inPos.xyz, 1.0); }