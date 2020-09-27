#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <tuple>
#include <utility>
#include <unordered_map>
#include "CameraUniforms.h"
#include "Camera.h"
typedef struct GLFWwindow GLFWwindow;

#define CAMERA_FREE_MODE 0
#define CAMERA_CIRCLE_MODE 1

class SimpleCamera : public Camera
{
private:
        static SimpleCamera *instance;

	glm::dmat4 get_proj_view(int w, int h);
	glm::dmat4 get_cmodel();
public:

	constexpr static double NEAR_PLANE = 1e-6;

        bool keyboard_blocked;
	unsigned char mode;

	// In degrees
	double fov;

	double speed;

	double distance;

	glm::dvec3 pos;
	glm::dvec3 up;
	glm::dvec3 fw;

	glm::dvec3 center;
	glm::dvec3 center_fw;

	void update(double dt);

	// Relative to system, not to center planet
	std::pair<glm::dvec3, glm::dvec3> get_camera_pos_dir();

	virtual CameraUniforms get_camera_uniforms(int w, int h) override;
        void forwards(double dt);
        void backwards(double dt);
        void leftwards(double dt);
        void rightwards(double dt);
        void tilt(double dt, double dir);
        void upwards(double dt);
        void downwards(double dt);
	void toSphere();
	void mouse(glm::dvec2 deltas, double dt);
	void set_mode(unsigned char mode);
	void init(glm::dvec3 up);
	SimpleCamera();
        void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
