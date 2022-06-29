#include "LuaImGUI.h"
#include <imgui/imgui.h>

void LuaImGUI::load_to(sol::table &table)
{
	table.set_function("begin", ImGui::Begin);
	// Can't have stuff named table.set_function("end" in lua!
	table.set_function("endw",  ImGui::End);
	table.set_function("text", [](const std::string& str)
	{
		ImGui::Text("%s", str.c_str());
	});
	table.set_function("text_colored", [](float r, float g, float b, const std::string& str)
	{
		ImGui::TextColored(ImVec4(r, g, b, 1.0), "%s", str.c_str());
	});
	table.set_function("bullet_text", [](const std::string& str)
	{
	   ImGui::BulletText("%s", str.c_str());
    });
	table.set_function("separator", ImGui::Separator);
	table.set_function("same_line", ImGui::SameLine);
	table.set_function("tree_node", sol::overload<bool(const char*)>(ImGui::TreeNode));
	table.set_function("tree_pop", ImGui::TreePop);
	// Slightly different behaviour due to limitation, use as value = imgui.checkbox("wathever", value)
	table.set_function("checkbox", [](const std::string& text, bool b)
	{
		ImGui::Checkbox(text.c_str(), &b);
		return b;
	});




}
