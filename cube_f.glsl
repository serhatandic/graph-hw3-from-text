#version 330 core

// Uniforms and variables
uniform vec3 eyePos;
uniform vec3 lightPos; // Moved light position to uniform for flexibility
uniform int checkpointType; // 0 for yellow, 1 for red

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 FragColor;

void main(void)
{
    // Define light and material properties
    vec3 I = vec3(1, 1, 1);          // point light intensity
    vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
    vec3 kd;                         // diffuse reflectance coefficient
    vec3 ka = vec3(0.3, 0.3, 0.3);   // ambient reflectance coefficient
    vec3 ks = vec3(0.8, 0.8, 0.8);   // specular reflectance coefficient

    // Set checkpoint color based on type
    if (checkpointType == 1) {
        kd = vec3(1, 1, 0); // Yellow
    } else {
        kd = vec3(1, 0, 0); // Red
    }

    // Compute lighting
    vec3 L = normalize(lightPos - vec3(fragWorldPos));
    vec3 V = normalize(eyePos - vec3(fragWorldPos));
    vec3 H = normalize(L + V);
    vec3 N = normalize(fragWorldNor);

    float NdotL = dot(N, L); // for diffuse component
    float NdotH = dot(N, H); // for specular component

    vec3 diffuseColor = I * kd * max(0, NdotL);
    vec3 specularColor = I * ks * pow(max(0, NdotH), 100);
    vec3 ambientColor = Iamb * ka;

    // Calculate final color
    FragColor = vec4(diffuseColor + specularColor + ambientColor, 1);
}
