#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include "pch.h"
#include "KeyboardHandler.h"
#include "MouseHandler.h"

class InputHandler
{
private:
	InputHandler() {}

	KeyboardHandler m_keyboard;
	MouseHandler m_mouse;

public:
	static InputHandler& getInstance()
	{
		static InputHandler instance;
		return instance;
	}

	InputHandler(InputHandler const&) = delete;
	void operator=(InputHandler const&) = delete;

	// Keyboard Pass Thorugh
	bool keyBufferIsEmpty() { return m_keyboard.keyBufferIsEmpty(); }
	bool charBufferIsEmpty() { return m_keyboard.charBufferIsEmpty(); }

	bool keyIsPressed(const unsigned char key) { return m_keyboard.keyIsPressed(key); }
	KeyboardEvent readKey() { return m_keyboard.readKey(); }
	unsigned char readChar() { return m_keyboard.readChar(); }

	void onKeyPressed(const unsigned char key) { m_keyboard.onKeyPressed(key); }
	void onKeyReleased(const unsigned char key) { m_keyboard.onKeyReleased(key); }
	void onChar(const unsigned char key) { m_keyboard.onChar(key); }

	void enableAutoRepeatKeys() { m_keyboard.enableAutoRepeatKeys(); }
	void disableAutoRepeatKeys() { m_keyboard.disableAutoRepeatKeys(); }
	bool isAutoRepeatingKeys() { return m_keyboard.isAutoRepeatingKeys(); }

	void enableAutoRepeatChars() { m_keyboard.enableAutoRepeatChars(); }
	void disableAutoRepeatChars() { m_keyboard.disableAutoRepeatChars(); }
	bool isAutoRepeatingChars() { return m_keyboard.isAutoRepeatingChars(); }
	
	// Mouse Pass Through
	bool mouseBufferIsEmpty() { return m_mouse.eventBufferIsEmpty(); }
	void onMouseMove(int x, int y) { m_mouse.onMouseMove(x, y); }
	void onMouseRawMove(int x, int y) { m_mouse.onMouseRawMove(x, y); }
	
	MouseEvent readMouseEvent() { return m_mouse.readEvent(); }
	bool isMouseLeftDown() {	return m_mouse.isLeftDown(); }
	bool isMouseRightDown() { return m_mouse.isRightDown(); }
	bool isMouseMiddleDown() { return m_mouse.isMiddleDown(); }

	void onLeftPressed(int x, int y) { m_mouse.onLeftPressed(x, y); }
	void onRightPressed(int x, int y) { m_mouse.onRightPressed(x, y); }
	void onMiddlePressed(int x, int y) { m_mouse.onMiddlePressed(x, y); }
	
	void onLeftReleased(int x, int y) { m_mouse.onLeftReleased(x, y); }
	void onRightReleased(int x, int y) { m_mouse.onRightReleased(x, y); }
	void onMiddleReleased(int x, int y) { m_mouse.onMiddleReleased(x, y); }
	
	void onWheelUp(int x, int y) { m_mouse.onWheelUp(x, y); }
	void onWheelDown(int x, int y) { m_mouse.onWheelDown(x, y); }
};

#endif //!INPUTHANDLER_H