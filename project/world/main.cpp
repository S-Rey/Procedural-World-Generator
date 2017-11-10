// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// contains helper functions such as shader compiler
#include "icg_helper.h"

#include <glm/gtc/matrix_transform.hpp>

#include "fractal/fractal.h"
#include "grid/grid.h"
#include "framebuffer.h"
#include "cube/cube.h"
#include "water/water.h"
#include "bezier/bezier.h"

Water water;
Grid grid;
Cube cube;
Fractal fractal;
FrameBuffer framePerlin;
FrameBuffer framebuffer;
Bezier bezier;

int window_width = 800;
int window_height = 600;

using namespace glm;

mat4 projection_matrix;
mat4 orthographic_matrix;
mat4 translate_matrix;
mat4 rotation_y_matrix;
mat4 rotation_x_matrix;
mat4 view_matrix;
mat4 quad_model_matrix;
mat4 cube_scale;

vec3 cam_pos = vec3(2.0f, 2.0f, 10.0f);
vec3 cam_look = vec3(0.0f, 0.0f, -1.0f);
vec3 cam_up = vec3(0.0f, 1.0f, 0.0f);

bool shift, ctrl, fps, speedmode = false;
bool rotl, rotr, rotu, rotd = false;
bool strafl, strafr, forw, backward = false;
float speedf = 0;
float speedb = 0;
float speedY = 0;
float speedl = 0;
float speedr = 0;
int view_mode = 1;


mat4 OrthographicProjection(float left, float right, float bottom,
                            float top, float near, float far) {
    assert(right > left);
    assert(far > near);
    assert(top > bottom);
    mat4 projection = mat4(1.0f);
    projection[0][0] = 2.0f / (right - left);
    projection[1][1] = 2.0f / (top - bottom);
    projection[2][2] = -2.0f / (far - near);
    projection[3][3] = 1.0f;
    projection[3][0] = -(right + left) / (right - left);
    projection[3][1] = -(top + bottom) / (top - bottom);
    projection[3][2] = -(far + near) / (far - near);
    return projection;
}

mat4 PerspectiveProjection(float fovy, float aspect, float near, float far) {
    // Creation of perspective projection matrix given the field of view,
    // aspect ratio, and near and far plane distances.
    mat4 projection = IDENTITY_MATRIX;
    projection[0][0] = 1/(aspect*tan(fovy/2));
    projection[1][1] = 1/(tan(fovy/2));
    projection[2][2] = (-near-far)/(far-near);
    projection[3][2] = (-2*near*far)/(far-near);
    projection[3][3] = 0.0f;
    projection[2][3] = -1.0f;
    return projection;
}

mat4 LookAt(vec3 eye, vec3 center, vec3 up) {

    vec3 z_cam = normalize(eye - center);
    vec3 x_cam = normalize(cross(up, z_cam));
    vec3 y_cam = cross(z_cam, x_cam);

    mat3 R(x_cam, y_cam, z_cam);
    R = transpose(R);

    mat4 look_at(vec4(R[0], 0.0f),
                 vec4(R[1], 0.0f),
                 vec4(R[2], 0.0f),
                 vec4(-R * (eye), 1.0f));
    return look_at;
}

// find right perpendicular for rotation up and down
vec3 perp(vec3 v)
{
    return normalize(vec3(-v.z, 0, v.x));
}
// rotate a vector according to certain axis
vec3 rot(vec3 v, vec3 axis, float deg)
{
    vec4 result = glm::rotate(mat4(1.f), deg, axis) * vec4(v, 1);
    return vec3(result.x, result.y, result.z);
}

void Init() {
    // sets background color
    glClearColor(0.937, 0.937, 0.937 /*gray*/, 1.0 /*solid*/);
    
    // Framebuffer for the reflects
    GLuint framebuffer_texture_id_2 = framebuffer.Init(window_width, window_height, true);
    water.Init(framebuffer_texture_id_2);
    
    // Initialize the perlin framebuffer
    // The size is 1.0 because the perlin take all the window

    GLuint framebuffer_texture_id = framePerlin.Init(window_width, window_height, true);
    grid.Init(framebuffer_texture_id);
    fractal.Init();
    cube.Init();

    // enable depth test.
    glEnable(GL_DEPTH_TEST);

    // TODO 3: once you use the trackball, you should use a view matrix that
    // looks straight down the -z axis. Otherwise the trackball's rotation gets
    // applied in a rotated coordinate frame.
    // uncomment lower line to achieve this.
    view_matrix = lookAt(vec3(2.0f, 2.0f, 4.0f),
                         vec3(0.0f, 0.0f, 0.0f),
                         vec3(0.0f, 1.0f, 0.0f));
    translate_matrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -4.0f));
    rotation_y_matrix = IDENTITY_MATRIX;
    rotation_x_matrix = IDENTITY_MATRIX;
    
    cube_scale = mat4(0.25f, 0.0f,  0.0f,  0.0f,
                      0.0f,  0.25f, 0.0f,  0.0f,
                      0.0f,  0.0f,  0.25f, 0.0f,
                      0.0f,  0.0f,  0.0f,  1.0f);

    quad_model_matrix = translate(mat4(1.0f), vec3(0.0f, -0.25f, 0.0f));

    framePerlin.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    fractal.Draw();
    framePerlin.Unbind();
}

// gets called for every frame.
void Display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float time = glfwGetTime();

    
    //mat4 cube_transf = rotate(mat4(1.0f), 2.0f * time, vec3(0.0f, 1.0f, 0.0f));
    //cube_transf = translate(cube_transf, vec3(0.75f, 0.0f, 0.0f));
    //cube_transf = rotate(cube_transf, 2.0f * time, vec3(0.0f, 1.0f, 0.0f));
    
    mat4 cube_model_matrix = /*cube_transf * */ cube_scale;

    // Bezier
    if (bezier.run) {
        if (bezier.end_time > time) {
            struct camera_variables cam_t =
                    bezier.compute((time - bezier.begin_time) / bezier.running_time);
            cam_pos = cam_t.pos;
            cam_look = cam_t.look;
            cam_up = cam_t.up;
            view_matrix = LookAt(cam_pos, cam_look + cam_pos, cam_up);
        } else {
            bezier.run = false;
        }
    } else {
        // Move camera around according to what key is pressed
        if (rotl) {
            if (ctrl) {
                cam_pos = rot(cam_pos, vec3(0,-1,0), 3.1415/256);
            } else {
                cam_look = rot(cam_look, vec3(0,1,0), 3.1415/256);
            }
        }
        if (rotr) {
            if (ctrl) {
                cam_pos = rot(cam_pos, vec3(0,1,0), 3.1415/256);
            } else {
                cam_look = rot(cam_look, vec3(0,-1,0), 3.1415/256);
            }
        }
        if (rotu) {
            if (ctrl) {
                cam_pos = rot(cam_pos, perp(cam_pos), 3.1415/256);
            } else {
                if (cam_look.y <= 0.99) {
                    cam_look = rot(cam_look, perp(cam_look), 3.1415/256);
                } else if (cam_look.y >= 0.99) {
                    cam_look.y = 0.98;
                    cam_look = normalize(cam_look);
                }
            }
        }
        if (rotd) {
            if (ctrl) {
                cam_pos = rot(cam_pos, -perp(cam_pos), 3.1415/256);
            } else {
                if (cam_look.y >= -0.99) {
                    cam_look = rot(cam_look, -perp(cam_look), 3.1415/256);
                } else if (cam_look.y <= -0.99) {
                    cam_look.y = -0.98;
                    cam_look = normalize(cam_look);
                }
            }
        }


        if (view_mode != 3) {
            if (forw) {
                if(speedf <= 0.01)
                    speedf += 0.0004;
            }
            if (backward) {
                if(speedb <= 0.01)
                    speedb += 0.0004;
            }
            if (strafl) {
                if(speedl <= 0.01)
                    speedl += 0.0004;
            }
            if (strafr) {
                if(speedr <= 0.01)
                    speedr += 0.0004;
            }
        } else {
            if (forw) {
                speedf = 0.1;
            }
            if (backward) {
                speedb = 0.1;
            }
            if (strafl) {
                speedl = 0.1;
            }
            if (strafr) {
                speedr = 0.1;
            }
        }

        if (fps) {
            cam_pos += speedr * perp(cam_look) - speedl * perp(cam_look);

            if (view_mode == 3) {
                speedl = 0;
                speedr = 0;
            }

            if (speedl > 0)
                speedl -= 0.0002;
            else
                speedl = 0;
            if (speedr > 0)
                speedr -= 0.0002;
            else
                speedr = 0;
        }


        // Camera position
        if (ctrl) {
            cam_look = -cam_pos;
            view_matrix = LookAt(cam_pos, cam_look, cam_up);
        } else {

            // according to view mode just the y coord changes
            if (view_mode == 1 || view_mode == 3) {
                cam_pos.y += speedf * cam_look.y - speedb * cam_look.y;
            } else if (view_mode == 2) {
                float height;
                if (cam_pos.x > -30 && cam_pos.x < 30 && cam_pos.z > -30 && cam_pos.z < 30) {
                    framePerlin.Bind();
                    GLfloat array[3];
                    glReadBuffer(GL_COLOR_ATTACHMENT0);
                    glReadPixels((cam_pos.x/60 + 0.5)*window_width, (cam_pos.z/60 + 0.5)*window_height, 1, 1, GL_RGB, GL_FLOAT, array);
                    height = (sqrt(array[0]*array[0] + array[1]*array[1] + array[2]*array[2]) * 30.0 / 3.0) - 0.15*30;
                    framePerlin.Unbind();
                } else {
                    height = 0;
                }

                if (speedY > 0 || cam_pos.y > height) {
                    speedY -= 0.00025;
                    cam_pos.y += speedY;
                } else {
                    cam_pos.y = height;
                    speedY = 0;
                }
            }

            cam_pos.x += speedf * cam_look.x - speedb * cam_look.x;
            cam_pos.z += speedf * cam_look.z - speedb * cam_look.z;
            view_matrix = LookAt(cam_pos, cam_look + cam_pos, cam_up);

            if (view_mode == 3) {
                speedf = 0;
                speedb = 0;
            }

            if (speedf > 0) {
                speedf -= 0.0002;
            } else {
                speedf = 0;
            }
            if (speedb > 0) {
                speedb -= 0.0002;
            } else {
                speedb = 0;
            }
        }
    }

    
    // Change of the direction of the camera in order to "capture" the reflects
    
    vec3 cam_pos_mir = vec3(cam_pos.x, -cam_pos.y - 0.5, cam_pos.z);
    vec3 cam_look_mir = vec3(cam_look.x, -cam_look.y, cam_look.z);
    mat4 view_m = lookAt(cam_pos_mir, cam_look_mir + cam_pos_mir, cam_up);

    
    // Put the texture of the sky and the moutains for the reflects in the framebuffer
    framebuffer.Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    grid.Draw(time, quad_model_matrix, view_m, projection_matrix, true);
    cube.Draw(cube_model_matrix, view_m, projection_matrix);
    framebuffer.Unbind();
    

    // draw the grid, the water and the skybox

    grid.Draw(time, quad_model_matrix, view_matrix, projection_matrix, false);
    glEnable (GL_BLEND); // in order to have alpha transparency for the water
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    water.Draw(time, quad_model_matrix, view_matrix, projection_matrix);
    glDisable(GL_BLEND);
    cube.Draw(cube_model_matrix, view_matrix, projection_matrix);

}

// transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 TransformScreenCoords(GLFWwindow* window, int x, int y) {
    // the framebuffer and the window doesn't necessarily have the same size
    // i.e. hidpi screens. so we need to get the correct one
    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);
    return vec2(2.0f * (float)x / width - 1.0f,
                1.0f - 2.0f * (float)y / height);
}

void MouseButton(GLFWwindow* window, int button, int action, int mod) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double x_i, y_i;
        glfwGetCursorPos(window, &x_i, &y_i);
        vec2 p = TransformScreenCoords(window, x_i, y_i);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        double x_i, y_i;
        glfwGetCursorPos(window, &x_i, &y_i);
    }
}

void MousePos(GLFWwindow* window, double x, double y) {


    if (fps) {
        vec2 p = TransformScreenCoords(window, x, y);
        cam_look = rot(cam_look, vec3(0,-1,0), p.x * 3.1415 / 4);
        if (-0.99 <= cam_look.y && cam_look.y <= 0.99) {
            cam_look = rot(cam_look, perp(cam_look), p.y * 3.1415 / 8);
        } else if (cam_look.y <= -0.99) {
            cam_look.y = -0.98;
            cam_look = normalize(cam_look);
        } else if (cam_look.y >= 0.99) {
            cam_look.y = 0.98;
            cam_look = normalize(cam_look);
        }
        glfwSetCursorPos(window, window_width/2, window_height/2);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        vec2 p = TransformScreenCoords(window, x, y);
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        vec2 p = TransformScreenCoords(window, x, y);
    }
}

// Gets called when the windows/framebuffer is resized.
void SetupProjection(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    cout << "Window has been resized to "
         << window_width << "x" << window_height << "." << endl;

    glViewport(0, 0, window_width, window_height);

    framebuffer.Cleanup();
    framebuffer.Init(window_width, window_height);

    // TODO 1: Use a perspective projection instead;
    projection_matrix = PerspectiveProjection(45.0f,(GLfloat)window_width /window_height,0.1f, 100.0f);
    GLfloat top = 1.0f;
    GLfloat right = (GLfloat)window_width / window_height * top;
    orthographic_matrix = OrthographicProjection(-right, right, -top, top, -10.0, 10.0f);
    //projection_matrix = OrthographicProjection(-right, right, -top, top, -10.0, 10.0f);

}

void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // hold down shift to zoom
    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS) {
        shift = true;
    } else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE) {
        shift = false;
    }

    // activate control mode
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
        ctrl = !ctrl;
        if (ctrl)
            cam_pos = vec3(0.0f, 10.0f, 30.0f);
        else
            cam_look = vec3(0.0f, 0.0f, -1.0f);

    }

    // activate fps mode
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        fps = !fps;
        if (fps) {
            glfwSetCursorPos(window, window_width/2, window_height/2);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // choose camera mode
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        view_mode = 1;
    } else if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        view_mode = 2;
    } else if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        view_mode = 3;
    }

    // if shift is pressed
    if (shift) {
        if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS) ) {
            cam_pos -= vec3(0.1f) * cam_pos;
        } else if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS) ) {
            cam_pos += vec3(0.1f) * cam_pos;
        }
    // then if ctrl mode not activated
    } else if (!ctrl) {
        if (key == GLFW_KEY_A && action == GLFW_PRESS ) {
            if (fps)
                strafl = true;
            else
                rotl = true;
        } else if (key == GLFW_KEY_A && action == GLFW_RELEASE ) {
                strafl = false;
                rotl = false;
        } else if (key == GLFW_KEY_D && action == GLFW_PRESS ) {
            if (fps)
                strafr = true;
            else
                rotr = true;
        } else if (key == GLFW_KEY_D && action == GLFW_RELEASE ) {
            strafr = false;
            rotr = false;
        } else if (key == GLFW_KEY_E && action == GLFW_PRESS ) {
            rotu = true;
        } else if (key == GLFW_KEY_E && action == GLFW_RELEASE ) {
            rotu = false;
        } else if (key == GLFW_KEY_Q && action == GLFW_PRESS ) {
            rotd = true;
        } else if (key == GLFW_KEY_Q && action == GLFW_RELEASE ) {
            rotd = false;
        } else if (key == GLFW_KEY_W && action == GLFW_PRESS ) {
            forw = true;
        } else if (key == GLFW_KEY_W && action == GLFW_RELEASE ) {
            forw = false;
        } else if (key == GLFW_KEY_S && action == GLFW_PRESS ) {
            backward = true;
        } else if (key == GLFW_KEY_S && action == GLFW_RELEASE ) {
            backward = false;
        } else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            speedY = 0.015;
        }
    }else {
        if (key == GLFW_KEY_LEFT && action == GLFW_PRESS ) {
            rotl = true;
        } else if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE ) {
            rotl = false;
        } else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS ) {
            rotr = true;
        } else if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE ) {
            rotr = false;
        } else if (key == GLFW_KEY_UP && action == GLFW_PRESS ) {
            rotu = true;
        } else if (key == GLFW_KEY_UP && action == GLFW_RELEASE ) {
            rotu = false;
        } else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS ) {
            rotd = true;
        } else if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE ) {
            rotd = false;
        }
    }

    // bezier mode
    if(key == GLFW_KEY_C && action == GLFW_PRESS && !bezier.run) {
        cout << "Bezier cleared" <<endl;
        bezier.clear();
    }
    if(key == GLFW_KEY_V && action == GLFW_PRESS && bezier.on) {
        cout << "Add position" << endl;
        bezier.add(cam_pos, cam_look, cam_up);
    }
    if(key == GLFW_KEY_B && action == GLFW_PRESS && bezier.on && bezier.enough_measures()) {
        cout << "Launch Bezier" << endl;
        bezier.begin_time = glfwGetTime();
        bezier.end_time = bezier.begin_time + bezier.running_time;
        bezier.running_time = 1.0 * bezier.size();
        bezier.run = true;

        cout << bezier.begin_time << " " <<
                bezier.end_time << " " <<
                bezier.running_time << endl;
    }
}


int main(int argc, char *argv[]) {
    // GLFW Initialization
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(ErrorCallback);

    // hint GLFW that we would like an OpenGL 3 context (at least)
    // http://www.glfw.org/faq.html#how-do-i-create-an-opengl-30-context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // attempt to open the window: fails if required version unavailable
    // note some Intel GPUs do not support OpenGL 3.2
    // note update the driver of your graphic card
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          "CG Project", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);

    // set the callback for escape key
    glfwSetKeyCallback(window, KeyCallback);

    // set the framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, SetupProjection);

    // set the mouse press and position callback
    glfwSetMouseButtonCallback(window, MouseButton);
    glfwSetCursorPosCallback(window, MousePos);

    // GLEW Initialization (must have a context)
    // https://www.opengl.org/wiki/OpenGL_Loading_Library
    glewExperimental = GL_TRUE; // fixes glew error (see above link)
    if(glewInit() != GLEW_NO_ERROR) {
        fprintf( stderr, "Failed to initialize GLEW\n");
        return EXIT_FAILURE;
    }

    cout << "OpenGL" << glGetString(GL_VERSION) << endl;

    // initialize our OpenGL program
    Init();

    // update the window size with the framebuffer size (on hidpi screens the
    // framebuffer is bigger)
    glfwGetFramebufferSize(window, &window_width, &window_height);
    SetupProjection(window, window_width, window_height);

    // render loop
    while(!glfwWindowShouldClose(window)){
        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    grid.Cleanup();
    fractal.Cleanup();
    cube.Cleanup();
    

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
