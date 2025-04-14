#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 0) uniform sampler2D depthTexSampler;
layout(set = 2, binding = 0) uniform Ubo {
    mat4 shadowVP;
    vec4 ambientColor;
    vec4 sunDir;
    vec4 sunColor;
} ubo;

void main() {
    vec3 sunDir = normalize(ubo.sunDir.xyz);

    // Shadow stuff
    vec4 shadowPos = ubo.shadowVP * vec4(fragWorldPos, 1.0);
    float shadowDepth = texture(depthTexSampler, (shadowPos.xy + vec2(1.0)) / 2).r;

    // Simple diffuse shader
    vec3 lightColor = ubo.ambientColor.rgb;
    if (shadowPos.z < shadowDepth) 
        lightColor += ubo.sunColor.rgb * max(dot(sunDir, fragNormal), 0);

    outColor = vec4(texture(texSampler, fragTexCoord).rgb * lightColor, 1.0);
}
