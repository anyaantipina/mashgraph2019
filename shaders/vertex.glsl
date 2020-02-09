#version 330

layout(location = 0) in vec3 vertex;
layout (location = 1) in vec2 vertex_texture;
layout (location = 2) in vec3 vertex_color;
layout (location = 3) in vec3 vertex_normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec2 fragm_texture;
out vec3 fragm_color;
out vec3 fragm_normal;
out vec3 fragm_pos;

void main(void) {
    gl_Position  = projection * view * model * vec4(vertex,1.0);
    fragm_color = vertex_color;
    fragm_texture = vertex_texture;
    fragm_normal = mat3(transpose(inverse(model)))*vertex_normal;
    fragm_pos = vec3(model * vec4(vertex, 1.0f));
}
