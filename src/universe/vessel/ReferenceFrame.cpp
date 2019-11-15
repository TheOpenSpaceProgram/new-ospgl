#include "ReferenceFrame.h"

glm::dvec3 ReferenceFrame::relative_velocity(glm::dvec3 vel) const
{
	return vel - center.get_velocity_now();
}
