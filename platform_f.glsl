#version 330 core

in vec3 fragWorldPos; // Received from the vertex shader
out vec4 FragColor;

uniform float scale; // Size of the checkerboard squares
uniform float offset; // Offset of the checkerboard pattern

void main()
{
    // Apply the checkerboard pattern logic using world position
    int x = int((fragWorldPos.x + offset) * scale) % 2;
    int y = int((fragWorldPos.y + offset) * scale) % 2;
    int z = int((fragWorldPos.z + offset) * scale) % 2;
    
    bool xorXY = (x != y);
    bool checker = (xorXY != (z == 1));

    // Assign color based on checker value
    FragColor = checker ? vec4(0.3, 0.5, 1, 1.0) : vec4(0.05, 0.05, 0.05, 1.0);
}