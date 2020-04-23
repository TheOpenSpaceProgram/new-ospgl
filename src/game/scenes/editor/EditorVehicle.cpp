#include "EditorVehicle.h"


void EditorVehicle::deferred_pass(CameraUniforms& cu)
{	
	for (Piece* p : veh->all_pieces)
	{
		glm::dmat4 tform = to_dmat4(p->get_graphics_transform());
		p->model_node->draw(cu, tform, drawable_uid, true);
	}
}

void EditorVehicle::forward_pass(CameraUniforms& cu)
{

}

void EditorVehicle::shadow_pass(ShadowCamera& cu)
{
	for(Piece* p : veh->all_pieces)
	{
		glm::dmat4 tform = to_dmat4(p->get_graphics_transform());
		p->model_node->draw_shadow(cu, tform, true);
	}
}
