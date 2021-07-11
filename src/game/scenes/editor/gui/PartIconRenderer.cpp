#include "PartIconRenderer.h"
#include <renderer/Renderer.h>
#include <assets/Cubemap.h>

PartIconRenderer::PartIconRenderer(glm::ivec2 size, double fov) : 
	gbuffer((size_t)size.x, (size_t)size.y),
	fbuffer((size_t)size.x, (size_t)size.y),
	cubemap("debug_system:skybox.hdr")
{
	this->size = size;
	this->fov = fov;

}


void PartIconRenderer::render(PartPrototype* proto, double angle, GLuint target)
{
	// This is safe to do as it's only done once, so noconst is safe
	if(cubemap->specular == nullptr)
	{
		cubemap.get_noconst()->generate_ibl_irradiance(32, 32);
	}

	constexpr double BIG_NUMBER = 9999999999.0;
	// Find part AABB
	glm::dvec3 min = glm::dvec3(BIG_NUMBER, BIG_NUMBER, BIG_NUMBER);	
	glm::dvec3 max = glm::dvec3(-BIG_NUMBER, -BIG_NUMBER, -BIG_NUMBER);

	for(auto& pair : proto->pieces)
	{
		btTransform t = pair.second.collider_offset;
		btVector3 nmin, nmax;
		pair.second.collider->getAabb(t, nmin, nmax);

		if(nmin.x() < min.x){ min.x = nmin.x(); }
		if(nmin.y() < min.y){ min.y = nmin.y(); }
		if(nmin.z() < min.z){ min.z = nmin.z(); }
		if(nmax.x() > max.x){ max.x = nmax.x(); }
		if(nmax.y() > max.y){ max.y = nmax.y(); }
		if(nmax.z() > max.z){ max.z = nmax.z(); }
	}

	// Find bounding sphere so we can do a simple trigonometric calculation
	double radius = glm::max(max.x - min.x, glm::max(max.y - min.y, max.z - min.z)) / 2.0;
	glm::dvec3 origin = (max + min) / 2.0;

	// Knowing FOV and bounding sphere we can calculate camera distance so the
	// part fits perfectly in the camera view
	
	double cam_distance = radius / glm::sin(fov / 2.0); 

	cam_distance *= 1.6;

	glm::dvec3 cam_pos = glm::dvec3(cam_distance, 0.0, 0.0);
	
	glm::dmat4 proj = glm::perspective(fov, (double)size.x / (double)size.y, 0.001, 1000.0);
	glm::dmat4 view = glm::lookAt(glm::dvec3(0, 0, 0), glm::dvec3(-1.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, -1.0));
	glm::dmat4 model = glm::translate(glm::dmat4(1.0), -origin); 
	model = glm::rotate(model, angle, glm::dvec3(0.0, 0.0, 1.0));

	glm::dmat4 c_model = glm::translate(glm::dmat4(1.0), -cam_pos);
	light.sun_dir = glm::normalize(glm::dvec3(1.0, 0.5, 0.0));

	CameraUniforms cu;
	cu.cam_pos = cam_pos;
	cu.c_model = c_model;
	cu.far_plane = 1000.0f;
	cu.iscreen_size = size;
	cu.proj = proj;
	cu.proj_view = proj * view;
	cu.screen_size = (glm::vec2)size;
	cu.tform = proj * view * c_model;
	cu.view = view;
	cu.irradiance = cubemap->irradiance->id;
	cu.specular = cubemap->specular->id;
	cu.brdf = osp->renderer->get_brdf_id();

	// Now we can do the rendering
	glDisable(GL_BLEND);

	gbuffer.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for(auto piece : proto->pieces)
	{
		glm::dmat4 sub_model = piece.second.piece_offset;
		piece.second.model_node->draw(cu, model * sub_model, 0, true);
	}
	gbuffer.unbind();

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);

	// This guarantees that alpha is "overwritten"
	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);
	glBlendEquation(GL_FUNC_ADD);

	// Draw to the framebuffer and copy the texture
	fbuffer.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	light.do_pass(cu, &gbuffer);	
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbuffer.fbuffer);
	glBindTexture(GL_TEXTURE_2D, target);
	// We use glCopyTexSubImage2D instead of glCopyImageSubData because the latter doesn't
	// seem to work properly (atleast on Intel GPU Mesa drivers, or I don't know how it works!)
	// Perfomance should be the same, but could use testing if this ends up being a bottleneck
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, size.x, size.y);

	fbuffer.unbind();	

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glBindTexture(GL_TEXTURE_2D, target);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

}	


GLuint PartIconRenderer::create_texture()
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)size.x, (GLsizei)size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	return tex;
}
