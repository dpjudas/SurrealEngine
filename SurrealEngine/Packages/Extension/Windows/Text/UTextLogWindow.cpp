
#include "Precomp.h"
#include "UTextLogWindow.h"

void UTextLogWindow::AddLog(const std::string& NewText, const Color& linecol)
{
	if (!bPaused())
	{
		Text() += NewText;
		TextColor() = linecol; // To do: we need more complex colored text drawing it seems.
	}
}

void UTextLogWindow::ClearLog()
{
	Text().clear();
}

void UTextLogWindow::PauseLog(bool bNewPauseState)
{
	bPaused() = bNewPauseState;
}

void UTextLogWindow::SetTextTimeout(float newTimeout)
{
	textTimeout() = newTimeout;
}
