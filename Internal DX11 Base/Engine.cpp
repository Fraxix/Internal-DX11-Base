#include "pch.h"
#include "Engine.h"
#include "Menu.h"
#include "Fonts.h"
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static uint64_t* MethodsTable{ nullptr };

namespace DX11Base 
{

	Engine::Engine()
	{

		g_D3D11Window = std::make_unique<D3D11Window>();
		g_Hooking = std::make_unique<Hooking>();
	}

	Engine::~Engine()
	{
		g_Hooking.release();
		g_D3D11Window.release();
	}


	D3D11Window::D3D11Window() {}

	D3D11Window::~D3D11Window() { bInit = false; }

	LRESULT D3D11Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
	{
		ImGuiIO& io = ImGui::GetIO();
		POINT mPos;
		GetCursorPos(&mPos);
		ScreenToClient(g_Engine->pGameWindow, &mPos);
		ImGui::GetIO().MousePos.x = mPos.x;
		ImGui::GetIO().MousePos.y = mPos.y;

		if (g_Engine->bShowMenu) 
		{
			ImGui_ImplWin32_WndProcHandler((HWND)g_D3D11Window->m_OldWndProc, msg, wParam, lParam);
			return true;
		}
		return CallWindowProc((WNDPROC)g_D3D11Window->m_OldWndProc, hWnd, msg, wParam, lParam);
	}

	HRESULT APIENTRY D3D11Window::SwapChain_Present_hook(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
	{
		g_D3D11Window->Overlay(pSwapChain);

		return g_D3D11Window->IDXGISwapChain_Present_stub(pSwapChain, SyncInterval, Flags);
	}

	HRESULT APIENTRY D3D11Window::SwapChain_ResizeBuffers_hook(IDXGISwapChain* p, UINT bufferCount, UINT Width, UINT Height, DXGI_FORMAT fmt, UINT scFlags)
	{
		//  Get new data & release render target
		g_D3D11Window->m_pSwapChain = p;
		g_D3D11Window->m_RenderTargetView->Release();
		g_D3D11Window->m_RenderTargetView = nullptr;

		//  get fn result
		HRESULT result = g_D3D11Window->IDXGISwapChain_ResizeBuffers_stub(p, bufferCount, Width, Height, fmt, scFlags);

		// Get new render target
		ID3D11Texture2D* backBuffer;
		p->GetBuffer(0, __uuidof(ID3D11Texture2D*), (LPVOID*)&backBuffer);
		if (backBuffer)
		{
			g_D3D11Window->m_Device->CreateRenderTargetView(backBuffer, 0, &g_D3D11Window->m_RenderTargetView);
			backBuffer->Release();
		}

		//  Reset ImGui 
		if (g_D3D11Window->bInitImGui)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(static_cast<float>(Width), static_cast<float>(Height));
		}

		return result;
	}

	bool D3D11Window::HookD3D()
	{
		if (GetD3DContext())
		{
			Hooking::CreateHook((void*)MethodsTable[IDXGI_PRESENT], &SwapChain_Present_hook, (void**)&IDXGISwapChain_Present_stub);
			Hooking::CreateHook((void*)MethodsTable[IDXGI_RESIZE_BUFFERS], &SwapChain_ResizeBuffers_hook, (void**)&IDXGISwapChain_ResizeBuffers_stub);
			bInit = true;
			return true;
		}
		return false;
	}

	void D3D11Window::UnhookD3D()
	{
		SetWindowLongPtr(g_Engine->pGameWindow, GWLP_WNDPROC, (LONG_PTR)m_OldWndProc);
		Hooking::DisableHook((void*)MethodsTable[IDXGI_PRESENT]);
		Hooking::DisableHook((void*)MethodsTable[IDXGI_RESIZE_BUFFERS]);
		MH_DisableHook(MH_ALL_HOOKS);
		MH_Uninitialize();

		free(MethodsTable);
	}

	bool D3D11Window::GetD3DContext()
	{
		if (!InitWindow())
			return false;

		HMODULE D3D11Module = GetModuleHandleA("d3d11.dll");

		D3D_FEATURE_LEVEL FeatureLevel;
		const D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_11_0 };

		DXGI_RATIONAL RefreshRate;
		RefreshRate.Numerator = 60;
		RefreshRate.Denominator = 1;

		DXGI_MODE_DESC BufferDesc;
		BufferDesc.Width = 100;
		BufferDesc.Height = 100;
		BufferDesc.RefreshRate = RefreshRate;
		BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		DXGI_SAMPLE_DESC SampleDesc;
		SampleDesc.Count = 1;
		SampleDesc.Quality = 0;

		DXGI_SWAP_CHAIN_DESC SwapChainDesc;
		SwapChainDesc.BufferDesc = BufferDesc;
		SwapChainDesc.SampleDesc = SampleDesc;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.BufferCount = 1;
		SwapChainDesc.OutputWindow = WindowHwnd;
		SwapChainDesc.Windowed = 1;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		IDXGISwapChain* SwapChain;
		ID3D11Device* Device;
		ID3D11DeviceContext* Context;
		if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, FeatureLevels, 1, D3D11_SDK_VERSION, &SwapChainDesc, &SwapChain, &Device, &FeatureLevel, &Context) < 0)
		{
			DeleteWindow();
			return false;
		}

		MethodsTable = (uint64_t*)::calloc(205, sizeof(uint64_t));
		memcpy(MethodsTable, *(uint64_t**)SwapChain, 18 * sizeof(uint64_t));
		memcpy(MethodsTable + 18, *(uint64_t**)Device, 43 * sizeof(uint64_t));
		memcpy(MethodsTable + 18 + 43, *(uint64_t**)Context, 144 * sizeof(uint64_t));
		Sleep(1000);

		SwapChain->Release();
		SwapChain = 0;
		Device->Release();
		Device = 0;
		Context->Release();
		Context = 0;
		DeleteWindow();
		return true;
	}

	bool D3D11Window::InitWindow()
	{
		WindowClass.cbSize = sizeof(WNDCLASSEX);
		WindowClass.style = CS_HREDRAW | CS_VREDRAW;
		WindowClass.lpfnWndProc = DefWindowProc;
		WindowClass.cbClsExtra = 0;
		WindowClass.cbWndExtra = 0;
		WindowClass.hInstance = GetModuleHandle(NULL);
		WindowClass.hIcon = NULL;
		WindowClass.hCursor = NULL;
		WindowClass.hbrBackground = NULL;
		WindowClass.lpszMenuName = NULL;
		WindowClass.lpszClassName = L"MJ";
		WindowClass.hIconSm = NULL;
		RegisterClassEx(&WindowClass);
		WindowHwnd = CreateWindow(WindowClass.lpszClassName, L"DX11 Window", WS_OVERLAPPEDWINDOW, 0, 0, 100, 100, NULL, NULL, WindowClass.hInstance, NULL);
		if (WindowHwnd == NULL) {
			return FALSE;
		}
		return TRUE;
	}

	bool D3D11Window::DeleteWindow()
	{
		DestroyWindow(WindowHwnd);
		UnregisterClass(WindowClass.lpszClassName, WindowClass.hInstance);
		if (WindowHwnd != 0) {
			return FALSE;
		}
		return TRUE;
	}

	bool D3D11Window::InitImGui(IDXGISwapChain* swapChain)
	{
		if (SUCCEEDED(swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&m_Device))) {
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			io.IniFilename = 0;
			ImGui::StyleColorsDark();
			m_Device->GetImmediateContext(&m_DeviceContext);

			ImFontConfig font;
			font.FontDataOwnedByAtlas = false;
			io.Fonts->AddFontFromMemoryTTF((void*)RudaBold, sizeof(RudaBold), 16.5f, &font);

			DXGI_SWAP_CHAIN_DESC Desc;
			swapChain->GetDesc(&Desc);
			g_Engine->pGameWindow = Desc.OutputWindow;

			ID3D11Texture2D* BackBuffer;
			swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
			m_Device->CreateRenderTargetView(BackBuffer, NULL, &m_RenderTargetView);
			BackBuffer->Release();

			ImGui_ImplWin32_Init(g_Engine->pGameWindow);
			ImGui_ImplDX11_Init(m_Device, m_DeviceContext);
			ImGui_ImplDX11_CreateDeviceObjects();
			ImGui::GetIO().ImeWindowHandle = g_Engine->pGameWindow;
			m_OldWndProc = (WNDPROC)SetWindowLongPtr(g_Engine->pGameWindow, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
			bInitImGui = true;
			m_pSwapChain = swapChain;
			pImGui = GImGui;
			pViewport = pImGui->Viewports[0];
			return true;
		}
		bInitImGui = false;
		return false;
	}
	void D3D11Window::Overlay(IDXGISwapChain* pSwapChain)
	{
		if (!bInitImGui)
			InitImGui(pSwapChain);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::GetIO().MouseDrawCursor = g_Engine->bShowMenu;

		//	Render Menu Loop
		Menu::Render();

		ImGui::EndFrame();
		ImGui::Render();
		m_DeviceContext->OMSetRenderTargets(1, &m_RenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}

	Hooking::Hooking()
	{
		MH_Initialize();
	}

	Hooking::~Hooking()
	{
		DisableAllHooks();
		RemoveAllHooks();
		MH_Uninitialize();
	}

	void Hooking::Initialize()
	{
		EnableAllHooks();
	}

	void Hooking::Shutdown()
	{
		RemoveAllHooks();

	}

	bool Hooking::CreateHook(LPVOID lpTarget, LPVOID pDetour, LPVOID* pOrig)
	{
		if (MH_CreateHook(lpTarget, pDetour, pOrig) != MH_OK || MH_EnableHook(lpTarget) != MH_OK)
			return false;
		return true;
	}

	void Hooking::EnableHook(LPVOID lpTarget) { MH_EnableHook(lpTarget); }

	void Hooking::DisableHook(LPVOID lpTarget) { MH_DisableHook(lpTarget); }

	void Hooking::RemoveHook(LPVOID lpTarget) { MH_RemoveHook(lpTarget); }

	void Hooking::EnableAllHooks() { MH_EnableHook(MH_ALL_HOOKS); }

	void Hooking::DisableAllHooks() { MH_DisableHook(MH_ALL_HOOKS); }

	void Hooking::RemoveAllHooks() { MH_RemoveHook(MH_ALL_HOOKS); }
}