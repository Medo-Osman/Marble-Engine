#ifndef APPLICATION_H
#define APPLICATION_H

#include "InputHandler.h"
#include "resource.h"
#include "RenderHandler.h"
#include "GameState.h"

class Application
{
private:
	Application();

	// Window
	HWND m_window;
	std::wstring m_name;
	bool m_shouldQuit;

	// Settings
	Settings m_settings;

	// Timer
	Timer m_timer;
	float m_deltaTime;

	// Game
	GameState m_game;

	// Renderer
	RenderHandler* m_renderer;

	// Functions
	bool initRawMouseDevice();

public:
	static Application& getInstance()
	{
		static Application instance;
		return instance;
	}
	~Application();

	Application(Application const&) = delete;
	void operator=(Application const&) = delete;

	// Initialize
	bool initialize(const HINSTANCE hInstance, const LPWSTR lpCmdLine, const HWND hwnd, const int showCmd);
	void createWin32Window(const HINSTANCE hInstance, const wchar_t* windowTitle, HWND& _d3d11Window);

	// Getters
	HWND getWindow() const;

	// Applicaiton Loop
	void applicationLoop();
};

#endif // !APPLICATION_H