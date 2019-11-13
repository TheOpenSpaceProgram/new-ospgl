#pragma once

#include "../../assets/Image.h"
#include "../../assets/Shader.h"
#include "../../assets/AssetManager.h"
#include "../../util/render/Framebuffer.h"
#include "../../util/render/TextureDrawer.h"
#include "../../util/geometry/SphereGeometry.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

class Navball
{
public:

	GLuint ball_vao, ball_vbo, ball_ebo;
	size_t ball_index_count;

	Framebuffer fbuffer;

	Shader* navball_shader;
	Image* texture;
	Image* frame_tex;
	Image* prograde_tex;
	Image* retrograde_tex;
	Image* normal_tex;
	Image* antinormal_tex;
	Image* radialin_tex;
	Image* radialout_tex;

	float view_distance = 4.0f;
	// Used to fit the navball inside the frame texture
	float scale = 0.85f;

	// Actual scale of the UI element
	float final_scale = 1.0f;

	// Scale of the markers, scaled when the markers get near the edges
	float icon_scale = 0.3f;

	// The coordinate it's given is what the navball
	// will show at its center (the direction of the vessel)
	void draw_to_texture(glm::quat rot, glm::quat prog);

	// Draws the navball to the screen centered bottom
	void draw_to_screen(glm::ivec2 screen_size);

	Navball();
	~Navball();
};

