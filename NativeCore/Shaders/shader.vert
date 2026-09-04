#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

// Per-instance MVP matrix passed via binding 1 (VK_VERTEX_INPUT_RATE_INSTANCE)
// In GLSL, mat4 automatically consumes 4 attribute locations: 2, 3, 4, 5
layout(location = 2) in mat4 inMvpMatrix;

layout(location = 0) out vec2 fragUV;

void main() {
    gl_Position = inMvpMatrix * vec4(inPosition, 1.0);
    fragUV = inUV;
}
