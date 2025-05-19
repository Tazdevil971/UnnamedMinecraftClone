#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

layout(push_constant) uniform PushConstant { mat4 mvp; }
pushConstant;

void main() {
    gl_Position = pushConstant.mvp * vec4(inPos.xyz, 1.0);
    fragTexCoord = inTexCoord;
}