#include "LuaRenderer.h"
#include "renderer/Renderer.h"

void LuaRenderer::load_to(sol::table& table)
{
	// We use sol's convenient class passing so the user doesn't need to specify
	// which renderer to use (as there's always only one renderer)
	table.set_function("add_drawable", &Renderer::add_drawable, osp->renderer);
	table.set_function("remove_drawable", &Renderer::remove_drawable, osp->renderer);
	table.set_function("add_light", &Renderer::add_light, osp->renderer);
	table.set_function("remove_light", &Renderer::remove_light, osp->renderer);

}
