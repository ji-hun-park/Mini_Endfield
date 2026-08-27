#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(push_constant) uniform PushConstants {
    mat4 mvpMatrix;
} push;

layout(location = 0) out vec2 fragUV;

void main() {
    gl_Position = push.mvpMatrix * vec4(inPosition, 1.0);
    
    // Convert from Unity NDC (Z is [-1, 1] or [1, 0]) to Vulkan NDC?
    // Actually Unity's GL.GetGPUProjectionMatrix already converts it properly for the target API!
    
    // In Vulkan, Y is down. But Unity's camera matrix might not have flipped Y if we are not rendering to a texture.
    // If the image is upside down, we can flip it later.
    
    fragUV = inUV;
}
