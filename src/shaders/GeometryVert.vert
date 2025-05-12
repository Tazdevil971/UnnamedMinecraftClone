#version 450

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragTangent;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragWorldPos;

layout(push_constant) uniform PushConstant {
    mat4 m;
    mat4 vp;
} pushConstant;

void main() {
    vec4 worldPos = pushConstant.m * vec4(inPos.xyz, 1.0);
    gl_Position = pushConstant.vp * worldPos;

    mat4 n = pushConstant.m;
    // Zero out translation component
    n[3] = vec4(0.0, 0.0, 0.0, 1.0);

    fragNormal = (n * vec4(inNormal.xyz, 1.0)).xyz;
    fragTangent = (n * vec4(inTangent.xyz, 1.0)).xyz;
    fragTexCoord = inTexCoord;
    fragWorldPos = worldPos.xyz;
}