#include "LuaImGUI.h"
#include <imgui/imgui.h>
#include <util/Logger.h>

void LuaImGUI::load_to(sol::table &table)
{
	table.set_function("begin", sol::overload(
			[](const std::string& str){ ImGui::Begin(str.c_str());}
			));
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
	table.set_function("button", [](const std::string& str)
	{
		return ImGui::Button(str.c_str());
	});
	table.set_function("same_line", [](){ImGui::SameLine();});
	table.set_function("tree_node", sol::overload<bool(const char*)>(ImGui::TreeNode));
	table.set_function("tree_pop", ImGui::TreePop);
	// Slightly different behaviour due to limitation, use as value = imgui.checkbox("wathever", value)
	table.set_function("checkbox", [](const std::string& text, bool b)
	{
		ImGui::Checkbox(text.c_str(), &b);
		return b;
	});

	// Allows using normal integers and strings (use tostring with other types)
	table.set_function("push_id", sol::overload(
			sol::resolve<void(int)>(ImGui::PushID),
			[](const std::string& str)
			{
				logger->info("Pushed: {}", str);
				ImGui::PushID(str.c_str());
			}));

	table.set_function("pop_id", ImGui::PopID);

	// TODO: Overload that takes flags
	table.set_function("collapsing_header", [](const std::string& text)
	{
		return ImGui::CollapsingHeader(text.c_str());
	});

}
