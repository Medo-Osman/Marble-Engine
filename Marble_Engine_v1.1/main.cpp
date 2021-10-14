#include "pch.h"
#include "Application.h"

Application* app;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	HRESULT hr = CoInitialize(NULL);

	bool initOK = false;
	app = &Application::getInstance();
	initOK = app->initialize(hInstance, lpCmdLine, app->getWindow(), nShowCmd);

	if (initOK)
	{
		OutputDebugStringA("Window Created!\n");
		app->applicationLoop();
	}

	return 0;
};