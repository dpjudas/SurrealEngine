#pragma once

#include "UObject.h"

class UFont;
class UViewport;
class UTexture;
class UPlayerReplicationInfo;
class UPlayerPawn;

class UClient : public UObject { using UObject::UObject; };

class UCanvas : public UObject
{
public:
	using UObject::UObject;

	UFont*& BigFont() { return Value<UFont*>(PropOffsets_Canvas.BigFont); }
	float& ClipX() { return Value<float>(PropOffsets_Canvas.ClipX); }
	float& ClipY() { return Value<float>(PropOffsets_Canvas.ClipY); }
	float& CurX() { return Value<float>(PropOffsets_Canvas.CurX); }
	float& CurY() { return Value<float>(PropOffsets_Canvas.CurY); }
	float& CurYL() { return Value<float>(PropOffsets_Canvas.CurYL); }
	Color& DrawColor() { return Value<Color>(PropOffsets_Canvas.DrawColor); }
	UFont*& Font() { return Value<UFont*>(PropOffsets_Canvas.Font); }
	int& FramePtr() { return Value<int>(PropOffsets_Canvas.FramePtr); }
	UFont*& LargeFont() { return Value<UFont*>(PropOffsets_Canvas.LargeFont); }
	UFont*& MedFont() { return Value<UFont*>(PropOffsets_Canvas.MedFont); }
	float& OrgX() { return Value<float>(PropOffsets_Canvas.OrgX); }
	float& OrgY() { return Value<float>(PropOffsets_Canvas.OrgY); }
	int& RenderPtr() { return Value<int>(PropOffsets_Canvas.RenderPtr); }
	int& SizeX() { return Value<int>(PropOffsets_Canvas.SizeX); }
	int& SizeY() { return Value<int>(PropOffsets_Canvas.SizeY); }
	UFont*& SmallFont() { return Value<UFont*>(PropOffsets_Canvas.SmallFont); }
	float& SpaceX() { return Value<float>(PropOffsets_Canvas.SpaceX); }
	float& SpaceY() { return Value<float>(PropOffsets_Canvas.SpaceY); }
	uint8_t& Style() { return Value<uint8_t>(PropOffsets_Canvas.Style); }
	UViewport*& Viewport() { return Value<UViewport*>(PropOffsets_Canvas.Viewport); }
	float& Z() { return Value<float>(PropOffsets_Canvas.Z); }
	bool& bCenter() { return Value<bool>(PropOffsets_Canvas.bCenter); }
	bool& bNoSmooth() { return Value<bool>(PropOffsets_Canvas.bNoSmooth); }
};

class UConsole : public UObject
{
public:
	using UObject::UObject;

	std::string& AvgText() { return Value<std::string>(PropOffsets_Console.AvgText); }
	UTexture*& Border() { return Value<UTexture*>(PropOffsets_Console.Border); }
	int& BorderLines() { return Value<int>(PropOffsets_Console.BorderLines); }
	int& BorderPixels() { return Value<int>(PropOffsets_Console.BorderPixels); }
	int& BorderSize() { return Value<int>(PropOffsets_Console.BorderSize); }
	UTexture*& ConBackground() { return Value<UTexture*>(PropOffsets_Console.ConBackground); }
	std::string& ConnectingMessage() { return Value<std::string>(PropOffsets_Console.ConnectingMessage); }
	float& ConsoleDest() { return Value<float>(PropOffsets_Console.ConsoleDest); }
	int& ConsoleLines() { return Value<int>(PropOffsets_Console.ConsoleLines); }
	float& ConsolePos() { return Value<float>(PropOffsets_Console.ConsolePos); }
	float& ExtraTime() { return Value<float>(PropOffsets_Console.ExtraTime); }
	int& FrameCount() { return Value<int>(PropOffsets_Console.FrameCount); }
	std::string& FrameRateText() { return Value<std::string>(PropOffsets_Console.FrameRateText); }
	float& FrameX() { return Value<float>(PropOffsets_Console.FrameX); }
	float& FrameY() { return Value<float>(PropOffsets_Console.FrameY); }
	std::string& FramesText() { return Value<std::string>(PropOffsets_Console.FramesText); }
	std::string& History() { return Value<std::string>(PropOffsets_Console.History); }
	int& HistoryBot() { return Value<int>(PropOffsets_Console.HistoryBot); }
	int& HistoryCur() { return Value<int>(PropOffsets_Console.HistoryCur); }
	int& HistoryTop() { return Value<int>(PropOffsets_Console.HistoryTop); }
	float& LastFrameTime() { return Value<float>(PropOffsets_Console.LastFrameTime); }
	float& LastSecFPS() { return Value<float>(PropOffsets_Console.LastSecFPS); }
	std::string& LastSecText() { return Value<std::string>(PropOffsets_Console.LastSecText); }
	int& LastSecondFrameCount() { return Value<int>(PropOffsets_Console.LastSecondFrameCount); }
	float& LastSecondStartTime() { return Value<float>(PropOffsets_Console.LastSecondStartTime); }
	std::string& LoadingMessage() { return Value<std::string>(PropOffsets_Console.LoadingMessage); }
	float& MaxFPS() { return Value<float>(PropOffsets_Console.MaxFPS); }
	std::string& MaxText() { return Value<std::string>(PropOffsets_Console.MaxText); }
	float& MinFPS() { return Value<float>(PropOffsets_Console.MinFPS); }
	std::string& MinText() { return Value<std::string>(PropOffsets_Console.MinText); }
	UPlayerReplicationInfo*& MsgPlayer() { return Value<UPlayerReplicationInfo*>(PropOffsets_Console.MsgPlayer); }
	std::string& MsgText() { return Value<std::string>(PropOffsets_Console.MsgText); }
	float& MsgTick() { return Value<float>(PropOffsets_Console.MsgTick); }
	float& MsgTickTime() { return Value<float>(PropOffsets_Console.MsgTickTime); }
	float& MsgTime() { return Value<float>(PropOffsets_Console.MsgTime); }
	NameString& MsgType() { return Value<NameString>(PropOffsets_Console.MsgType); }
	std::string& PausedMessage() { return Value<std::string>(PropOffsets_Console.PausedMessage); }
	std::string& PrecachingMessage() { return Value<std::string>(PropOffsets_Console.PrecachingMessage); }
	std::string& SavingMessage() { return Value<std::string>(PropOffsets_Console.SavingMessage); }
	int& Scrollback() { return Value<int>(PropOffsets_Console.Scrollback); }
	std::string& SecondsText() { return Value<std::string>(PropOffsets_Console.SecondsText); }
	float& StartTime() { return Value<float>(PropOffsets_Console.StartTime); }
	int& TextLines() { return Value<int>(PropOffsets_Console.TextLines); }
	UFont*& TimeDemoFont() { return Value<UFont*>(PropOffsets_Console.TimeDemoFont); }
	int& TopLine() { return Value<int>(PropOffsets_Console.TopLine); }
	std::string& TypedStr() { return Value<std::string>(PropOffsets_Console.TypedStr); }
	UViewport*& Viewport() { return Value<UViewport*>(PropOffsets_Console.Viewport); }
	bool& bNoDrawWorld() { return Value<bool>(PropOffsets_Console.bNoDrawWorld); }
	bool& bNoStuff() { return Value<bool>(PropOffsets_Console.bNoStuff); }
	bool& bRestartTimeDemo() { return Value<bool>(PropOffsets_Console.bRestartTimeDemo); }
	bool& bSaveTimeDemoToFile() { return Value<bool>(PropOffsets_Console.bSaveTimeDemoToFile); }
	bool& bStartTimeDemo() { return Value<bool>(PropOffsets_Console.bStartTimeDemo); }
	bool& bTimeDemo() { return Value<bool>(PropOffsets_Console.bTimeDemo); }
	bool& bTyping() { return Value<bool>(PropOffsets_Console.bTyping); }
	std::string& fpsText() { return Value<std::string>(PropOffsets_Console.fpsText); }
	int& numLines() { return Value<int>(PropOffsets_Console.numLines); }
	int& vtblOut() { return Value<int>(PropOffsets_Console.vtblOut); }
};

class UPlayer : public UObject
{
public:
	using UObject::UObject;

	UPlayerPawn*& Actor() { return Value<UPlayerPawn*>(PropOffsets_Player.Actor); }
	int& ConfiguredInternetSpeed() { return Value<int>(PropOffsets_Player.ConfiguredInternetSpeed); }
	int& ConfiguredLanSpeed() { return Value<int>(PropOffsets_Player.ConfiguredLanSpeed); }
	UConsole*& Console() { return Value<UConsole*>(PropOffsets_Player.Console); }
	int& CurrentNetSpeed() { return Value<int>(PropOffsets_Player.CurrentNetSpeed); }
	uint8_t& SelectedCursor() { return Value<uint8_t>(PropOffsets_Player.SelectedCursor); }
	float& WindowsMouseX() { return Value<float>(PropOffsets_Player.WindowsMouseX); }
	float& WindowsMouseY() { return Value<float>(PropOffsets_Player.WindowsMouseY); }
	bool& bShowWindowsMouse() { return Value<bool>(PropOffsets_Player.bShowWindowsMouse); }
	bool& bSuspendPrecaching() { return Value<bool>(PropOffsets_Player.bSuspendPrecaching); }
	bool& bWindowsMouseAvailable() { return Value<bool>(PropOffsets_Player.bWindowsMouseAvailable); }
	int& vfExec() { return Value<int>(PropOffsets_Player.vfExec); } // native
	int& vfOut() { return Value<int>(PropOffsets_Player.vfOut); } // native
};

class UViewport : public UPlayer
{
public:
	using UPlayer::UPlayer;

	bool IsRealtime() { return true; } // To do: when is this false?
};

class UNetConnection : public UPlayer
{
public:
	using UPlayer::UPlayer;
};

class UDemoRecConnection : public UNetConnection
{
public:
	using UNetConnection::UNetConnection;
};

class UPendingLevel : public UObject
{
public:
	using UObject::UObject;
};

class UNetPendingLevel : public UPendingLevel
{
public:
	using UPendingLevel::UPendingLevel;
};

class UDemoPlayPendingLevel : public UPendingLevel
{
public:
	using UPendingLevel::UPendingLevel;
};

class UChannel : public UObject
{
public:
	using UObject::UObject;
};

class UControlChannel : public UChannel
{
public:
	using UChannel::UChannel;
};

class UActorChannel : public UChannel
{
public:
	using UChannel::UChannel;
};

class UFileChannel : public UChannel
{
public:
	using UChannel::UChannel;
};
