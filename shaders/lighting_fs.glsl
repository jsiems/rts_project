
#version 330 core

#define NR_POINT_LIGHTS 4

// ***** Structs *****
struct Material {
    //opengl magically knows these texture positions
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;

    float constant;
    float linear;
    float quadratic;

    vec3 diffuse;
    vec3 specular;

    float cutoff;
    float outer_cutoff;

    int on;
};

// ***** Function Declarations *****
// MUST be declared AFTER the structs they are using!!
// also, SHADERS CAN CAUSE SEG FAULTS!!!!
vec3 calculateDirLight(DirLight light, vec3 normal, vec3 view_dir);
vec3 calculatePointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir);

// ***** uniforms *****
uniform Material material;
uniform DirLight dir_light;
uniform PointLight point_lights[NR_POINT_LIGHTS];
uniform SpotLight spot_light;
uniform vec3 view_pos;

// ***** inputs / outputs *****
out vec4 FragColor;
in vec3 normal_vec;
in vec3 frag_pos;
in vec2 tex_coords;

void main() {
    vec3 norm = normalize(normal_vec);
    vec3 view_dir = normalize(view_pos - frag_pos);

    vec3 result = calculateDirLight(dir_light, norm, view_dir);
    for(int i = 0; i < NR_POINT_LIGHTS; i ++) {
        result += calculatePointLight(point_lights[i], norm, frag_pos, view_dir);
    }
    result += calculateSpotLight(spot_light, norm, frag_pos, view_dir);

    FragColor = vec4(result, 1.0);
}

vec3 calculateDirLight(DirLight light, vec3 normal, vec3 view_dir) {
    vec3 light_dir = normalize(-light.direction);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, tex_coords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, tex_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, tex_coords));

    return (ambient + diffuse + specular);
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                               light.quadratic * (distance * distance));
    
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, tex_coords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, tex_coords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, tex_coords));

    return (attenuation * (ambient + diffuse + specular));
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 frag_pos, vec3 view_dir) {
    vec3 light_dir = normalize(light.position - frag_pos);
    float diff = max(dot(normal, light_dir), 0.00);
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, tex_coords));

    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * vec3(texture(material.specular, tex_coords));

    float distance = length(light.position - frag_pos);
    float attenuation = 1.0 / (light.constant + light.linear * distance +
                               light.quadratic * (distance * distance));

    float theta = dot(light_dir, normalize(-light.direction));
    float epsilon = light.cutoff - light.outer_cutoff;
    float intensity = clamp((theta - light.outer_cutoff) / epsilon, 0.0, 1.0);

    //no ambient because it is unaffected by distance
    return (intensity * attenuation * (diffuse + specular)) * light.on;
}
