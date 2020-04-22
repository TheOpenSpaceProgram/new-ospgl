#pragma once 
#include <renderer/GBuffer.h>
#include <util/render/Framebuffer.h>
#include <assets/PartPrototype.h>
#include <renderer/lighting/PartIconLight.h>

// Renders part prototypes to a framebuffer only when needed,
// used for the part icons in the editor
// We use a shared framebuffer for all and copy the generated texture
// after rendering so some GPU memory can be saved by not having [X] 
// frambuffers, each with all the memory overhead
// Renders the parts using the deferred renderer for maximum quality
class PartIconRenderer
{
public:

	PartIconLight light;

	// IN RADIANS
	double fov;
	glm::ivec2 size;

	// There is only one of these for all parts
	GBuffer gbuffer;
	
	Framebuffer fbuffer;

	// After rendering, data is copied to the target texture
	// We fit the part as best as possible into the viewport using its
	// physics AABB
	void render(PartPrototype* proto, double angle, GLuint target);

	GLuint create_texture();

	// Fov already in radians!
	PartIconRenderer(glm::ivec2 size, double fov = glm::radians(20.0));

};
