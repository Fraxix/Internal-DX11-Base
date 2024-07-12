#pragma once
#include "helper.h"

namespace DX11Base 
{
	class Menu
	{
	public:
		static void Render();
		static void Loops();
		static void DrawMenu();

		Menu()  noexcept = default;
		~Menu() noexcept = default;
	};
}