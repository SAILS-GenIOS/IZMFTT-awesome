#version 330 core

// Blinn-Phong mesh fragment shader.
// Fixed directional light from upper-right.
// Selection highlight tints with blue.

in vec3 v_worldPos;
in vec3 v_worldNormal;

uniform vec3 u_viewPos;
uniform vec3 u_lightDir;       // Direction TO the light (normalized)
uniform vec3 u_objectColor;
uniform bool u_selected;

out vec4 fragColor;

void main() {
    vec3 normal = normalize(v_worldNormal);
    vec3 lightDir = normalize(u_lightDir);
    vec3 viewDir = normalize(u_viewPos - v_worldPos);

    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * u_objectColor;

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * u_objectColor;

    // Specular (Blinn-Phong)
    float specularStrength = 0.5;
    float shininess = 32.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0);

    vec3 result = ambient + diffuse + specular;

    // Selection highlight: tint with blue
    if (u_selected) {
        result = mix(result, vec3(0.3, 0.5, 1.0), 0.3);
    }

    fragColor = vec4(result, 1.0);
}
