#include "SimpleCamera.h"
#include "../../util/InputUtil.h"
#include <imgui/imgui.h>
#include "../../util/Logger.h"

void SimpleCamera::update(double dt)
{
    keyboard_blocked = false;

    if (!ImGui::IsAnyItemFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsAnyWindowFocused())
    {
        bool moved = false;
        // Motion
        if (glfwGetMouseButton(input->window, GLFW_MOUSE_BUTTON_2))
        {
            keyboard_blocked = true;

            if (glfwGetKey(input->window, GLFW_KEY_W) == GLFW_PRESS)
            {
                forwards(dt);
                moved = true;
            }

            if (glfwGetKey(input->window, GLFW_KEY_S) == GLFW_PRESS)
            {
                backwards(dt);
                moved = true;
            }
            if (mode==CAMERA_FREE_MODE)
            {
                if (glfwGetKey(input->window, GLFW_KEY_A) == GLFW_PRESS)
                {
                    leftwards(dt);
                    moved = true;
                }

                if (glfwGetKey(input->window, GLFW_KEY_D) == GLFW_PRESS)
                {
                    rightwards(dt);
                    moved = true;
                }

                if (glfwGetKey(input->window, GLFW_KEY_R) == GLFW_PRESS)
                {
                    upwards(dt);
                    moved = true;
                }

                if (glfwGetKey(input->window, GLFW_KEY_F) == GLFW_PRESS)
                {
                    downwards(dt);
                    moved = true;
                }

                if (glfwGetKey(input->window, GLFW_KEY_Q) == GLFW_PRESS)
                {
                    tilt(dt, -1.0f);
                }

                if (glfwGetKey(input->window, GLFW_KEY_E) == GLFW_PRESS)
                {
                    tilt(dt, 1.0f);
                }
            }
            glfwSetInputMode(input->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (input->mouse_delta != glm::dvec2(0.0, 0.0))
            {
                mouse(input->mouse_delta, dt);
            }

            if (input->mouse_scroll_delta != 0)
            {
                speed += speed * input->mouse_scroll_delta * 0.05;
            }
        }
        else
        {
            glfwSetInputMode(input->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    else
    {
        keyboard_blocked = true;
        glfwSetInputMode(input->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

std::pair<glm::dvec3, glm::dvec3> SimpleCamera::get_camera_pos_dir()
{
    return std::make_pair(pos + center, fw);
}

void SimpleCamera::forwards(double dt)
{
    switch(mode)
    {
    case CAMERA_FREE_MODE:
        pos += speed * glm::normalize(fw) * (double)dt;
        break;
    case CAMERA_CIRCLE_MODE:
        distance-=speed * (double)dt;
        toSphere();
        break;
    }
}

void SimpleCamera::backwards(double dt)
{
    switch(mode)
    {
    case CAMERA_FREE_MODE:
        pos -= speed * glm::normalize(fw) * (double)dt;
        break;
    case CAMERA_CIRCLE_MODE:
        distance+=speed * (double)dt;
        toSphere();
        break;
    }
}

void SimpleCamera::leftwards(double dt)
{
    pos -= speed * glm::normalize(glm::cross(fw, up)) * (double)dt;
}

void SimpleCamera::rightwards(double dt)
{
    pos += speed * glm::normalize(glm::cross(fw, up)) * (double)dt;
}

void SimpleCamera::tilt(double dt, double dir)
{
    glm::mat4 f = glm::rotate(glm::dmat4(1.0), (double)dir * (double)dt, fw);
    up = f * glm::dvec4(up, 1.0);
}

void SimpleCamera::upwards(double dt)
{
    pos += speed * glm::normalize(up) * (double)dt;
}

void SimpleCamera::downwards(double dt)
{
    pos -= speed * glm::normalize(up) * (double)dt;
}

glm::dmat4 SimpleCamera::get_cmodel()
{
    return glm::translate(glm::dmat4(1.0), -get_camera_pos_dir().first);
}

void SimpleCamera::set_mode(unsigned char mode)
{
    this->mode=mode;
}

void SimpleCamera::init(glm::dvec3 up)
{
    fov = 60.0;
    speed = 20.0;
    center_fw = glm::dvec3(0.0, 0.0, 0.0);
    distance=10.0;
    this->up=up;
    pos = glm::normalize(glm::dvec3(0.0, distance, 0.0)+up*5.0)*distance;
    fw = glm::normalize(-pos);
}

void SimpleCamera::toSphere()
{
    fw=glm::normalize(-pos);
    pos=fw*-distance;
}

void SimpleCamera::mouse(glm::dvec2 deltas, double dt)
{
    switch(mode)
    {
    case CAMERA_FREE_MODE:
    {
        glm::dvec3 or_forward = fw;
        glm::dvec3 or_up = up;

        // Rotate forward original up
        glm::mat4 hor = glm::rotate(glm::dmat4(1.0), -deltas.x * (double)dt * 0.45, or_up);

        glm::dvec3 right = glm::cross(or_forward, or_up);

        glm::mat4 vert = glm::rotate(glm::dmat4(1.0), -deltas.y * (double)dt * 0.45, right);
        fw = vert * hor * glm::dvec4(fw, 1.0);
        break;
    }
    case CAMERA_CIRCLE_MODE:
        pos += speed * glm::normalize(up) * deltas.y * (double)dt * 0.2 - speed * glm::cross(fw, up) * deltas.x * (double)dt * 0.2;
        toSphere();
        break;
    }
}

void SimpleCamera::key_callback(GLFWwindow* , int key, int , int action, int )
{
    if (glfwGetKey(input->window, GLFW_KEY_C) == GLFW_PRESS) { mode=!mode; }
}

void key_callback_bounce(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ((SimpleCamera *)glfwGetWindowUserPointer(window))->key_callback(window,key,scancode,action,mods);
}

SimpleCamera::SimpleCamera()
{
    keyboard_blocked=false;
    set_mode(CAMERA_CIRCLE_MODE);
    glfwSetWindowUserPointer(input->window,this);
    glfwSetKeyCallback(input->window,&key_callback_bounce);
}

glm::dmat4 SimpleCamera::get_proj_view(int width, int height)
{
    auto[camera_pos, camera_dir] = get_camera_pos_dir();

    // ~1 light year
    float far_plane = 1e16f;


    glm::dmat4 proj = glm::perspective(glm::radians(fov), (double)width / (double)height, NEAR_PLANE, (double)far_plane);
    glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, up);
    glm::dmat4 proj_view = proj * view;

    return proj_view;
}

CameraUniforms SimpleCamera::get_camera_uniforms(int w, int h)
{
    CameraUniforms out;

    auto[camera_pos, camera_dir] = get_camera_pos_dir();

    // ~1 light year
    float far_plane = 1e16f;

    glm::dmat4 proj = glm::perspective(glm::radians(fov), (double)w / (double)h, NEAR_PLANE, (double)far_plane);
    glm::dmat4 view = glm::lookAt(glm::dvec3(0.0, 0.0, 0.0), camera_dir, up);
    //glm::dmat4 view = glm::lookAt(-camera_pos, camera_dir, up);
    glm::dmat4 proj_view = proj * view;

    out.proj = proj;
    out.view = view;
    out.proj_view = proj_view;
    out.c_model = glm::translate(glm::dmat4(1.0), -camera_pos);
    out.tform = proj * view * out.c_model;
    out.far_plane = far_plane;
    out.cam_pos = camera_pos;

    out.screen_size = glm::vec2((float)w, (float)h);
    out.iscreen_size = glm::ivec2(w, h);

    return out;
}
