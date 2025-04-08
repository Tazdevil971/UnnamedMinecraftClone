#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;
layout(location = 3) in vec3 fragWorldPos;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texSampler;

// vec3 lightPos() {
//     return vec3(0.0, 14.0, 0.0);
// }

void main() {
    // float blend = dot(lightPos() - fragWorldPos, fragNormal);

    outColor = vec4(texture(texSampler, fragTexCoord).rgb, 1.0);
}
