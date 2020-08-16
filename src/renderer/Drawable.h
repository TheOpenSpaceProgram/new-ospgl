#pragma once
#include <glm/glm.hpp>
#include "camera/CameraUniforms.h"
#include "lighting/ShadowCamera.h"
#include <string>

class Renderer;

// Base class for all stuff that can be drawn
class Drawable
{
private:

	bool added;
	std::string drawable_id;

protected:

	GLint drawable_uid;

public:

	virtual void deferred_pass(CameraUniforms& cu){}
	virtual void forward_pass(CameraUniforms& cu){}
	virtual void gui_pass(CameraUniforms& cu) {}
	virtual void shadow_pass(ShadowCamera& cu) {}
	virtual void far_shadow_pass(ShadowCamera& cu) {}

	virtual bool needs_deferred_pass() { return false; }
	virtual bool needs_forward_pass() { return false; }
	virtual bool needs_gui_pass() { return false; }
	virtual bool needs_shadow_pass() { return false; }
	virtual bool needs_far_shadow_pass() { return false; }

	bool is_in_renderer() 
	{
		return added;
	}

	void notify_add_to_renderer(GLint uid)
	{
		added = true;
		drawable_uid = uid;
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
