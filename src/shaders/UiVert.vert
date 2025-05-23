#version 450

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;

layout(push_constant) uniform PushConstant {
    vec2 pos;
    vec2 dimension;
    vec2 anchor;
}
pushConstant;

void main() {
    vec2 coord = (inPosition + pushConstant.pos) / pushConstant.dimension +
                 pushConstant.anchor;
    gl_Position = vec4(coord.xy, 0.0, 1.0);
    fragTexCoord = inTexCoord;
}