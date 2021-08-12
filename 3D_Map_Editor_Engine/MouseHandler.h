#ifndef MOUSEHANDLER_H
#define MOUSEHANDLER_H

#include "pch.h"

enum class MouseEventType {LPress, LRelease, RPress, RRelease, MPress, MRelease, WheelUp, WheelDown, Move, RawMove, Invalid };

struct MousePoint
{
	int x;
	int y;
};

struct MouseEvent
{
	MouseEventType type;
	MousePoint point;
	MouseEvent()
	{
		type = MouseEventType::Invalid;
		point.x = 0;
		point.y = 0;
	}
	MouseEvent(MouseEventType type, int x, int y)
	{
		this->type = type;
		this->point.x = x;
		this->point.y = y;
	}
};

class MouseHandler
{
private:
	std::queue<MouseEvent> m_eventBuffer;
	bool m_leftIsDown;
	bool m_rightIsDown;
	bool m_middleIsDown;
	MousePoint m_position;

public:
	MouseHandler()
	{
		m_leftIsDown = false;
		m_rightIsDown = false;
		m_middleIsDown = false;
		m_position.x = false;
		m_position.y = false;
	}

	// Mouse Click
	void onLeftPressed(int x, int y)
	{
		m_leftIsDown = true;
		m_eventBuffer.push(MouseEvent(MouseEventType::LPress, x, y));
	}
	void onLeftReleased(int x, int y) 
	{
		m_leftIsDown = false;
		m_eventBuffer.push(MouseEvent(MouseEventType::LRelease, x, y));
	}
	void onRightPressed(int x, int y)
	{
		m_rightIsDown = true;
		m_eventBuffer.push(MouseEvent(MouseEventType::RPress, x, y));
	}
	void onRightReleased(int x, int y)
	{
		m_rightIsDown = false;
		m_eventBuffer.push(MouseEvent(MouseEventType::RRelease, x, y));
	}
	void onMiddlePressed(int x, int y)
	{
		m_middleIsDown = true;
		m_eventBuffer.push(MouseEvent(MouseEventType::MPress, x, y));
	}
	void onMiddleReleased(int x, int y)
	{
		m_middleIsDown = false;
		m_eventBuffer.push(MouseEvent(MouseEventType::MRelease, x, y));
	}

	// Wheel Scrolling
	void onWheelUp(int x, int y)
	{
		m_eventBuffer.push(MouseEvent(MouseEventType::WheelUp, x, y));
	}
	void onWheelDown(int x, int y)
	{
		m_eventBuffer.push(MouseEvent(MouseEventType::WheelDown, x, y));
	}
	void onMouseMove(int x, int y)
	{
		m_position.x = x;
		m_position.y = y;
		m_eventBuffer.push(MouseEvent(MouseEventType::Move, x, y));
	}
	void onMouseRawMove(int x, int y)
	{
		m_eventBuffer.push(MouseEvent(MouseEventType::RawMove, x, y));
	}

	// Is Down
	bool isLeftDown() { return m_leftIsDown; }
	bool isRightDown() { return m_rightIsDown; }
	bool isMiddleDown() { return m_middleIsDown; }

	// Get Mouse Position
	int getPosX() { return m_position.x; }
	int getPosY() { return m_position.y; }
	MousePoint getPos() { return m_position; }

	bool eventBufferIsEmpty() { return m_eventBuffer.empty(); }
	MouseEvent readEvent()
	{
		if (m_eventBuffer.empty())
			return MouseEvent();
		else
		{
			MouseEvent mouseEvent = m_eventBuffer.front();
			m_eventBuffer.pop();
			return mouseEvent;
		}
	}
};

#endif // !MOUSEHANDLER_H