#version 330 core

in vec3 fragm_color;
in vec2 fragm_texture;
in vec3 fragm_normal;
in vec3 fragm_pos;
out vec4 color;

uniform vec3 camera_pos;
uniform bool is_tex;
uniform bool is_color;


uniform sampler2D fragm_text;
//uniform sampler2D fragm_texture2;

void main() {
    if (is_tex && is_color) {
        color = texture(fragm_text, fragm_texture) * vec4(fragm_color, 1.0);
    }
    else if (is_color)
        color = vec4(fragm_color, 1.0);
    else if (is_tex) {
        vec4 tex_img = texture(fragm_text, fragm_texture);
        if(tex_img.a < 0.2)
            discard;
        color = tex_img;
    }
        
}
