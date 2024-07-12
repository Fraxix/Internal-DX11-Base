// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

// STANDARD LIBRARIES
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <iostream>
#include <conio.h>
#include <string>
#include <thread>
#include <vector>
#include <chrono>

// DIRECTX
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

//	GAMEPAD
#include <XInput.h>
#pragma comment(lib, "XInput.lib")

//	INTERNET
#include <Wininet.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

// MINHOOK
#include "Hooking/MinHook.h"

// DEAR IMGUI
#include "Framework/imgui.h"
#include "Framework/imgui_internal.h"
#include "Framework/imgui_Impl_dx11.h"
#include "Framework/imgui_Impl_Win32.h"
#endif //PCH_H
