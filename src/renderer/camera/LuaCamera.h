#pragma once
#include "Camera.h"
#include "CameraUniforms.h"
#include "util/LuaUtil.h"
#include <optional>
#include "OSP.h"
#include "game/GameState.h"

// This "camera" is used in all lua scenes and simply passes the
// get_camera_uniforms call to lua
class LuaCamera : public Camera
{
public:

	bool logged_error = false;
	sol::environment* env;

	// NOTE: Automatically fills widths
	virtual CameraUniforms get_camera_uniforms(int w, int h)
	{
		// This is the one that can be overriden
		if(osp->game_state->debug.override_camera)
		{
			return osp->game_state->debug.cam.get_camera_uniforms(w, h);
		}
		else
		{
			auto res = LuaUtil::call_function_if_present_returns<CameraUniforms>((*env)["get_camera_uniforms"], w, h);
			if (!res.has_value())
			{
				if (!logged_error)
				{
					logger->error("Scene has no camera function / didn't return CameraUniforms, rendering will not work!");
					logged_error = true;
				}
				return CameraUniforms();
			}
			CameraUniforms out = res.value();
			out.screen_size = glm::vec2((float) w, (float) h);
			out.iscreen_size = glm::ivec2(w, h);

			return out;
		}
	}

	explicit LuaCamera(sol::environment* env)
	{
		this->env = env;
	}
};

