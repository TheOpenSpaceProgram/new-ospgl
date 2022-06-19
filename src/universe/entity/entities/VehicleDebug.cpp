#include "VehicleDebug.h"
#include <imgui/imgui.h>
#include "VehicleEntity.h"

void VehicleDebug::show_imgui()
{
	std::vector<Machine*> closed;
	for(auto it = shown_machines.begin(); it != shown_machines.end();)
	{
		bool opened = true;
		(*it)->draw_imgui(&opened);
		if(opened)
		{
			it++;
		}
		else
		{
			it = shown_machines.erase(it);
		}
	}

	ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_FirstUseEver);
	ImGui::Begin(("Vehicle (" + std::to_string(ent->get_uid()) + ")").c_str());

	if(ImGui::BeginTabBar("##tabs"))
	{
		if(ImGui::BeginTabItem("Parts"))
		{
			parts_tab();
			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Pieces"))
		{

			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Plumbing"))
		{

			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Wiring"))
		{

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
	ImGui::End();
}

void VehicleDebug::parts_tab()
{
	// We show all parts in the vehicle
	// Show all machines
	for(Part* p : ent->vehicle->parts)
	{
		ImGui::PushID(p);
		std::string name = "Part (" + p->part_proto->get_asset_id() + ") " + std::to_string(p->id);
		if(ImGui::CollapsingHeader(name.c_str()))
		{
			ImGui::Button("Focus");
			ImGui::Text("Machines: ");
			for(auto pair : p->get_all_machines())
			{
				ImGui::PushID(pair.second);
				ImGui::Text("\t%s(%s)", pair.first.c_str(),
							pair.second->get_id().c_str());
				ImGui::SameLine();
				if(vector_contains(shown_machines, pair.second))
				{
					if (ImGui::Button("Hide Imgui"))
					{
						shown_machines.erase(std::find(shown_machines.begin(),
													   shown_machines.end(), pair.second));
					}
				}
				else
				{
					if(ImGui::Button("Show Imgui"))
					{
						shown_machines.push_back(pair.second);
					}
				}
				ImGui::SameLine();
				ImGui::Button("Show Wiring");
				ImGui::SameLine();
				ImGui::Button("Show Plumbing");
				ImGui::PopID();
			}

		}
		ImGui::PopID();
	}

}
