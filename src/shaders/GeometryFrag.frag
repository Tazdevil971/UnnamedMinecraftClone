#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 0) uniform Ubo {
    vec4 ambientColor;
    vec4 sunDir;
    vec4 sunColor;
} ubo;

void main() {
    vec3 sunDir = normalize(ubo.sunDir.xyz);

    // Simple diffuse shader
    vec3 lightColor = ubo.sunColor.rgb * max(dot(sunDir, fragNormal), 0) + ubo.ambientColor.rgb;

    outColor = vec4(texture(texSampler, fragTexCoord).rgb * lightColor, 1.0);
}
