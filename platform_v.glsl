#version 330 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;

out vec3 fragWorldPos; // This will be passed to the fragment shader
out vec3 fragWorldNor;

void main(void)
{
    vec4 worldPos = modelingMatrix * vec4(inVertex, 1);
    fragWorldPos = worldPos.xyz; // Pass the world position as a vec3
    fragWorldNor = inverse(transpose(mat3x3(modelingMatrix))) * inNormal;

    gl_Position = projectionMatrix * viewingMatrix * worldPos;
}