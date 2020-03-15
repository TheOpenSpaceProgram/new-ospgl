#pragma once
#include <glm/glm.hpp>
#include "camera/CameraUniforms.h"

// Base class for all stuff that can be drawn
class Drawable
{
private:

	std::string id;

public:

	virtual void deferred_pass(glm::ivec2 size, CameraUniforms& cu){}
	virtual void forward_pass(glm::ivec2 size, CameraUniforms& cu){}
	virtual void gui_pass(glm::ivec2 size, CameraUniforms& cu) {}
	
	virtual bool needs_deferred_pass() { return false; }
	virtual bool needs_forward_pass() { return false; }
	virtual bool needs_gui_pass() { return false; }

	bool is_added()
	{
		return this->id != "";
	}

	void set_id(const std::string& id)
	{
		this->id = id;
	}

	std::string get_id()
	{
		return this->id;
	}

	Drawable()
	{
		id = "";
	}
};