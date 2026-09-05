#version 450
layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    // Screen-space derivative surface normal
    vec3 dX = dFdx(vec3(gl_FragCoord.xy, gl_FragCoord.z));
    vec3 dY = dFdy(vec3(gl_FragCoord.xy, gl_FragCoord.z));
    vec3 N = normalize(cross(dX, dY));

    // Directional light from front-top
    vec3 L = normalize(vec3(0.3, 0.8, 0.5));
    float NdotL = dot(N, L);
    // Half-lambert diffuse (Arknights / Anime cell shading style)
    float halfLambert = clamp(NdotL * 0.5 + 0.5, 0.0, 1.0);
    // Smooth 2-band cell step
    float cel = smoothstep(0.4, 0.55, halfLambert) * 0.4 + 0.6;

    // Pleasant stylized character palette based on UV
    vec3 baseCol = mix(vec3(0.5, 0.55, 0.65), vec3(0.7, 0.75, 0.85), fragUV.y);
    baseCol = mix(baseCol, vec3(0.85, 0.7, 0.6), fragUV.x * 0.3);

    outColor = vec4(baseCol * cel, 1.0);
}
