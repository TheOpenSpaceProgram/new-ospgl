#pragma once
#include <glm/glm.hpp>
#include "camera/CameraUniforms.h"

// Base class for all stuff that can be drawn
class Drawable
{
private:

	bool added;
	std::string drawable_id;

public:

	virtual void deferred_pass(glm::ivec2 size, CameraUniforms& cu){}
	virtual void forward_pass(glm::ivec2 size, CameraUniforms& cu){}
	virtual void gui_pass(glm::ivec2 size, CameraUniforms& cu) {}
	
	virtual bool needs_deferred_pass() { return false; }
	virtual bool needs_forward_pass() { return false; }
	virtual bool needs_gui_pass() { return false; }

	bool is_in_renderer() 
	{
		return added;
	}

	void notify_add_to_renderer()
	{
		added = true;
	}

	void notify_remove_from_renderer()
	{
		added = false;
	}
	
	void set_drawable_id(const std::string& id)
	{
		drawable_id = id;
	}

	std::string get_drawable_id()
	{
		return drawable_id;
	}

	Drawable()
	{
		added = false;
	}
};