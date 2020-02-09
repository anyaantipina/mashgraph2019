#version 330 core

in vec3 fragm_color;
in vec2 fragm_texture;
in vec3 fragm_normal;
in vec3 fragm_pos;
out vec4 color;

uniform vec3 lightPos;
uniform vec3 camera_pos;

uniform sampler2D fragm_text;
//uniform sampler2D fragm_texture2;

struct DirectionalLight {
    vec3 direction;
    
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
    float specularIntensity; // for debug purposes, should be set to 1.0
};

struct PointLight {
    vec3 position;
    
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
    float specularIntensity; // for debug purposes, should be set to 1.0
};

struct Material {
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float shininess;
};

Material planeMat = Material (vec4(0.2125,0.1275,0.054,1),
                              vec4(0.714,0.4284,0.18144,1),
                              vec4(0.0,0.0,0.0,1),
                              5.6);
Material bronze = Material(vec4(0.2125,0.1275,0.054,1),
                           vec4(0.714,0.4284,0.18144,1),
                           vec4(0.393548,0.271906,0.166721,1),
                           25.6);
Material brass = Material(vec4(0.329412,0.223529,0.027451,1),
                          vec4(0.780392,0.568627,0.113725,1),
                          vec4(0.992157,0.941176,0.807843,1),
                          27.8974);

vec4 calcDirectionalLight(vec3 normal, vec3 RayDir, DirectionalLight light) {
    vec4 ambient = vec4(light.color, 1) * light.ambientIntensity * bronze.ambient;
    float diffuseFactor = clamp(dot(normal, -light.direction), 0.0, 1.0);
    vec4 diffuse = vec4(light.color, 1) * light.diffuseIntensity * diffuseFactor * bronze.diffuse;
    vec3 lightReflect = normalize(reflect(light.direction, normal));
    float specularFactor = pow(clamp(dot(RayDir, lightReflect), 0.0, 1.0),
                               bronze.shininess);
    vec4 specular = light.specularIntensity * vec4(light.color, 1) * bronze.specular * specularFactor;
    return ambient + diffuse + specular;
}

vec4 calcPointLight(vec3 normal, vec3 RayDir, PointLight light) {
    vec3 lightDirection = normalize(fragm_pos - light.position);
    float dist = length(fragm_pos - light.position);
    float pointFactor = 1.0 / pow(dist, 2.);
    
    DirectionalLight directionalLight;
    directionalLight.direction = lightDirection;
    directionalLight.color = light.color;
    directionalLight.ambientIntensity = light.ambientIntensity;
    directionalLight.diffuseIntensity = light.diffuseIntensity;
    directionalLight.specularIntensity = light.specularIntensity;
    
    return pointFactor * calcDirectionalLight(normal, RayDir, directionalLight);
}

void main() {
    PointLight pointlight;
    pointlight.position = lightPos;
    pointlight.color = vec3(1.0,1.0,1.0);
    pointlight.ambientIntensity = 45.8;
    pointlight.diffuseIntensity = 45.8;
    pointlight.specularIntensity = 5.0;
    vec3 normal = normalize(fragm_normal);
    vec3 RayDir = normalize(camera_pos - fragm_pos);
    vec4 light = calcPointLight(normal, RayDir, pointlight);
    color = texture(fragm_text, fragm_texture);
}
