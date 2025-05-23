#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D dayTexSampler;
layout(set = 1, binding = 0) uniform sampler2D nightTexSampler;
layout(set = 2, binding = 0) uniform Ubo { float blend; }
ubo;

void main() {
    vec3 dayColor = texture(dayTexSampler, fragTexCoord).rgb;
    vec3 nightColor = texture(nightTexSampler, fragTexCoord).rgb;

    outColor = vec4(mix(nightColor, dayColor, ubo.blend), 1.0);
}
