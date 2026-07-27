#pragma once

#include "UPlayer.h"

class UViewport : public UPlayer
{
public:
	using UPlayer::UPlayer;

	bool IsRealtime() const { return m_IsRealTime; }
	void SetRealtime(const bool value) { m_IsRealTime = value; }

	int ViewportX() const { return m_ViewportX; }
	int ViewportY() const { return m_ViewportY; }
	int ViewportWidth() const { return m_ViewportWidth; }
	int ViewportHeight() const { return m_ViewportHeight; }

	void SetViewportRect(int xPos, int yPos, int width, int height)
	{
		m_ViewportX = xPos;
		m_ViewportY = yPos;
		m_ViewportWidth = width;
		m_ViewportHeight = height;
	}

private:
	int m_ViewportX = 0;
	int m_ViewportY = 0;
	int m_ViewportWidth = 0;
	int m_ViewportHeight = 0;

	bool m_IsRealTime = true;
};
