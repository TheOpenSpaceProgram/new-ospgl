#pragma once
#include <renderer/camera/Camera.h>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <gui/GUIInput.h>

// The camera can be configured, but default settings should be familiar:
// Right-click rotates the camera around the center
// Shift-Right-click moves the center along the current camera view plane (panning)
// Ctrl-Right-click moves the center along the vehicle plane (O-Z-X)
// Shift-Ctrl-Right-click moves the center along the alternative vehicle plane (O-Z-Y)
// Scrollwheel moves the camera up-down / forward-backwards (+Z-Z)
// Shift-Scrollwheel changes distance to the vehicle
class EditorCamera : public Camera
{
public:

	int main_button;
	int pan_key;
	int altpan_key;	
	int zoom_key;
	double pan_vel = 1.0;
	double rot_vel = 0.5;
	double zoom_vel = 7.0;	
	double zoom_attn = 8.0;
	double updown_vel = 4.0;
	double updown_attn = 6.0;


	// Scrolling stuff uses acceleration so it feels smooth
	double updown_delta{};
	double radius_delta{};

	// We block both the mouse and keyboard as we are an orbit camera
	// This means we succesfully started an orbit motions, and stays
	// until released
	bool blocked;

	// We implement a simple orbit camera
	glm::dvec3 center;
	glm::dvec3 direction;
	double radius;
	double fov;		//< In degrees

	glm::dvec3 up = glm::dvec3(0.0, 0.0, 1.0);

	void update(double game_dt, GUIInput* gui_input);

	virtual CameraUniforms get_camera_uniforms(int w, int h);

	EditorCamera();
};
