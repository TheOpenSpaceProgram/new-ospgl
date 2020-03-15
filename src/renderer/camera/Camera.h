#pragma once
#include "CameraUniforms.h"

class Camera
{
public:

	virtual CameraUniforms get_camera_uniforms(int w, int h) = 0;
};