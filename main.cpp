//internal includes
#include "common.h"
#include "ShaderProgram.h"
//#include <mach-o/dyld.h> //FOR MACOS

//External dependencies
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <random>
#include <math.h>
#include <unistd.h> //FOR LINUX

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define wid(x) (x)
#define hei(x) (1.0f - (x))

static const GLsizei WIDTH = 640, HEIGHT = 480; //размеры окна

int initGL() {
    int res = 0;
	//грузим функции opengl через glad
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
		return -1;
	}

	std::cout << "Vendor: "   << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: "  << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: "     << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	return 0;
}

float random(float min, float max) {
    return (float)(-1 + rand())/(RAND_MAX)*(max-min) + min;
}


glm::vec3 camera_pos   = glm::vec3(0.0f, 0.0f,  2.65f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up    = glm::vec3(0.0f, 1.0f,  0.0f);

bool firstMouse = true;

float aim_coord_x = 0.0;
float aim_coord_y = 0.0;
GLfloat lastX = 0.0;
GLfloat lastY = 0.0;
bool keys[1024];
GLfloat delta = 0.0f;
GLfloat lastFrame = 0.0f;

std::vector<GLuint> all_VAOs;
std::vector<GLuint> all_VBOs;

void cursor_callback(GLFWwindow* window, double xpos, double ypos){
    float x, y;
    float y_offset = float(HEIGHT) - ypos;
    if ((xpos >= 0) && (xpos <= WIDTH) && (ypos >= 0) && (ypos <= HEIGHT)) {
        aim_coord_x = (xpos - (WIDTH/2.0))/(WIDTH/2.0);
        aim_coord_y = (y_offset - (HEIGHT/2.0))/(HEIGHT/2.0);
    }
    else {
        aim_coord_x = lastX;
        aim_coord_y = lastY;
    }
    
    lastX = aim_coord_x;
    lastY = aim_coord_y;
}

bool shooting = false;
void mouse_callback(GLFWwindow* window, int but, int act, int mod){
    if (act == GLFW_PRESS) {
        shooting = true;
    }
}

enum Type {TRIANGLE, SQUARE, UNKNOWN};

struct Object {
    float square[44];
    float triangle[33];
    void generate_square(glm::vec3[4], glm::vec2[4], glm::vec3[4], glm::vec3[1]);
    void generate_triangle(glm::vec3[3], glm::vec2[3], glm::vec3[3], glm::vec3[1]);
    GLuint generate_obj(Type, bool is_tex, bool is_clr, bool is_norm);
};

struct Damage_str {
    int damage_src;
    glm::vec3 damage_pos;
};

struct Player {
    float health;
    int score;
    bool game_over;
    bool update_health;
    bool update_score;
    int first_h;
    int second_h;
    int third_h;
    int weapon_type;
    float damage_time;
    std::vector<Damage_str> damages;
    std::vector<int> numbers_sc;
    Object obj;
    GLuint VAO;
    Player(): weapon_type(0), health(100.0), score(0), update_score(0), update_health(0), game_over(0),
    first_h(1), second_h(0), third_h(0), numbers_sc({0}), damage_time(1.0){};
    void generate_obj();
    void health_loss(float);
    void score_up(int);
    void draw(ShaderProgram, GLuint, GLuint, GLuint[10],
              GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint, GLuint);
    void draw_game_over(ShaderProgram, GLuint, GLuint, GLuint, GLuint, GLuint[10]);
};

Player player;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
    if(key == GLFW_KEY_Q && action == GLFW_PRESS)
        player.weapon_type = (player.weapon_type + 1) % 3;
}
void do_movement()
{
    GLfloat camera_speed = 5.0f * delta;
    if(keys[GLFW_KEY_UP])
        camera_pos += camera_speed * camera_front;
    if(keys[GLFW_KEY_DOWN])
        camera_pos -= camera_speed * camera_front;
    if(keys[GLFW_KEY_W])
        camera_pos += camera_speed * camera_up;
    if(keys[GLFW_KEY_S])
        camera_pos -= camera_speed * camera_up;
    if(keys[GLFW_KEY_A])
        camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
    if(keys[GLFW_KEY_D])
        camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
}

struct Textures{
    GLuint front;
    GLuint up;
    GLuint down;
    GLuint right;
    GLuint left;
    GLuint enemy1;
    GLuint enemy2;
    GLuint enemy3;
    GLuint bang_enemy1;
    GLuint bang_enemy2;
    GLuint bang_enemy3;
    GLuint asteroid1;
    GLuint asteroid2;
    GLuint damage;
    GLuint damage_enemy;
    GLuint damage_asteroid;
    GLuint aim;
    GLuint shot1;
    GLuint shot2;
    GLuint shot3;
    GLuint dang_shot1;
    GLuint dang_shot2;
    GLuint dang_shot3;
    GLuint particle;
    GLuint dust;
    GLuint health;
    GLuint score;
    GLuint weapon;
    GLuint zero;
    GLuint one;
    GLuint two;
    GLuint three;
    GLuint four;
    GLuint five;
    GLuint six;
    GLuint seven;
    GLuint eight;
    GLuint nine;
    GLuint game_over;
    GLuint score_title;
    GLuint sad_cat;
    int load_textures();
};

struct Particle {
    Object obj;
    GLuint VAO;
    GLuint texture;
    glm::vec3 start_pos;
    glm::vec3 curr_pos;
    glm::vec3 end_pos;
    glm::vec3 direction;
    bool not_visible;
    float speed;
    float size;
    Particle(): texture(0), start_pos(glm::vec3(0.0,0.0,0.0)),
                            curr_pos(glm::vec3(0.0,0.0,0.0)),
                            end_pos(glm::vec3(0.0,0.0,0.0)),
                            direction(glm::vec3(0.0,0.0,0.0)),
                            size(1.0), speed(0.07), not_visible(0) {};
    void generate_obj(GLuint, glm::vec3);
    void check_alive();
    void draw(ShaderProgram);
};


struct Asteroid{
    std::vector<GLuint> VAOs;
    std::vector<Object> objs;
    Type type;
    GLuint texture;
    glm::vec3 start_pos;
    glm::vec3 curr_pos;
    std::vector<glm::vec3> particles_pos;
    glm::vec3 end_pos;
    glm::vec3 direction;
    float speed;
    std::vector<float> particles_speed;
    std::vector<glm::vec3> particle_dirs;
    bool killed;
    bool bump;
    bool not_visible;
    float size;
    float radius;
    Asteroid(): type(UNKNOWN), texture(0), start_pos(glm::vec3(0.0,0.0,0.0)),
                                            curr_pos(glm::vec3(0.0,0.0,0.0)),
                                            end_pos(glm::vec3(0.0,0.0,0.0)),
                                            direction(glm::vec3(0.0,0.0,0.0)),
                                            killed(0), size(1.0), radius(1.0),
                                            not_visible(0), speed(0.07), bump(0){};
    void generate_obj(Type, GLuint);
    void check_alive();
    void kill();
    void draw(ShaderProgram);
};

struct DangerShot{
    GLuint VAO;
    Object obj;
    GLuint texture;
    glm::vec3 start_pos;
    glm::vec3 curr_pos;
    glm::vec3 direction;
    bool shooted;
    DangerShot(): texture(0), start_pos(glm::vec3(0.0,0.0,0.0)),
    curr_pos(glm::vec3(0.0,0.0,0.0)),
    direction(glm::vec3(0.0,0.0,0.0)), shooted(1) {};
    void generate_obj(GLuint, glm::vec3);
    void new_shot(glm::vec3);
    bool check_hit();
    void draw(ShaderProgram);
};

struct Enemy{
    GLuint VAO;
    Object obj;
    GLuint texture;
    GLuint shot_text;
    GLuint bang_text;
    float start_bang;
    glm::vec3 start_pos;
    glm::vec3 curr_pos;
    glm::vec3 end_pos;
    glm::vec3 direction;
    float speed;
    bool killed;
    bool bang;
    bool bump;
    float size;
    float size_bang;
    float radius;
    std::vector<DangerShot> dang_shots;
    Enemy(): texture(0), start_pos(glm::vec3(0.0,0.0,0.0)),
                                        curr_pos(glm::vec3(0.0,0.0,0.0)),
                                        end_pos(glm::vec3(0.0,0.0,0.0)),
                                        direction(glm::vec3(0.0,0.0,0.0)),
                                        killed(0), size(1.0), speed(0.05),
                                        radius(0.0), shot_text(0), bang(0), bump(0) {};
    void generate_obj(GLuint, GLuint, GLuint, float);
    void check_alive();
    void generate_shot();
    void kill();
    void draw(ShaderProgram);
};

struct Shot{
    GLuint VAO;
    Object obj;
    glm::vec3 start_pos;
    glm::vec3 curr_pos;
    glm::vec3 direction;
    bool shooted;
    Shot(): start_pos(glm::vec3(0.0,0.0,0.0)),
        curr_pos(glm::vec3(0.0,0.0,0.0)),
        direction(glm::vec3(0.0,0.0,0.0)), shooted(1) {};
    void generate_obj();
    void new_shot();
    bool check_hit();
    void draw(ShaderProgram, GLuint, GLuint, GLuint);
};

std::vector<Enemy> enemies;
std::vector<Asteroid> asters;

struct SkyBox {
    std::vector<Object> objs;
    std::vector<GLuint> VAOs;
    SkyBox(){};
    void generate_cube();
    void draw(ShaderProgram, GLuint[5]);
    
};

//|||||||||||||||COORDINATS|||||||||||||||NORMALS|||||||||||||||

glm::vec3 square_dot[] = {
    glm::vec3(0.5f, -0.5f, 1.0f),
    glm::vec3(0.5f, 0.5f, 1.0f),
    glm::vec3(-0.5f, 0.5f, 1.0f),
    glm::vec3(-0.5f, -0.5f, 1.0f)
};

glm::vec3 square_normals[] = {
    glm::vec3(0.0, 0.0, 1.0) //0
};

//        1
//       / \
//     /     \
//  2/_________\0

glm::vec3 triangle1_dot[] = {
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(-1.0f, 0.0f, 0.0f)
};

//         1
//        //
//      / /
//    /  /
//  2---0
glm::vec3 triangle2_dot[] = {
    glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(-1.0f, -1.0f, 0.0f)
};

//      1
//     / |
//    /  |
//   /   |
//  2----0
glm::vec3 triangle3_dot[] = {
    glm::vec3(1/2.0f, -1.0f, 0.0f),
    glm::vec3(1/2.0f, 1.0f, 0.0f),
    glm::vec3(-1/2.0f, -1.0f, 0.0f)
};


//   1
//   \\
//    \ \
//     \  \
//     2---0
glm::vec3 triangle4_dot[] = {
    glm::vec3(1.0f, -1.0f, 0.0f),
    glm::vec3(-1.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
};

//  1
//  |\
//  | \
//  |  \
//  2----0
glm::vec3 triangle5_dot[] = {
    glm::vec3(1/2.0f, -1.0f, 0.0f),
    glm::vec3(-1/2.0f, 1.0f, 0.0f),
    glm::vec3(-1/2.0f, -1.0f, 0.0f)
};

glm::vec3 triangle_normals[] = {
    glm::vec3(0.0, 0.0, 1.0) //0
};

//|||||||||||||||PlAYER|||||||||||||||PLAYER|||||||||||||||PLAYER|||||||||||||||

void Player::generate_obj(){
    glm::vec2 square_tex_coord[] = {
        glm::vec2(wid(1.0f), hei(0.0f)), //0
        glm::vec2(wid(1.0f), hei(1.0f)), //1
        glm::vec2(wid(0.0f), hei(1.0f)), //2
        glm::vec2(wid(0.0f), hei(0.0f)) //3
        
    };
    glm::vec3 color_coord[] = {
        glm::vec3(0.0f, 0.0f, 0.0f), //0
        glm::vec3(0.0f, 0.0f, 0.0f), //1
        glm::vec3(0.0f, 0.0f, 0.0f), //2
        glm::vec3(0.0f, 0.0f, 0.0f) //3
    };
    obj.generate_square(square_dot, square_tex_coord, color_coord, square_normals);
    VAO = obj.generate_obj(SQUARE, true, false, true);
}

void Player::health_loss(float damage) {
    health -= damage;
    if (health <= 0)
        game_over = true;
    else {
        update_health = true;
    }
}

void Player::score_up(int profit) {
    update_score = true;
    score += profit;
}

void Player::draw(ShaderProgram shader, GLuint health_tex,
                  GLuint score_tex, GLuint num[10],
                  GLuint game_over_tex, GLuint score_title,
                  GLuint sad_cat, GLuint damage_tex, GLuint damage_enemy,
                  GLuint damage_asteroid, GLuint weapon_tex, GLuint shot1, GLuint shot2, GLuint shot3){
    if (!game_over) {
    glBindVertexArray(VAO); GL_CHECK_ERRORS
        glDisable(GL_BLEND); GL_CHECK_ERRORS
    //health
    glm::mat4 model(1.0f);
    glm::vec3 offset = glm::vec3(camera_pos.x-7/8.0,camera_pos.y + 7/8.0,camera_pos.z-2.55);
    model = glm::translate(model, offset);
    float size = 0.225;
    model = glm::scale(model, glm::vec3(size,size,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, health_tex); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    
    //score
    model = glm::mat4(1.0f);
    offset = glm::vec3(camera_pos.x-7/8.0,camera_pos.y+5/8.0,camera_pos.z-2.55);
    model = glm::translate(model, offset);
    size = 0.2;
    model = glm::scale(model, glm::vec3(size,size,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, score_tex); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
        
        //weapon
        model = glm::mat4(1.0f);
        offset = glm::vec3(camera_pos.x-0.83,camera_pos.y + 3/8.0,camera_pos.z-2.55);
        model = glm::translate(model, offset);
        size = 0.225;
        model = glm::scale(model, glm::vec3(size,size,size));
        shader.SetUniform("model", model); GL_CHECK_ERRORS
        glBindTexture(GL_TEXTURE_2D, weapon_tex); GL_CHECK_ERRORS
        shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
        
        //weapon type
        model = glm::mat4(1.0f);
        offset = glm::vec3(camera_pos.x-0.63,camera_pos.y + 3/8.0,camera_pos.z-2.55);
        model = glm::translate(model, offset);
        size = 0.225;
        model = glm::scale(model, glm::vec3(size,size,size));
        shader.SetUniform("model", model); GL_CHECK_ERRORS
        switch (weapon_type) {
            case 0:
                glBindTexture(GL_TEXTURE_2D, shot1); GL_CHECK_ERRORS
                break;
            case 1:
                glBindTexture(GL_TEXTURE_2D, shot2); GL_CHECK_ERRORS
                break;
            case 2:
                glBindTexture(GL_TEXTURE_2D, shot3); GL_CHECK_ERRORS
                break;
        }
        shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
        
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS

    
    if (update_health) {
        int h = round(this->health);
        first_h = h / 100;
        second_h = (h % 100) / 10;
        third_h = h % 10;
        update_health = false;
        damage_time = 0.0;
    }
        
        
    size = 0.2;
    model = glm::mat4(1.0f);
    offset = glm::vec3(camera_pos.x-0.7,camera_pos.y+7/8.0,camera_pos.z-2.55);
    model = glm::translate(model, offset);
    model = glm::scale(model, glm::vec3(size*0.7,size,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, num[first_h]); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    
    model = glm::mat4(1.0f);
    offset = glm::vec3(camera_pos.x-0.57,camera_pos.y+7/8.0,camera_pos.z-2.55);
    model = glm::translate(model, offset);
    model = glm::scale(model, glm::vec3(size*0.7,size,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, num[second_h]); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    
    model = glm::mat4(1.0f);
    offset = glm::vec3(camera_pos.x-0.44,camera_pos.y+7/8.0,camera_pos.z-2.55);
    model = glm::translate(model, offset);
    model = glm::scale(model, glm::vec3(size*0.7,size,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, num[third_h]); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    
        
    if (update_score && !game_over) {
        int s = score;
        numbers_sc.clear();
        while (s >= 10) {
            int number = s % 10;
            numbers_sc.insert(numbers_sc.begin(), number);
            s = s / 10;
        }
        numbers_sc.insert(numbers_sc.begin(),s);
        update_score = false;
    }
    float index = 0.13;
    offset = glm::vec3(camera_pos.x-0.7,camera_pos.y+5/8.0,camera_pos.z-2.55);
    for (auto it : numbers_sc) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, offset);
        model = glm::scale(model, glm::vec3(size*0.7,size,size));
        shader.SetUniform("model", model); GL_CHECK_ERRORS
        glBindTexture(GL_TEXTURE_2D, num[it]); GL_CHECK_ERRORS
        shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
        offset.x += index;
    }
    glEnable(GL_BLEND); GL_CHECK_ERRORS
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); GL_CHECK_ERRORS
        if (damages.size() != 0){
            for (auto it =  damages.begin(); it != damages.end(); it++) {
                if (it->damage_src == 1) {
                    glBindTexture(GL_TEXTURE_2D, damage_enemy); GL_CHECK_ERRORS
                    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
                    size = 0.8;
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, damage_asteroid); GL_CHECK_ERRORS
                    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
                    size = 0.8;
                }
                model = glm::mat4(1.0f);
                model = glm::translate(model, camera_pos - it->damage_pos);
                model = glm::scale(model, glm::vec3(size,size,size));
                shader.SetUniform("model", model); GL_CHECK_ERRORS
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
            }
        }
        if ((damage_time >= 0.05) && (damage_time < 0.5)) {
            model = glm::mat4(1.0f);
            offset = glm::vec3(camera_pos.x+0.0,camera_pos.y-0.1,camera_pos.z-2.65);
            model = glm::translate(model, offset);
            size = 1.6;
            model = glm::scale(model, glm::vec3(size,size,size));
            shader.SetUniform("model", model); GL_CHECK_ERRORS
            glBindTexture(GL_TEXTURE_2D, damage_tex); GL_CHECK_ERRORS
            shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
            damage_time+=0.01;
        }
        if (damage_time < 0.05) {
            damage_time+=0.01;
        }
        glBindVertexArray(0); GL_CHECK_ERRORS;
    }
    else {
        draw_game_over(shader, game_over_tex, score_title, sad_cat, score_tex, num);
    }
}

void Player::draw_game_over(ShaderProgram shader, GLuint game_over_tex, GLuint sad_cat,
                            GLuint score_title, GLuint pizza_tex, GLuint num[10]){
    glDisable(GL_BLEND); GL_CHECK_ERRORS
    shader.SetUniform("is_tex", true); GL_CHECK_ERRORS
    shader.SetUniform("is_color", false); GL_CHECK_ERRORS
    glBindVertexArray(VAO); GL_CHECK_ERRORS
    glm::mat4 model(1.0f);
    glm::vec3 offset = glm::vec3(camera_pos.x,camera_pos.y+0.5,camera_pos.z-2.65);
    model = glm::translate(model, offset);
    float size = 1.2;
    model = glm::scale(model, glm::vec3(size,size*0.2,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, game_over_tex); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    
    size = 0.5;
    model = glm::mat4(1.0f);
    offset = glm::vec3(camera_pos.x,camera_pos.y+0.3,camera_pos.z-2.65);
    model = glm::translate(model, offset);
    model = glm::scale(model, glm::vec3(size,size,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, sad_cat); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    
    float config_offset = 0.1;
    int score_size = numbers_sc.size();
    if ((score_size % 2) == 0)
        offset = glm::vec3(-0.19*(score_size/2) + config_offset + camera_pos.x,camera_pos.y-0.15,camera_pos.z-2.65);
    else
        offset = glm::vec3(-0.19*(score_size/2)-0.095+ config_offset + camera_pos.x,camera_pos.y-0.15,camera_pos.z-2.65);
    float index = 0.19;
    size = 0.32;
    for (auto it : numbers_sc) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, offset);
        model = glm::scale(model, glm::vec3(size*0.7,size,size));
        shader.SetUniform("model", model); GL_CHECK_ERRORS
        glBindTexture(GL_TEXTURE_2D, num[it]); GL_CHECK_ERRORS
        shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
        offset.x += index;
    }
    
    model = glm::mat4(1.0f);
    offset = glm::vec3(camera_pos.x,camera_pos.y-0.7,camera_pos.z-2.65);
    model = glm::translate(model, offset);
    size = 0.3;
    model = glm::scale(model, glm::vec3(size,size,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, pizza_tex); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    
    model = glm::mat4(1.0f);
    offset = glm::vec3(camera_pos.x,camera_pos.y-0.32,camera_pos.z-2.65);
    model = glm::translate(model, offset);
    size = 1.1;
    model = glm::scale(model, glm::vec3(size,size*0.2,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, score_title); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    
    
    glEnable(GL_BLEND); GL_CHECK_ERRORS
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); GL_CHECK_ERRORS
    
    glBindVertexArray(0); GL_CHECK_ERRORS;
}

//|||||||||||||||OBJECT|||||||||||||||OBJECT|||||||||||||||OBJECT|||||||||||||||

GLuint square_indices[] = {
    0, 1, 2,
    2, 3, 0
};

//  v2------v1
//  |       |
//  |       |
//  |       |
//  v3------v0

void Object::generate_square(glm::vec3 pos[4], glm::vec2 tex[4], glm::vec3 clr[4], glm::vec3 nrm[1]) {
    float vertex_square[44] = {
        pos[0].x, pos[0].y, pos[0].z, tex[0].x, tex[0].y, clr[0].r, clr[0].g, clr[0].b, nrm[0].x, nrm[0].y, nrm[0].z, //0
        pos[1].x, pos[1].y, pos[1].z, tex[1].x, tex[1].y, clr[1].r, clr[1].g, clr[1].b, nrm[0].x, nrm[0].y, nrm[0].z, //1
        pos[2].x, pos[2].y, pos[2].z, tex[2].x, tex[2].y, clr[2].r, clr[2].g, clr[2].b, nrm[0].x, nrm[0].y, nrm[0].z, //2
        pos[3].x, pos[3].y, pos[3].z, tex[3].x, tex[3].y, clr[3].r, clr[3].g, clr[3].b, nrm[0].x, nrm[0].y, nrm[0].z, //3
        
    };
    memcpy(this->square, vertex_square, sizeof(vertex_square));
}

void Object::generate_triangle(glm::vec3 pos[3], glm::vec2 tex[3], glm::vec3 clr[3], glm::vec3 nrm[1]) {
    float vertex_triangle[33] = {
        pos[0].x, pos[0].y, pos[0].z, tex[0].x, tex[0].y, clr[0].r, clr[0].g, clr[0].b, nrm[0].x, nrm[0].y, nrm[0].z, //0
        pos[1].x, pos[1].y, pos[1].z, tex[1].x, tex[1].y, clr[1].r, clr[1].g, clr[1].b, nrm[0].x, nrm[0].y, nrm[0].z, //1
        pos[2].x, pos[2].y, pos[2].z, tex[2].x, tex[2].y, clr[2].r, clr[2].g, clr[2].b, nrm[0].x, nrm[0].y, nrm[0].z, //2
        
    };
    memcpy(this->triangle, vertex_triangle, sizeof(vertex_triangle));
}

GLuint Object::generate_obj(Type t, bool is_tex, bool is_clr, bool is_norm) {
    int n = 0;
    switch(t){
        case TRIANGLE:
            n = 3;
            break;
        case SQUARE:
            n = 4;
            break;
        case UNKNOWN:
            break;
    }
    GLuint VBO;
    GLuint VAO;
    GLuint IBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); GL_CHECK_ERRORS
    glBindBuffer(GL_ARRAY_BUFFER, VBO); GL_CHECK_ERRORS
    
    switch(t) {
        case TRIANGLE:
            glBufferData(GL_ARRAY_BUFFER, 11*n * sizeof(GLfloat), triangle, GL_STATIC_DRAW); GL_CHECK_ERRORS
            break;
        case SQUARE:
            glBufferData(GL_ARRAY_BUFFER, 11*n * sizeof(GLfloat), square, GL_STATIC_DRAW); GL_CHECK_ERRORS
            glGenBuffers(1, &IBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO); GL_CHECK_ERRORS
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(square_indices), square_indices, GL_STATIC_DRAW); GL_CHECK_ERRORS
            break;
        case UNKNOWN:
            break;
    }
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), 0); GL_CHECK_ERRORS
    glEnableVertexAttribArray(0); GL_CHECK_ERRORS
    
    //ПРИВЯЗКА ТЕКСТУРНЫХ КООРДИНАТ
    if (is_tex) {
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))); GL_CHECK_ERRORS
        glEnableVertexAttribArray(1); GL_CHECK_ERRORS
    }
    
    //ПРИВЯЗКА ЦВЕТОВЫХ АТРИБУТОВ
    if (is_clr) {
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat))); GL_CHECK_ERRORS
        glEnableVertexAttribArray(2); GL_CHECK_ERRORS
    }
    
    //ПРИВЯЗКА АТРИБУТОВ НОРМАЛЕЙ
    if (is_norm) {
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat))); GL_CHECK_ERRORS
        glEnableVertexAttribArray(3); GL_CHECK_ERRORS
    }
    all_VBOs.push_back(VBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0); GL_CHECK_ERRORS // unbind VBO
    glBindVertexArray(0); // unbind VAO
    all_VAOs.push_back(VAO);
    return VAO;
}

//|||||||||||||||TEXTURES|||||||||||||||TEXTURES|||||||||||||||TEXTURES|||||||||||||||

//FOR LINUX
std::string getexepath_in_linux() {
    char result[ PATH_MAX ];
    ssize_t count = readlink( "/proc/self/exe", result, PATH_MAX );
    std::string path = std::string( result, (count > 0) ? count : 0 );
    std::string::size_type n = std::string(path).rfind("/build/main");
    std::string cut_str = "";
    if (n == std::string::npos) {
        std::cout << "please, run ./main from build dir" << std::endl;
    }
    else {
        cut_str = std::string(path).erase(n+1, 11);
    }
    return cut_str;
}

//FOR MACOS
/*std::string getexepath_in_macos()
{
    char path[1024];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0)
        printf("executable path is %s\n", path);
    else
        printf("buffer too small; need size %u\n", size);
    std::string::size_type n = std::string(path).rfind("/build/./main");
    std::string cut_str = "";
    if (n == std::string::npos) {
        std::cout << "please, run ./main from build dir" << std::endl;
    }
    else {
        cut_str = std::string(path).erase(n+1, 13);
    }
    return cut_str;
}*/


int Textures::load_textures(){
    //ПОИСК ПУТИ ДО КОРНЕВОЙ ДИРЕКТОРИИ
    //std::string cut_str = getexepath_in_macos(); //FOR MACOS
    std::string cut_str = getexepath_in_linux(); //FOR LINUX
    
    //ENEMY 1
    cut_str += "pics/";
    std::string name = cut_str + "enemy1.png";
    const char *file_name = name.c_str();
    
    int width_im, height_im, n_im;
    unsigned char *textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "enemy1.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &enemy1);
    glBindTexture(GL_TEXTURE_2D, enemy1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ENEMY 2
    name = cut_str + "enemy2.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "enemy2.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &enemy2);
    glBindTexture(GL_TEXTURE_2D, enemy2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ENEMY 3
    name = cut_str + "enemy3.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "enemy3.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &enemy3);
    glBindTexture(GL_TEXTURE_2D, enemy3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //FRONT
    name = cut_str + "front.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 3);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "front.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &front);
    glBindTexture(GL_TEXTURE_2D, front);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_im, height_im, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //UP
    name = cut_str + "up.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 3);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "up.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &up);
    glBindTexture(GL_TEXTURE_2D, up);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_im, height_im, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //DOWN
    name = cut_str + "down.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 3);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "down.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &down);
    glBindTexture(GL_TEXTURE_2D, down);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_im, height_im, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //RIGHT
    name = cut_str + "right.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 3);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "right.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &right);
    glBindTexture(GL_TEXTURE_2D, right);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_im, height_im, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //LEFT
    name = cut_str + "left.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 3);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "left.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &left);
    glBindTexture(GL_TEXTURE_2D, left);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_im, height_im, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ENEMY BANG 1
    name = cut_str + "bang_enemy1.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "bang_enemy1.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &bang_enemy1);
    glBindTexture(GL_TEXTURE_2D, bang_enemy1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ENEMY BANG 2
    name = cut_str + "bang_enemy2.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "bang_enemy2.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &bang_enemy2);
    glBindTexture(GL_TEXTURE_2D, bang_enemy2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ENEMY BANG 3
    name = cut_str + "bang_enemy3.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "bang_enemy3.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &bang_enemy3);
    glBindTexture(GL_TEXTURE_2D, bang_enemy3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //AIM
    name = cut_str + "aim.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "aim.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &aim);
    glBindTexture(GL_TEXTURE_2D, aim);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //PARTICLE
    name = cut_str + "particle.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 3);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "particle.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &particle);
    glBindTexture(GL_TEXTURE_2D, particle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_im, height_im, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //DUST
    name = cut_str + "dust.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 3);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "dust.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &dust);
    glBindTexture(GL_TEXTURE_2D, dust);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_im, height_im, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ASTEROID
    name = cut_str + "asteroid1.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "asteroid1.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &asteroid1);
    glBindTexture(GL_TEXTURE_2D, asteroid1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ASTEROID
    name = cut_str + "asteroid2.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "asteroid2.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &asteroid2);
    glBindTexture(GL_TEXTURE_2D, asteroid2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SHOT 1
    name = cut_str + "shot1.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "shot1.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &shot1);
    glBindTexture(GL_TEXTURE_2D, shot1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SHOT 2
    name = cut_str + "shot2.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "shot2.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &shot2);
    glBindTexture(GL_TEXTURE_2D, shot2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SHOT 3
    name = cut_str + "shot3.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "shot3.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &shot3);
    glBindTexture(GL_TEXTURE_2D, shot3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //DANGER SHOT 1
    name = cut_str + "dange_shot1.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "dange_shot1.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &dang_shot1);
    glBindTexture(GL_TEXTURE_2D, dang_shot1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //DANGER SHOT 2
    name = cut_str + "dange_shot2.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "dange_shot2.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &dang_shot2);
    glBindTexture(GL_TEXTURE_2D, dang_shot2);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //DANGER SHOT 3
    name = cut_str + "dange_shot3.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "dange_shot3.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &dang_shot3);
    glBindTexture(GL_TEXTURE_2D, dang_shot3);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //DAMAGE
    name = cut_str + "damage.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "damage.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &damage);
    glBindTexture(GL_TEXTURE_2D, damage);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //HEALTH
    name = cut_str + "health.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "health.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &health);
    glBindTexture(GL_TEXTURE_2D, health);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SCORE
    name = cut_str + "score.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "score.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &score);
    glBindTexture(GL_TEXTURE_2D, score);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SCORE
    name = cut_str + "weapon.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "weapon.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &weapon);
    glBindTexture(GL_TEXTURE_2D, weapon);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ZER0
    name = cut_str + "zero.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "zero.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &zero);
    glBindTexture(GL_TEXTURE_2D, zero);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //ONE
    name = cut_str + "one.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "one.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &one);
    glBindTexture(GL_TEXTURE_2D, one);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //TWO
    name = cut_str + "two.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "two.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &two);
    glBindTexture(GL_TEXTURE_2D, two);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //THREE
    name = cut_str + "three.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "three.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &three);
    glBindTexture(GL_TEXTURE_2D, three);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //FOUR
    name = cut_str + "four.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "four.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &four);
    glBindTexture(GL_TEXTURE_2D, four);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //FIVE
    name = cut_str + "five.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "five.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &five);
    glBindTexture(GL_TEXTURE_2D, five);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SIX
    name = cut_str + "six.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "six.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &six);
    glBindTexture(GL_TEXTURE_2D, six);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SEVEN
    name = cut_str + "seven.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "seven.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &seven);
    glBindTexture(GL_TEXTURE_2D, seven);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //EIGHT
    name = cut_str + "eight.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "eight.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &eight);
    glBindTexture(GL_TEXTURE_2D, eight);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //NINE
    name = cut_str + "nine.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "nine.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &nine);
    glBindTexture(GL_TEXTURE_2D, nine);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //GAME OVER
    name = cut_str + "game_over.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "game_over.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &game_over);
    glBindTexture(GL_TEXTURE_2D, game_over);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SCORE TITLE
    name = cut_str + "score_title.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "score_title.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &score_title);
    glBindTexture(GL_TEXTURE_2D, score_title);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //SAD CAT
    name = cut_str + "sad_cat.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "sad_cat.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &sad_cat);
    glBindTexture(GL_TEXTURE_2D, sad_cat);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //DAMAGE ENEMY
    name = cut_str + "damage_enemy.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "damage_enemyt.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &damage_enemy);
    glBindTexture(GL_TEXTURE_2D, damage_enemy);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    //DAMAGE ASTEROID
    name = cut_str + "damage_asteroid.png";
    file_name = name.c_str();
    
    textureData = stbi_load(file_name, &width_im, &height_im, &n_im, 4);
    if(textureData == nullptr) {
        std::cout << "loadTexture failed, fname = " << "damage_asteroid.png" << std::endl;
        return -1;
    }
    glGenTextures(1, &damage_asteroid);
    glBindTexture(GL_TEXTURE_2D, damage_asteroid);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_im, height_im, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(textureData);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return 0;
}

//|||||||||||||||ASTEROID|||||||||||||||ASTEROID|||||||||||||||ASTEROID|||||||||||||||

//  4__________3__________2
//  |\\        | \     / /|
//  | \ \      |   \ /  / |
//  |  \  \    |   10  /  |
//  |   \   \  |  /   /   |
//  |5___12___\0/____14___|1
//  |\       //| \       /|
//  | \    / / |   \   /  |
//  |   \ /  / |    11    |
//  |   /13 /  |   /  \   |
//  | /    /   | /      \ |
//  6-----9----7----------8
glm::vec2 part_tc[] = {
    glm::vec2(wid(1/2.0f), hei(1/2.0f)), //0
    glm::vec2(wid(1.0f), hei(1/2.0f)), //1
    glm::vec2(wid(1.0f), hei(1.0f)), //2
    glm::vec2(wid(1/2.0f), hei(1.0f)), //3
    glm::vec2(wid(0.0f), hei(1.0f)), //4
    glm::vec2(wid(0.0f), hei(1/2.0f)), //5
    glm::vec2(wid(0.0f), hei(0.0f)), //6
    glm::vec2(wid(1/2.0f), hei(0.0f)), //7
    glm::vec2(wid(1.0f), hei(0.0f)), //8
    glm::vec2(wid(1/4.0f), hei(0.0f)), //9
    glm::vec2(wid(3/4.0f), hei(3/4.0f)), //10
    glm::vec2(wid(3/4.0f), hei(1/4.0f)), //11
    glm::vec2(wid(1/4.0f), hei(1/2.0f)), //12
    glm::vec2(wid(1/4.0f), hei(1/4.0f)), //13
    glm::vec2(wid(3/4.0f), hei(1/2.0f)), //14
};

int part_v[] = {
    14, 2, 0, 1, 2, 14, 3, 10, 2, 0, 10, 3,
    0, 3, 4, 0, 4, 12, 12, 4, 5,
    6, 13, 5, 5, 13, 0, 9, 0, 6, 7, 0, 9,
    7, 11, 0, 8, 11, 7, 8, 1, 0
};

int part_t[] = {
    2, 3, 1, 1,
    1, 4, 5,
    1, 1, 2, 3,
    1, 1, 1
};

void Asteroid::generate_obj(Type t, GLuint tex){
    glm::vec3 triangle_color_coord[] = {
        glm::vec3(1.0f, 0.0f, 0.0f), //0
        glm::vec3(1.0f, 0.0f, 0.0f), //1
        glm::vec3(1.0f, 0.0f, 0.0f) //2
    };
    
    int num_v = 0;
    for (int i = 0; i < 14; i++) {
        glm::vec2 triangle_tex_coord[] = {
            part_tc[part_v[num_v]], part_tc[part_v[num_v+1]], part_tc[part_v[num_v+2]]
        };
        num_v+=3;
        Object obj;
        if (part_t[i] == 1)
            obj.generate_triangle(triangle1_dot, triangle_tex_coord, triangle_color_coord, triangle_normals);
        if (part_t[i] == 2)
            obj.generate_triangle(triangle2_dot, triangle_tex_coord, triangle_color_coord, triangle_normals);
        if (part_t[i] == 3)
            obj.generate_triangle(triangle3_dot, triangle_tex_coord, triangle_color_coord, triangle_normals);
        if (part_t[i] == 4)
            obj.generate_triangle(triangle4_dot, triangle_tex_coord, triangle_color_coord, triangle_normals);
        if (part_t[i] == 5)
            obj.generate_triangle(triangle5_dot, triangle_tex_coord, triangle_color_coord, triangle_normals);
        objs.push_back(obj);
        GLuint VAO = obj.generate_obj(t, true, true, true);
        VAOs.push_back(VAO);
    }
    type = t;
    texture = tex;
    start_pos = glm::vec3(camera_pos.x+random(-10,10),camera_pos.y+random(-10,10), camera_pos.z-40.0f);
    curr_pos = start_pos;
    end_pos = camera_pos + glm::vec3(random(-1, 1), random (-1,1), 2.35);
    glm::vec3 dir = end_pos - start_pos;
    float len_dir = glm::length(dir);
    direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
    speed = random(0.06, 0.1);
    killed = false;
    size = random(0.4,1.3);
    radius *= size;
}

void Asteroid::check_alive(){
    if (not_visible) {
        killed = false;
        not_visible = false;
        bump = false;
        start_pos = glm::vec3(camera_pos.x+random(-10,10),camera_pos.y+random(-10,10), camera_pos.z-40.0f);
        curr_pos = start_pos;
        end_pos = camera_pos + glm::vec3(random(-1, 1), random (-1,1), 2.35);
        glm::vec3 dir = end_pos - start_pos;
        float len_dir = glm::length(dir);
        direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
        speed = random(0.06, 0.1);
        size = random(0.4,1.3);
        particles_pos.clear();
        particle_dirs.clear();
        particles_speed.clear();
    }
    else if (!bump){
        //float distanse = glm::length(curr_pos-camera_pos);
        if (curr_pos.z > camera_pos.z-3.65) {
            bump = true;
            kill();
            player.health_loss(3.0);
            Damage_str dam;
            dam.damage_pos = camera_pos - curr_pos;
            dam.damage_src = 2;
            player.damages.push_back(dam);
        }
    }
}

float angle[] = {
    0.0f, 0.0f, 180.0f, -90.0f,
    -45.0f, 0.0f, 0.0f,
    -90.0f, 180.0f, 0.0f, 0.0f,
    -90.0f, 0.0f, -45.0f
};
float value2 = sqrt(2);
glm::vec3 translate[] = {
    glm::vec3(1/2.0f, 1/2.0f, 0.0f), //0
    glm::vec3(3/4.0f, 1/2.0f, 0.0f), //1
    glm::vec3(1/2.0f, 1.0f, 0.0f), //2
    glm::vec3(0.0f, 1/2.0f, 0.0f), //3
    glm::vec3(-1/2.0f, 1/2.0f, 0.0f), //4
    glm::vec3(-1/2.0f, 1/2.0f, 0.0f), //5
    glm::vec3(-3/4.0f, 1/2.0f, 0.0f), //6
    glm::vec3(-1.0f, -1/2.0f, 0.0f), //7
    glm::vec3(-1/2.0f, 0.0f, 0.0f), //8
    glm::vec3(-1/2.0f, -1/2.0f, 0.0f), //9
    glm::vec3(-1/4.0f, -1/2.0f, 0.0f), //10
    glm::vec3(0.0f, -1/2.0f, 0.0f), //11
    glm::vec3(1/2.0f, -1.0f, 0.0f), //12
    glm::vec3(1/2.0f, -1/2.0f, 0.0f) //13
};
glm::vec3 scale[] = {
    glm::vec3(0.5,0.5,0.5), //0
    glm::vec3(0.5,0.5,0.5), //1
    glm::vec3(0.5,0.5,0.5), //2
    glm::vec3(0.5,0.5,0.5), //3
    glm::vec3(1/value2,1/value2,1/value2), //4
    glm::vec3(0.5,0.5,0.5), //5
    glm::vec3(0.5,0.5,0.5), //6
    glm::vec3(0.5,0.5,0.5), //7
    glm::vec3(0.5,0.5,0.5), //8
    glm::vec3(0.5,0.5,0.5), //9
    glm::vec3(0.5,0.5,0.5), //10
    glm::vec3(0.5,0.5,0.5), //11
    glm::vec3(0.5,0.5,0.5), //12
    glm::vec3(1/value2,1/value2,1/value2) //13
};

void Asteroid::draw(ShaderProgram shader) {
    glBindTexture(GL_TEXTURE_2D, texture); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    curr_pos += glm::vec3(direction.x*speed,direction.y*speed,direction.z*speed);
    bool visible_particle = false;
    if (!killed) {
        for (int i = 0; i < 14; i++){
            glm::mat4 model(1.0f);
            model = glm::translate(model, curr_pos);
            glm::vec3 offset = glm::vec3(translate[i].x*size, translate[i].y*size, translate[i].z*size);
            model = glm::translate(model, offset);
            model = glm::scale(model, glm::vec3(size,size,size));
            model = glm::scale(model, scale[i]);
            model = glm::rotate(model, glm::radians(angle[i]), glm::vec3(0.0,0.0,1.0));
            shader.SetUniform("model", model); GL_CHECK_ERRORS
            
            glBindVertexArray(VAOs[i]); GL_CHECK_ERRORS
            glDrawArrays(GL_TRIANGLES, 0, 3); GL_CHECK_ERRORS
            glBindVertexArray(0); GL_CHECK_ERRORS;
        }
    }
    else {
        int size_particles = particle_dirs.size();
        for (int i = 0; i < 14; i++){
            glm::mat4 model(1.0f);
            if (size_particles == 0) {
                glm::vec3 dir = glm::vec3(random(-1.0, 1.0),random(-1.0, 1.0),random(-1.0, 1.0));
                float len_dir = glm::length(dir);
                glm::vec3 part_dir = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
                particle_dirs.push_back(part_dir);
                
                float part_speed = random(speed, speed+0.3);
                particles_speed.push_back(part_speed);
                
                curr_pos += glm::vec3(part_dir.x*part_speed,part_dir.y*part_speed,part_dir.z*part_speed);
                model = glm::translate(model, curr_pos);
                glm::vec3 offset = glm::vec3(translate[i].x*size, translate[i].y*size, translate[i].z*size);
                model = glm::translate(model, offset);
                curr_pos +=offset;
                particles_pos.push_back(curr_pos);
                
                model = glm::scale(model, glm::vec3(size,size,size));
                model = glm::scale(model, scale[i]);
                model = glm::rotate(model, glm::radians(angle[i]), glm::vec3(0.0,0.0,1.0));
                shader.SetUniform("model", model); GL_CHECK_ERRORS
            }
            particles_pos[i] += glm::vec3(particle_dirs[i].x*particles_speed[i],
                                          particle_dirs[i].y*particles_speed[i],
                                          particle_dirs[i].z*particles_speed[i]);
            model = glm::translate(model, particles_pos[i]);
            float distanse_particle = glm::length(particles_pos[i] - camera_pos);
            if (distanse_particle < 40.0)
                visible_particle = true;
            model = glm::scale(model, glm::vec3(size,size,size));
            model = glm::scale(model, scale[i]);
            
            glm::vec3 axis = glm::vec3(random(-1.0, 1.0),random(-1.0, 1.0),random(-1.0, 1.0));
            float len_axis = glm::length(axis);
            glm::vec3 rot_axis = glm::vec3(axis.x/len_axis, axis.y/len_axis, axis.z/len_axis);
            model = glm::rotate(model,glm::radians((GLfloat)glfwGetTime() * 50.0f), rot_axis);
            shader.SetUniform("model", model); GL_CHECK_ERRORS
            
            glBindVertexArray(VAOs[i]); GL_CHECK_ERRORS
            glDrawArrays(GL_TRIANGLES, 0, 3); GL_CHECK_ERRORS
            glBindVertexArray(0); GL_CHECK_ERRORS;
        }
    }
    if (!(visible_particle || curr_pos.z < 3.0)) {
        not_visible = true;
    }
}

void Asteroid::kill(){
    killed = true;
}

//|||||||||||||||ENEMY|||||||||||||||ENEMY|||||||||||||||ENEMY|||||||||||||||

void Enemy::kill() {
    bang = true;
    start_bang = glfwGetTime(); GL_CHECK_ERRORS
    size_bang = size;
}

void Enemy::generate_obj(GLuint tex, GLuint shot_tex, GLuint bang_tex, float size){
    glm::vec2 square_tex_coord[] = {
        glm::vec2(wid(1.0f), hei(0.0f)), //0
        glm::vec2(wid(1.0f), hei(1.0f)), //1
        glm::vec2(wid(0.0f), hei(1.0f)), //2
        glm::vec2(wid(0.0f), hei(0.0f)) //3
    };
    glm::vec3 square_color_coord[] = {
        glm::vec3(1.0f, 0.0f, 0.0f), //0
        glm::vec3(1.0f, 0.0f, 0.0f), //1
        glm::vec3(1.0f, 0.0f, 0.0f), //2
        glm::vec3(1.0f, 0.0f, 0.0f) //3
    };
    obj.generate_square(square_dot, square_tex_coord, square_color_coord, square_normals);
    VAO = obj.generate_obj(SQUARE, true, false, true);
    texture = tex;
    shot_text = shot_tex;
    bang_text = bang_tex;
    start_pos = glm::vec3(camera_pos.x+random(-10,10),camera_pos.y+random(-10,10), camera_pos.z-40.0f);
    curr_pos = start_pos;
    end_pos = camera_pos + glm::vec3(random(-0.6, 0.6), random (-0.6,0.6), 3.0);
    glm::vec3 dir = end_pos - start_pos;
    float len_dir = glm::length(dir);
    direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
    speed = random(0.07, 0.12);
    radius *= size;
}

void Enemy::check_alive(){
    if (curr_pos.z > end_pos.z || killed) {
        killed = false;
        bang = false;
        bump = false;
        start_pos = glm::vec3(camera_pos.x+random(-10,10),camera_pos.y+random(-10,10), camera_pos.z-40.0f);
        curr_pos = start_pos;
        end_pos = camera_pos + glm::vec3(random(-0.6, 0.6), random (-0.6,0.6), 3.0);
        glm::vec3 dir = end_pos - start_pos;
        float len_dir = glm::length(dir);
        direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
        speed = random(0.07, 0.12);
    }
    else if (!bump){
        if (curr_pos.z > camera_pos.z-22.65)
            generate_shot();
        //float distanse = glm::length(curr_pos-camera_pos);
        if (curr_pos.z > camera_pos.z-3.65) {
            bump = true;
            kill();
            player.health_loss(5.0);
            Damage_str dam;
            dam.damage_pos = camera_pos - curr_pos;
            dam.damage_src = 1;
            player.damages.push_back(dam);
        }
    }
}

void Enemy::generate_shot(){
    float r = random(-20,20);
    if ((r>0) && (r < 1.5)) {
        bool flag = false;
        for (auto it=dang_shots.begin(); it != dang_shots.end(); it++) {
            if (!(it->shooted)) {
                it->new_shot(curr_pos);
                flag = true;
                break;
            }
        }
        if (!flag) {
            DangerShot dang_shot;
            dang_shot.generate_obj(shot_text, curr_pos);
            dang_shots.push_back(dang_shot);
        }
    }
}

void Enemy::draw(ShaderProgram shader) {
    //MODEL
    glm::mat4 model(1.0f);
    if (!bang) {
        curr_pos += glm::vec3(direction.x*speed,direction.y*speed,direction.z*speed);
        model = glm::translate(model, curr_pos);
        model = glm::scale(model, glm::vec3(size,size,size));
        shader.SetUniform("model", model); GL_CHECK_ERRORS
        glBindTexture(GL_TEXTURE_2D, texture); GL_CHECK_ERRORS
        shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    }
    else {
        model = glm::translate(model, curr_pos);
        model = glm::scale(model, glm::vec3(size_bang,size_bang,size_bang));
        shader.SetUniform("model", model); GL_CHECK_ERRORS
        glBindTexture(GL_TEXTURE_2D, bang_text); GL_CHECK_ERRORS
        shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
        float offset;
        if (!bump) {
            offset = 0.4;
        }
        else {
            offset = 0.4;
        }
        size_bang+=0.03;
        if ((glfwGetTime() - start_bang) > offset) {
            killed = true;
        }
    }
    glBindVertexArray(VAO); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    glBindVertexArray(0); GL_CHECK_ERRORS;
}

//|||||||||||||||SHOT|||||||||||||||SHOT|||||||||||||||SHOT|||||||||||||||

void Shot::generate_obj(){
    glm::vec2 square_tex_coord[] = {
        glm::vec2(wid(1.0f), hei(0.0f)), //0
        glm::vec2(wid(1.0f), hei(1.0f)), //1
        glm::vec2(wid(0.0f), hei(1.0f)), //2
        glm::vec2(wid(0.0f), hei(0.0f)) //3
    };
    glm::vec3 square_color_coord[] = {
        glm::vec3(1.0f, 0.0f, 0.0f), //0
        glm::vec3(1.0f, 0.0f, 0.0f), //1
        glm::vec3(1.0f, 0.0f, 0.0f), //2
        glm::vec3(1.0f, 0.0f, 0.0f) //3
    };
    obj.generate_square(square_dot, square_tex_coord, square_color_coord, square_normals);
    VAO = obj.generate_obj(SQUARE, true, false, true);
    start_pos = glm::vec3(camera_pos.x+aim_coord_x, camera_pos.y+aim_coord_y, camera_pos.z-3.0);
    curr_pos = start_pos;
    glm::vec3 dir = start_pos - camera_pos;
    float len_dir = glm::length(dir);
    direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
}

void Shot::new_shot(){
    shooted = true;
    start_pos = glm::vec3(camera_pos.x+aim_coord_x, camera_pos.y+aim_coord_y, camera_pos.z-2.64);
    curr_pos = start_pos;
    glm::vec3 dir = start_pos - camera_pos;
    float len_dir = glm::length(dir);
    direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
}

bool Shot::check_hit(){
    bool missed = false;
    bool hitted = false;
    if (curr_pos.z < camera_pos.z-40.0) {
        missed = true;
        shooted = false;
    }
    else {
        for (auto it = enemies.begin(); it != enemies.end(); it++){
            glm::vec3 dist_to = glm::vec3(it->curr_pos.x, it->curr_pos.y, it->curr_pos.z - 3);
            float approach = glm::length(dist_to) / 15.0;
            float temp_rad = it->radius + approach;
            glm::vec3 dist_between = glm::vec3(it->curr_pos.x - curr_pos.x, it->curr_pos.y - curr_pos.y, it->curr_pos.z - curr_pos.z);
            if (glm::length(dist_between) < temp_rad) {
                hitted = true;
                shooted = false;
                player.score_up(3);
                it->kill();
            }
        }
        for (auto it = asters.begin(); it != asters.end(); it++){
            glm::vec3 dist_to = glm::vec3(it->curr_pos.x, it->curr_pos.y, it->curr_pos.z - 3);
            float approach = glm::length(dist_to) / 15.0;
            float temp_rad = it->radius + approach;
            glm::vec3 dist_between = glm::vec3(it->curr_pos.x - curr_pos.x, it->curr_pos.y - curr_pos.y, it->curr_pos.z - curr_pos.z);
            if (glm::length(dist_between) < temp_rad) {
                hitted = true;
                shooted = false;
                player.score_up(1);
                it->kill();
            }
        }
    }

    return (missed || hitted);
}

    void Shot::draw(ShaderProgram shader, GLuint shot1, GLuint shot2, GLuint shot3) {
    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(0.6, 0.6, 0.6));
    curr_pos += glm::vec3(direction.x*1.8,direction.y*1.8,direction.z*1.8);;
    model = glm::translate(model, curr_pos);
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    switch (player.weapon_type) {
        case 0:
            glBindTexture(GL_TEXTURE_2D, shot1); GL_CHECK_ERRORS
            break;
        case 1:
            glBindTexture(GL_TEXTURE_2D, shot2); GL_CHECK_ERRORS
            break;
        case 2:
            glBindTexture(GL_TEXTURE_2D, shot3); GL_CHECK_ERRORS
            break;
    }
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    
    glBindVertexArray(VAO); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    glBindVertexArray(0); GL_CHECK_ERRORS;

}

//|||||||||||||||DANGER SHOT|||||||||||||||DANGER SHOT|||||||||||||||DANGER SHOT|||||||||||||||

void DangerShot::generate_obj(GLuint tex, glm::vec3 start_pos){
    glm::vec2 square_tex_coord[] = {
        glm::vec2(wid(1.0f), hei(0.0f)), //0
        glm::vec2(wid(1.0f), hei(1.0f)), //1
        glm::vec2(wid(0.0f), hei(1.0f)), //2
        glm::vec2(wid(0.0f), hei(0.0f)) //3
    };
    glm::vec3 square_color_coord[] = {
        glm::vec3(1.0f, 0.0f, 0.0f), //0
        glm::vec3(1.0f, 0.0f, 0.0f), //1
        glm::vec3(1.0f, 0.0f, 0.0f), //2
        glm::vec3(1.0f, 0.0f, 0.0f) //3
    };
    obj.generate_square(square_dot, square_tex_coord, square_color_coord, square_normals);
    VAO = obj.generate_obj(SQUARE, true, false, true);\
    texture = tex;
    this->start_pos = start_pos;
    curr_pos = start_pos;
    glm::vec3 dir = camera_pos - start_pos;
    float len_dir = glm::length(dir);
    direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
}

void DangerShot::new_shot(glm::vec3 start_pos){
    shooted = true;
    this->start_pos = start_pos;
    curr_pos = start_pos;
    glm::vec3 dir = camera_pos - start_pos;
    float len_dir = glm::length(dir);
    direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
}

bool DangerShot::check_hit(){
    bool missed = false;
    bool hitted = false;
    if (curr_pos.z > 3.0) {
        missed = true;
        shooted = false;
    }
    else {
        glm::vec3 dist_between = camera_pos - curr_pos;
        if (glm::length(dist_between) < 0.1) {
            hitted = true;
            shooted = false;
            player.health_loss(0.5);
        }
    }
    return (missed || hitted);
}

void DangerShot::draw(ShaderProgram shader) {
    glm::mat4 model(1.0f);
    model = glm::scale(model, glm::vec3(0.1, 0.1, 0.1));
    curr_pos += glm::vec3(direction.x,direction.y,direction.z);
    model = glm::translate(model, curr_pos);
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, texture); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
    glBindVertexArray(VAO); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    glBindVertexArray(0); GL_CHECK_ERRORS;
}

//|||||||||||||||SKYBOX|||||||||||||||SKYBOX|||||||||||||||SKYBOX|||||||||||||||

glm::vec2 sky_tex_coord[] = {
    glm::vec2(wid(1.0f), hei(0.0f)), //
    glm::vec2(wid(1.0f), hei(1.0f)), //
    glm::vec2(wid(0.0f), hei(1.0f)), //
    glm::vec2(wid(0.0f), hei(0.0f)), //
    
    glm::vec2(wid(1.0f), hei(0.0f)), // 0
    glm::vec2(wid(1.0f), hei(1.0f)), // 1
    glm::vec2(wid(0.0f), hei(1.0f)), // 2
    glm::vec2(wid(0.0f), hei(0.0f)), // 3
    
    glm::vec2(wid(1.0f), hei(0.0f)), //
    glm::vec2(wid(1.0f), hei(1.0f)), //
    glm::vec2(wid(0.0f), hei(1.0f)), //
    glm::vec2(wid(0.0f), hei(0.0f)), //
    
    glm::vec2(wid(1.0f), hei(0.0f)), //
    glm::vec2(wid(1.0f), hei(1.0f)), //
    glm::vec2(wid(0.0f), hei(1.0f)), //
    glm::vec2(wid(0.0f), hei(0.0f)), //
    
    glm::vec2(wid(1.0f), hei(0.0f)), //
    glm::vec2(wid(1.0f), hei(1.0f)), //
    glm::vec2(wid(0.0f), hei(1.0f)), //
    glm::vec2(wid(0.0f), hei(0.0f)), //
    
};

int sky_ind[] = {
    0, 1, 2, 3, //front
    5, 4, 7, 6,  //up
    11, 10, 9, 8,  //down
    15,14, 13, 12, //right
    19,18,17,16, //left
};

void SkyBox::generate_cube() {
    glm::vec3 color_coord[] = {
        glm::vec3(1.0f, 1.0f, 1.0f), //0
        glm::vec3(1.0f, 1.0f, 1.0f), //1
        glm::vec3(1.0f, 1.0f, 1.0f), //2
        glm::vec3(1.0f, 1.0f, 1.0f) //3
    };
    int num_v = 0;
    for (int i = 0; i < 5 ; i++) {
        glm::vec2 square_tex_coord[] = {
            sky_tex_coord[sky_ind[num_v]], sky_tex_coord[sky_ind[num_v+1]],
            sky_tex_coord[sky_ind[num_v+2]], sky_tex_coord[sky_ind[num_v+3]]
        };
        Object obj;
        obj.generate_square(square_dot, square_tex_coord, color_coord, square_normals);
        objs.push_back(obj);
        GLuint VAO = obj.generate_obj(SQUARE, true, true, true);
        VAOs.push_back(VAO);
        num_v+=4;
    }
}
glm::vec3 sky_rot_axis[] = {
    glm::vec3(1.0,0.0,0.0),
    glm::vec3(1.0,0.0,0.0),
    glm::vec3(1.0,0.0,0.0),
    glm::vec3(0.0,1.0,0.0),
    glm::vec3(0.0,1.0,0.0)
};

float sky_angle[] = {
    0.0, -90.0, 90.0, 90.0, -90.0
};

glm::vec3 sky_translate[] = {
    glm::vec3(0.0f, 0.0f, -1.5f), //0
    glm::vec3(0.0f, -1/2.0f, 0.0f), //1
    glm::vec3(0.0f, 1/2.0f, 0.0f), //2
    glm::vec3(-1/2.0f, 0.0f, 0.0f), //3 right
    glm::vec3(1/2.0f, 0.0f, 0.0f), //4 left
    
};

void SkyBox::draw(ShaderProgram shader, GLuint texts[5]){
    float size = 60.0;
    for (int i = 0; i < 5; i++){
        glBindTexture(GL_TEXTURE_2D, texts[i]); GL_CHECK_ERRORS
        shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS
        glm::mat4 model(1.0f);
        glm::vec3 offset = glm::vec3(sky_translate[i].x*size, sky_translate[i].y*size, sky_translate[i].z*size);
        model = glm::translate(model, offset+camera_pos);
        model = glm::translate(model, glm::vec3(0.0,0.0,-size));
        model = glm::scale(model, glm::vec3(size,size,size));
        model = glm::rotate(model, glm::radians(sky_angle[i]), sky_rot_axis[i]);
        shader.SetUniform("model", model); GL_CHECK_ERRORS
        
        glBindVertexArray(VAOs[i]); GL_CHECK_ERRORS
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
        glBindVertexArray(0); GL_CHECK_ERRORS;
    }
}

//|||||||||||||||PARTICLE|||||||||||||||PARTICLE|||||||||||||||PARTICLE|||||||||||||||

void Particle::generate_obj(GLuint tex, glm::vec3 color) {
    glm::vec3 color_coord[] = {
        color, color, color, color
    };
    glm::vec2 square_tex_coord[] = {
        glm::vec2(wid(1.0f), hei(0.0f)), //0
        glm::vec2(wid(1.0f), hei(1.0f)), //1
        glm::vec2(wid(0.0f), hei(1.0f)), //2
        glm::vec2(wid(0.0f), hei(0.0f)) //3
    };
    obj.generate_square(square_dot, square_tex_coord, color_coord, square_normals);
    VAO = obj.generate_obj(SQUARE, true, true, true);
    texture = tex;
    start_pos = glm::vec3(camera_pos.x+random(-7,7),camera_pos.y+random(-7,7), camera_pos.z-20.0f);
    curr_pos = start_pos;
    glm::vec3 offset[4] = {
        glm::vec3(random(0.5, 2.0), random(0.5, 2.0), 0.0),
        glm::vec3(random(-2.0, -0.5), random(0.5, 2.0), 0.0),
        glm::vec3(random(0.5, 2.0), random(-2.0, -0.5), 0.0),
        glm::vec3(random(-2.0, -0.5), random(-2.0, -0.5), 0.0)
    };
    if ((start_pos.x > 0.0) && (start_pos.y > 0.0)){
        end_pos = camera_pos + offset[0];
    }
    if ((start_pos.x <= 0.0) && (start_pos.y > 0.0)) {
        end_pos = camera_pos + offset[1];
    }
    if ((start_pos.x > 0.0) && (start_pos.y <= 0.0)){
        end_pos = camera_pos + offset[2];
    }
    if ((start_pos.x <= 0.0) && (start_pos.y <= 0.0)) {
        end_pos = camera_pos + offset[3];
    }
    glm::vec3 dir = end_pos - start_pos;
    float len_dir = glm::length(dir);
    direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
    speed = random(0.1, 0.16);
    size = random(0.08,0.03);
}

void Particle::check_alive () {
    if (not_visible) {
        not_visible = false;
        start_pos = glm::vec3(camera_pos.x+random(-7,7),camera_pos.y+random(-7,7), camera_pos.z-20.0f);
        curr_pos = start_pos;
        end_pos = camera_pos + glm::vec3(random(-3, 3), random (-3,3), 3.0f);
        glm::vec3 dir = end_pos - start_pos;
        float len_dir = glm::length(dir);
        direction = glm::vec3(dir.x/len_dir, dir.y/len_dir, dir.z/len_dir);
        speed = random(0.13, 0.18);
        size = random(0.08,0.03);
    }
}

void Particle::draw(ShaderProgram shader){
    glm::mat4 model(1.0f);
    curr_pos += glm::vec3(direction.x*speed,direction.y*speed,direction.z*speed);
    model = glm::translate(model, curr_pos);
    model = glm::scale(model, glm::vec3(size,size,size));
    shader.SetUniform("model", model); GL_CHECK_ERRORS
    glBindTexture(GL_TEXTURE_2D, texture); GL_CHECK_ERRORS
    shader.SetUniform("fragm_text", 0); GL_CHECK_ERRORS

    if (curr_pos.z > 3.0)
        not_visible = true;
    
    glBindVertexArray(VAO); GL_CHECK_ERRORS
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
    glBindVertexArray(0); GL_CHECK_ERRORS;
}


//|||||||||||||||MAIN|||||||||||||||MAIN|||||||||||||||MAIN|||||||||||||||

int main(int argc, char** argv) {
	if(!glfwInit())
    return -1;
    
    srand((unsigned int)time(0));

	//запрашиваем контекст opengl версии 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); 
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //FOR MACOS
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow*  window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL basic sample", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    
	glfwMakeContextCurrent(window); 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if(initGL() != 0) {
        return -1;
    }
    
	GLenum gl_error = glGetError();
    while (gl_error != GL_NO_ERROR) {
        gl_error = glGetError();
    }
    
	std::unordered_map<GLenum, std::string> shaders1;
	shaders1[GL_VERTEX_SHADER]   = "vertex.glsl";
	shaders1[GL_FRAGMENT_SHADER] = "fragment_tex_clr_nrm.glsl";
	ShaderProgram shader_tex_clr_nrm(shaders1); GL_CHECK_ERRORS;
    
    std::unordered_map<GLenum, std::string> shaders2;
    shaders2[GL_VERTEX_SHADER]   = "vertex.glsl";
    shaders2[GL_FRAGMENT_SHADER] = "fragment_tex_clr.glsl";
    ShaderProgram shader_tex_clr(shaders2); GL_CHECK_ERRORS;
    
    std::unordered_map<GLenum, std::string> shaders3;
    shaders3[GL_VERTEX_SHADER]   = "vertex.glsl";
    shaders3[GL_FRAGMENT_SHADER] = "fragment_tex_nrm.glsl";
    ShaderProgram shader_tex_nrm(shaders3); GL_CHECK_ERRORS;


    glfwSwapInterval(1); // force 60 frames per second
    
    Textures texts;
    if (texts.load_textures() == -1)
        return -1;
    
    
    glm::vec2 square_tex_coord[] = {
        glm::vec2(wid(1.0f), hei(0.0f)), //0
        glm::vec2(wid(1.0f), hei(1.0f)), //1
        glm::vec2(wid(0.0f), hei(1.0f)), //2
        glm::vec2(wid(0.0f), hei(0.0f)) //3
        
    };
    glm::vec3 square_color_coord[] = {
        glm::vec3(1.0f, 0.0f, 0.0f), //0
        glm::vec3(1.0f, 0.0f, 0.0f), //1
        glm::vec3(1.0f, 0.0f, 0.0f), //2
        glm::vec3(1.0f, 0.0f, 0.0f) //3
    };
    Object objct;
    objct.generate_square(square_dot, square_tex_coord, square_color_coord, square_normals);
    GLuint aim_VAO = objct.generate_obj(SQUARE, true, false, false);
    
    std::vector<Shot> shots;
    
    Asteroid aster;
    aster.generate_obj(TRIANGLE, texts.asteroid1);
    asters.push_back(aster);
    
    std::vector<Particle> particles;
    Particle p1, p2;
    p1.generate_obj(texts.particle, glm::vec3(random(0.0,1.0), random(0.0,1.0),random(0.0,1.0)));
    p2.generate_obj(texts.dust, glm::vec3(random(0.0,1.0), random(0.0,1.0),random(0.0,1.0)));
    particles.push_back(p1);
    particles.push_back(p2);
    particles.push_back(p1);
    particles.push_back(p2);
    particles.push_back(p1);
    particles.push_back(p2);
    particles.push_back(p1);
    particles.push_back(p2);
    
    SkyBox skybox;
    skybox.generate_cube();
    GLuint skybox_textures[5] = {texts.front, texts.up, texts.down, texts.right, texts.left};
    
    GLuint numbers_textures[10] = {texts.zero, texts.one, texts.two,
                                    texts.three, texts.four, texts.five,
                                    texts.six, texts.seven, texts.eight, texts.nine};
    
    player.generate_obj();
    GLfloat currentFrame = 0.0;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_MULTISAMPLE);
        do_movement();
        GLfloat currentFrame = glfwGetTime();
        delta = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);               GL_CHECK_ERRORS;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); GL_CHECK_ERRORS;

        int width, height;
        //glfwGetFramebufferSize(window, &width, &height); //FOR MACOS
        
        width = WIDTH;         //FOR LINUX
        height = HEIGHT;        //FOR LINUX
        
        
        glViewport(0, 0, width, height);
        
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear     (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        
        
        shader_tex_clr_nrm.StartUseShader();  GL_CHECK_ERRORS;
        shader_tex_clr_nrm.SetUniform("is_tex", true); GL_CHECK_ERRORS
        shader_tex_clr_nrm.SetUniform("is_color", true); GL_CHECK_ERRORS
        
        //VIEW
        glm::mat4 view(1.0f);
        view = glm::lookAt(camera_pos,camera_pos + camera_front,camera_up);
        shader_tex_clr_nrm.SetUniform("view", view); GL_CHECK_ERRORS
        //PROJECTION
        glm::mat4 projection(1.0f);
        projection = glm::perspective(glm::radians(45.0f), float(width / height), 0.1f, 100.0f);
        shader_tex_clr_nrm.SetUniform("projection", projection); GL_CHECK_ERRORS
        
        skybox.draw(shader_tex_clr_nrm, skybox_textures); 
        
        
        if (particles.size() < 30) {
            float r = random(0, 5);
            if ((r > 0) && (r < 1)) {
                Particle p1, p2;
                p1.generate_obj(texts.particle, glm::vec3(random(0.0,1.0), random(0.0,1.0),random(0.0,1.0)));
                p2.generate_obj(texts.dust, glm::vec3(random(0.0,1.0), random(0.0,1.0),random(0.0,1.0)));
                particles.push_back(p1);
                particles.push_back(p2);
            }
        }
        for (auto it=particles.begin(); it != particles.end(); it++) {
            it->check_alive();
            if (!player.game_over)
                it->draw(shader_tex_clr_nrm);
        }
        
        
        if (enemies.size() < 4) {
            float r = random(-40, 40);
            int t = enemies.size();
            if ((r > 0) && (r < 1)) {
                Enemy enem;
                switch (t) {
                    case 0:
                        enem.generate_obj(texts.enemy1, texts.dang_shot1, texts.bang_enemy1, 1.0);
                        break;
                    case 1:
                        enem.generate_obj(texts.enemy2, texts.dang_shot2, texts.bang_enemy2, 0.8);
                        break;
                    case 2:
                        enem.generate_obj(texts.enemy3, texts.dang_shot3, texts.bang_enemy3, 1.1);
                        break;
                    case 3:
                        enem.generate_obj(texts.enemy1, texts.dang_shot3, texts.bang_enemy1, 1.1);
                        break;
                }
                enemies.push_back(enem);
            }
        }
        shader_tex_clr_nrm.SetUniform("is_color", false); GL_CHECK_ERRORS
        for (auto it=enemies.begin(); it != enemies.end(); it++) {
            it->check_alive();
            if (!(it->killed) && !player.game_over) {
                it->draw(shader_tex_clr_nrm);
                if (!(it->bang))
                    for (auto it_shots = it->dang_shots.begin(); it_shots != it->dang_shots.end(); it_shots++) {
                        if (it_shots->shooted) {
                            if (!(it_shots->check_hit()) && !player.game_over) {
                                it_shots->draw(shader_tex_clr_nrm);
                            }
                        }
                    }
            }
        }
        
        
        shader_tex_clr_nrm.SetUniform("is_color", false); GL_CHECK_ERRORS
        if (asters.size() < 5) {
            float r = random(-40, 40);
            int t = asters.size();
            if ((r > 0) && (r < 1)) {
                Asteroid aster;
                switch (t) {
                    case 1:
                        aster.generate_obj(TRIANGLE, texts.asteroid2);
                        break;
                    case 2:
                        aster.generate_obj(TRIANGLE, texts.asteroid1);
                        break;
                    case 3:
                        aster.generate_obj(TRIANGLE, texts.asteroid2);
                        break;
                    case 4:
                        aster.generate_obj(TRIANGLE, texts.asteroid1);
                        break;
                }
                asters.push_back(aster);
            }
        }
        for (auto it=asters.begin(); it != asters.end(); it++) {
            it->check_alive();
            if (!player.game_over)
                it->draw(shader_tex_clr_nrm);
        }
        
        
        if (shooting) {
            shooting = false;
            bool flag = false;
            for (auto it=shots.begin(); it != shots.end(); it++) {
                if (!(it->shooted)) {
                    it->new_shot();
                    flag = true;
                    break;
                }
            }
            if (!flag) {
                Shot shot;
                shot.generate_obj();
                shots.push_back(shot);
            }
        }
        for (auto it=shots.begin(); it != shots.end(); it++) {
            if (it->shooted) {
                if (!(it->check_hit()) && !player.game_over) {
                    it->draw(shader_tex_clr_nrm, texts.shot1, texts.shot2, texts.shot3);
                }
            }
            
        }
        
        
        glEnable(GL_BLEND); GL_CHECK_ERRORS
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); GL_CHECK_ERRORS
        player.draw(shader_tex_clr_nrm, texts.health, texts.score, numbers_textures, texts.game_over, texts.sad_cat, texts.score_title, texts.damage, texts.damage_enemy, texts.damage_asteroid, texts.weapon,
                    texts.shot1, texts.shot2, texts.shot3);
        glDisable(GL_BLEND);
        
        //VAO ПРИЦЕЛА
        glBindVertexArray(aim_VAO); GL_CHECK_ERRORS
        shader_tex_clr_nrm.SetUniform("is_color", false); GL_CHECK_ERRORS
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(aim_coord_x+camera_pos.x, aim_coord_y+camera_pos.y, camera_pos.z-2.65));
        model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
        shader_tex_clr_nrm.SetUniform("model", model); GL_CHECK_ERRORS
        glBindTexture(GL_TEXTURE_2D, texts.aim); GL_CHECK_ERRORS
        shader_tex_clr_nrm.SetUniform("fragm_text", 0); GL_CHECK_ERRORS

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); GL_CHECK_ERRORS
        glBindVertexArray(0); GL_CHECK_ERRORS;
        
        shader_tex_clr_nrm.StopUseShader(); GL_CHECK_ERRORS

        
        
		glfwSwapBuffers(window);
	}

	//очищаем vboи vao перед закрытием программы
    //
    
    for (auto it = all_VAOs.begin(); it != all_VAOs.end(); it++){
        glDeleteVertexArrays(1, &(*it));
    }
    
    for (auto it = all_VBOs.begin(); it != all_VBOs.end(); it++){
        glDeleteBuffers(1, &(*it));
    }

	glfwTerminate();
	return 0;
}
