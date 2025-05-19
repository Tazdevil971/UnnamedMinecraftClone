#version 450

layout(location = 0) in vec3 fragNormal;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragWorldPos;
layout(location = 3) in float fragSpecStrength;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D texSampler;
layout(set = 1, binding = 0) uniform sampler2D depthTexSampler;
layout(set = 2, binding = 0) uniform Ubo {
    mat4 shadowVP;
    vec4 ambientColor;
    vec4 sunDir;
    vec4 sunColor;
    vec4 viewPos;
}
ubo;

void main() {
    vec3 sunDir = normalize(ubo.sunDir.xyz);
    vec3 normal = normalize(fragNormal);
    vec3 viewPos = ubo.viewPos.xyz;

    // Shadow stuff
    vec4 shadowPos = ubo.shadowVP * vec4(fragWorldPos, 1.0);
    float shadowDepth =
        texture(depthTexSampler, (shadowPos.xy + vec2(1.0)) / 2).r;

    // Always add ambient color
    vec3 lightColor = ubo.ambientColor.rgb;
    if (shadowPos.z < (shadowDepth + 0.0001)) {
        // If we are not in shadow, compute diffuse component
        lightColor += ubo.sunColor.rgb * max(dot(sunDir, normal), 0);

        // Optional specular component
        vec3 viewDir = normalize(viewPos - fragWorldPos);
        vec3 reflectDir = reflect(-sunDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0), 256);

        lightColor += ubo.sunColor.rgb * spec * fragSpecStrength;
    }

    outColor = vec4(lightColor * texture(texSampler, fragTexCoord).rgb, 1.0);
}
