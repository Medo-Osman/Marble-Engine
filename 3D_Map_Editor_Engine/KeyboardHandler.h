#ifndef KEYBOARDHANDLER_H
#define KEYBOARDHANDLER_H

#include "pch.h"

enum class KeyboardEventType { Press, Release, Invalid };

struct KeyboardEvent
{
	KeyboardEventType type;
	unsigned char key;
	KeyboardEvent()
	{
		type = KeyboardEventType::Invalid;
		key = 0u;
	}
	KeyboardEvent(KeyboardEventType type, unsigned char key)
	{
		this->type = type;
		this->key = key;
	}
};

class KeyboardHandler
{
private:

	bool m_autoRepeatKeys;
	bool m_autoRepeatChars;
	bool m_keyStates[256];
	std::queue<KeyboardEvent> m_keyBuffer;
	std::queue<unsigned char> m_charBuffer;

public:
	KeyboardHandler()
	{
		m_autoRepeatKeys = false;
		m_autoRepeatChars = false;
		for (int i = 0; i < 256; i++)
			m_keyStates[i] = false;
	}

	// Is Buffers Empty
	bool keyBufferIsEmpty() { return m_keyBuffer.empty(); }
	bool charBufferIsEmpty() { return m_charBuffer.empty(); }
	
	// Read
	bool keyIsPressed(const unsigned char key) { return m_keyStates[key]; }
	KeyboardEvent readKey() 
	{
		if (m_keyBuffer.empty())
			return KeyboardEvent();
		else
		{
			KeyboardEvent event = m_keyBuffer.front();
			m_keyBuffer.pop();
			return event;
		}
	}
	unsigned char readChar()
	{
		if (m_charBuffer.empty())
			return 0u;
		else
		{
			unsigned char eventChar = m_charBuffer.front();
			m_charBuffer.pop();
			return eventChar;
		}
	}

	// On Events
	void onKeyPressed(const unsigned char key)
	{
		m_keyStates[key] = true;
		m_keyBuffer.push(KeyboardEvent(KeyboardEventType::Press, key));
	}
	void onKeyReleased(const unsigned char key)
	{
		m_keyStates[key] = false;
		m_keyBuffer.push(KeyboardEvent(KeyboardEventType::Release, key));
	}
	void onChar(const unsigned char key)
	{
		m_charBuffer.push(key);
	}

	// Auto Repeat Keys
	void enableAutoRepeatKeys() { m_autoRepeatKeys = true; }
	void disableAutoRepeatKeys() { m_autoRepeatKeys = false; }
	bool isAutoRepeatingKeys() { return m_autoRepeatKeys; }

	// Auto Repeat Chars
	void enableAutoRepeatChars() { m_autoRepeatChars = true; }
	void disableAutoRepeatChars() { m_autoRepeatChars = false; }
	bool isAutoRepeatingChars() { return m_autoRepeatChars; }
};

#endif // !KEYBOARDHANDLER_H