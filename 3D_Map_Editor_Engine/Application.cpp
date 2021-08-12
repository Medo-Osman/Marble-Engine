#include "pch.h"
#include "Application.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

extern Application* app;

Application::Application()
{
	m_name = L"3D Map Editor Engine";
	m_settings.width = 1280;
	m_settings.height = 720;
	m_settings.fov = 70;
	m_settings.mouseSensitivity = 0.005f;
	m_deltaTime = 0.f;
	m_shouldQuit = false;
}

bool Application::initRawMouseDevice()
{
	RAWINPUTDEVICE rawIDevice;
	rawIDevice.usUsagePage = 0x01;
	rawIDevice.usUsage = 0x02;
	rawIDevice.dwFlags = 0;
	rawIDevice.hwndTarget = NULL;
	if (RegisterRawInputDevices(&rawIDevice, 1, sizeof(rawIDevice)) == FALSE)
		return false;

	return true;
}

Application::~Application() {}

bool Application::initialize(HINSTANCE hInstance, LPWSTR lpCmdLine, HWND hwnd, int showCmd)
{
	// Window
	createWin32Window(hInstance, m_name.c_str(), hwnd);
	OutputDebugStringA("Window Created!\n");

	// ImGUI
	m_window = hwnd;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(m_window);
	ImGui::StyleColorsDark();
	
	// Renderer
	m_renderer = RenderHandler::getInstance();
	m_renderer->initialize(&m_window, &m_settings);

	// Utilites
	m_timer.start();

	// Game
	m_game.initialize(m_settings);
	
	// Initialize Raw Mouse Input
	if (!initRawMouseDevice())
		return false;

	// Show Window
	ShowWindow(m_window, showCmd);

	return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg)
	{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

			EndPaint(hwnd, &ps);
			break;
		}
		case WM_ACTIVATE:
		{
			if (wParam & WA_ACTIVE || wParam & WA_CLICKACTIVE)
			{
				RECT winRect;
				GetClientRect(hwnd, &winRect);
				MapWindowPoints(hwnd, nullptr, reinterpret_cast<POINT*>(&winRect), 2);
				ClipCursor(&winRect);
			}
			else
				ClipCursor(nullptr);

			break;
		}
		/*case WM_SIZING:
		{
			RECT winRect;
			GetWindowRect(hwnd, &winRect);
			ImGui::GetIO().DisplaySize.x = winRect.right;
			ImGui::GetIO().DisplaySize.y = winRect.bottom;

			break;
		}*/
		case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			InputHandler::getInstance().onLeftPressed(x, y);

			return 0;
		}
		case WM_RBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			InputHandler::getInstance().onRightPressed(x, y);

			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			InputHandler::getInstance().onMiddlePressed(x, y);

			return 0;
		}
		case WM_LBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			InputHandler::getInstance().onLeftReleased(x, y);

			return 0;
		}
		case WM_RBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			InputHandler::getInstance().onRightReleased(x, y);

			return 0;
		}
		case WM_MBUTTONUP:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			InputHandler::getInstance().onMiddleReleased(x, y);

			return 0;
		}
		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			/*std::string mousePos = std::to_string(x) + ", " + std::to_string(y) + "\n";
			OutputDebugStringA(mousePos.c_str());*/
			InputHandler::getInstance().onMouseMove(x, y);

			return 0;
		}
		case WM_INPUT:
		{
			UINT dataSize;
			GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
			if (dataSize > 0)
			{
				std::unique_ptr<BYTE[]> rawByteData = std::make_unique<BYTE[]>(dataSize);
				if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawByteData.get(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize)
				{
					RAWINPUT* rawInput = reinterpret_cast<RAWINPUT*>(rawByteData.get());
					if (rawInput->header.dwType == RIM_TYPEMOUSE)
						InputHandler::getInstance().onMouseRawMove(rawInput->data.mouse.lLastX, rawInput->data.mouse.lLastY);
				}
			}
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
		case WM_MOUSEWHEEL:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				InputHandler::getInstance().onWheelUp(x, y);
			else if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
				InputHandler::getInstance().onWheelDown(x, y);

			return 0;
		}
		case WM_KEYDOWN:
		{
			unsigned char keyCode = static_cast<unsigned char>(wParam);
			if (InputHandler::getInstance().isAutoRepeatingChars())
				InputHandler::getInstance().onKeyPressed(keyCode);
			else
			{
				const bool wasPressed = lParam & 0x40000000; // Checks if previous key is the same as current key
				if (!wasPressed)
					InputHandler::getInstance().onKeyPressed(keyCode);
			}
			return 0;
		}
		case WM_KEYUP:
		{
			unsigned char keyCode = static_cast<unsigned char>(wParam);
			InputHandler::getInstance().onKeyReleased(keyCode);
			return 0;
		}
		case WM_CHAR:
		{
			unsigned char keyChar = static_cast<unsigned char>(wParam);
			if (InputHandler::getInstance().isAutoRepeatingChars())
				InputHandler::getInstance().onChar(keyChar);
			else
			{
				const bool wasPressed = lParam & 0x40000000; // Checks if previous key is the same as current key
				if (!wasPressed)
					InputHandler::getInstance().onChar(keyChar);
			}
			break;
		}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Application::createWin32Window(HINSTANCE hInstance, const wchar_t* windowTitle, HWND& window)
{
	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = m_name.c_str();
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));

	RegisterClass(&wc);

	// Create the window.
	window = CreateWindowEx(
		0,                          // Optional window styles.
		m_name.c_str(),             // Window class
		windowTitle,				// Window text
		WS_OVERLAPPEDWINDOW,        // Window style
		CW_USEDEFAULT,				// Position, X
		CW_USEDEFAULT,				// Position, Y
		m_settings.width,			// Width
		m_settings.height,			// Height
		NULL,						// Parent window    
		NULL,						// Menu
		hInstance,					// Instance handle
		NULL						// Additional application data
	);
	assert(window);
}

HWND Application::getWindow() const
{
	return m_window;
}

void Application::applicationLoop()
{
	MSG msg = { };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) // Message loop
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else // Render/Logic Loop
		{
			// Delta Time
			m_deltaTime = (float)m_timer.timeElapsed();
			m_timer.restart();

			// Logic
			m_game.update(m_deltaTime);
			RenderHandler::getInstance()->update(m_deltaTime);

			if (!InputHandler::getInstance().keyBufferIsEmpty())
			{
				if (InputHandler::getInstance().keyIsPressed(KeyCodes::Esc)) // Escape
					PostQuitMessage(-1);
			}

			// Render
			RenderHandler::getInstance()->render();
		}
	}
}
