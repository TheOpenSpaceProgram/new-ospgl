#pragma once
#include <renderer/camera/Camera.h>

class EditorCamera : public Camera
{
public:

	virtual CameraUniforms get_camera_uniforms(int w, int h);

};
