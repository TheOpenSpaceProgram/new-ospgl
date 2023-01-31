#pragma once
#include "../PlanetarySystem.h"

class FrameOfReference
{
public:

	// If true, center_id refers to a solar system element
	// If false, center_id refers to an entity
	bool is_element;
	int center_id;

	// Does the frame rotate with the body?
	bool rotating;
	// Where on the body is the origin?
	// Relative to the unrotated body, if rotating is true, then it will rotate too
	glm::dvec3 offset;

	// These are used for prediction, as we store the reference body pos and vel
	glm::dvec3 get_rel_pos(glm::dvec3 abs_pos, glm::dvec3 body_pos, double body_rot);
	std::pair<glm::dvec3, glm::dvec3> get_rel_pos_vel(glm::dvec3 abs_pos, glm::dvec3 abs_vel, glm::dvec3 body_pos,
													  glm::dvec3 body_vel, double body_rot, double body_rotvel);

	// These use the universe
	glm::dmat4 get_tform_matrix();

	FrameOfReference();

};
