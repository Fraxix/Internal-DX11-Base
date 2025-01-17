#include "pch.h"
#include "Engine.h"
#include "Menu.h"

ImGuiWindowFlags Flags = 0;

namespace DX11Base {

	void Menu::Render() 
	{
		if (g_Engine->bShowMenu)
		{
			DrawMenu();
		}
	}

	void Menu::DrawMenu() 
	{
		if (ImGui::Begin("DX11 Base", NULL, Flags))
		{
			ImGui::Text("This is my DX11 Window!");
			ImGui::Text("Press [END] or the Button to Unload the DLL!");
			if(ImGui::Button("Uninject DLL")){g_KillSwitch = TRUE;}
			ImGui::Checkbox("Checkbox", &g_Engine->TestBool);
			ImGui::Spacing();
			ImGui::SmallCheckbox("Smaller Checkbox", &g_Engine->TestBool);
			ImGui::Spacing();
			ImGui::SetNextItemWidth(100);
			ImGui::CustomSliderInt("Custom Slider", &g_Engine->TestInt, 1, 10, 0, NULL);
			ImGui::End();
		}
	}

	void Menu::Loops()
	{

	}
}