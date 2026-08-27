#version 450
layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    // Simple UV visualization or just a solid color to verify mesh bounds
    outColor = vec4(fragUV, 1.0, 1.0);
}
