// Copyright 2020 Antoine Reversat All Rights Reserved.
#include "ScreenCapPlayer.h"
#include "ScreenCapMediaTextureSample.h"
#include "Async/Async.h"

DEFINE_LOG_CATEGORY(LogScreenCapMediaPlayer)

BOOL EnumMonitorCallback(
	HMONITOR Monitor,
	HDC Hdc,
	LPRECT Rect,
	LPARAM DwParam
) {
	FScreenCapPlayer* player = (FScreenCapPlayer*)DwParam;
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	if (GetMonitorInfoA(Monitor, &info)) {
		DEVICE_SCALE_FACTOR Scale;
		GetScaleFactorForMonitor(Monitor, &Scale);
		UE_LOG(LogScreenCapMediaPlayer, Verbose,
			TEXT("Got screen : x: %d, y: %d, w: %d, h: %d, scale %d"),
			info.rcMonitor.left, info.rcMonitor.top,
			info.rcMonitor.right - info.rcMonitor.left,
			info.rcMonitor.bottom - info.rcMonitor.top,
			Scale);
		player->AddScreen(
			info.rcMonitor.left,
			info.rcMonitor.top,
			info.rcMonitor.right,
			info.rcMonitor.bottom,
			Scale
		);
	}
	return 1;
}

FScreenCapPlayer::FScreenCapPlayer(IMediaEventSink& InEventSink) :
	m_CurrentState(EMediaState::Closed),
	m_CurrentTime(0),
	m_LastFrameTime(0),
	m_FrameRate(60.0),
	m_FrameReady(false),
	m_PlayRate(1.0),
	m_FrameIdx(0),
	m_TaskRunning(false),
	m_CaptureCursor(true),
	m_Height(0), m_Width(0)
{
	PROCESS_DPI_AWARENESS dpiAwareness;
	GetProcessDpiAwareness(GetCurrentProcess(), &dpiAwareness);
	UE_LOG(LogScreenCapMediaPlayer, Verbose, TEXT("DPI Awareness : %d"), dpiAwareness);
	if (dpiAwareness == 0 ) {
		m_ScaleScreen = true;
	} else {
		m_ScaleScreen = false;
	}

	EnumDisplayMonitors(NULL, NULL, EnumMonitorCallback, (LPARAM)this);
}

FScreenCapPlayer::~FScreenCapPlayer()
{
	Close();
}

void FScreenCapPlayer::AddScreen(int x_min, int y_min, int x_max, int y_max, DEVICE_SCALE_FACTOR scale)
{
	const FVector2D min((float)x_min, (float)y_min);
	if (m_ScaleScreen && scale != 100) {
		x_max = x_min + ((x_max - x_min)*(float(scale)/100.0));
		y_max = y_min + ((y_max - y_min)*(float(scale)/100.0));
	}
	const FVector2D max((float)x_max, (float)y_max);
	m_Screens.Add(Screen(FBox2D(min, max), float(scale)/100.0));
}

void FScreenCapPlayer::Close()
{
	m_CurrentState = EMediaState::Closed;
	m_MediaUrl = FString();
	m_MediaName = FString();
	m_CurrentTime = FTimespan(0);
	m_LastFrameTime = FTimespan(0);
	m_FrameReady = false;
	m_FrameIdx = 0;
	m_CaptureCursor = true;
	m_FrameRate = 60;

	if (m_Bitmap) 
		DeleteObject(m_Bitmap);

	if (m_DstHdc)
		DeleteDC(m_DstHdc);

}

IMediaCache& FScreenCapPlayer::GetCache()
{
	return *this;
}

IMediaControls& FScreenCapPlayer::GetControls()
{
	return *this;
}

FString FScreenCapPlayer::GetInfo() const
{

	return TEXT("");
}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
FName FScreenCapPlayer::GetPlayerName() const
{
	return FName(TEXT("ScreenCapturePlayer"));
}
#else
FGuid FScreenCapPlayer::GetPlayerPluginGUID() const
{
	static FGuid PlayerPluginGUID(0x7291E284, 0x4CB8EC1F, 0x4F9E8194, 0x2A6D9F73);
	return PlayerPluginGUID;
}
#endif

IMediaSamples& FScreenCapPlayer::GetSamples()
{
	return *this;
}

FString FScreenCapPlayer::GetStats() const
{
	return TEXT("");
}

IMediaTracks& FScreenCapPlayer::GetTracks()
{
	return *this;
}

FString FScreenCapPlayer::GetUrl() const
{
	return m_MediaUrl;
}

IMediaView& FScreenCapPlayer::GetView()
{
	return *this;
}

FText FScreenCapPlayer::GetMediaName() const
{
	return FText::FromString(m_MediaName);
}

void FScreenCapPlayer::TickFetch(FTimespan DeltaTime, FTimespan Timecode)
{
	if (m_CurrentState != EMediaState::Playing && m_CurrentState != EMediaState::Paused)
		return;

	m_CurrentTime += DeltaTime * m_PlayRate;
	if (m_CurrentTime >= m_LastFrameTime + FTimespan::FromSeconds(1) / m_FrameRate && !m_FrameReady) {
		if (!m_TaskRunning) {
			m_TaskRunning = true;
			Async(EAsyncExecution::TaskGraph, [this]() {
				CopyBuffer();
				if (m_CaptureCursor) {
					DrawCursor();
				}
				m_FrameReady = true;
				m_TaskRunning = false;
			});
		}
	}
}

bool FScreenCapPlayer::Open(const FString& Url, const IMediaOptions* Options)
{
	if (Url.IsEmpty() || !Url.StartsWith("screencap://")) {
		UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("Url is empty or doesn't start with screencap://"));
		return false;
	}

	TArray<FString> UrlParts;
	Url.ParseIntoArray(UrlParts, TEXT("/"), false);
	if (UrlParts.Num() < 3) {
		UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("Invalid Url"));
		return false;
	}

	FString OurUrl = UrlParts[2];
	int ScreenNum = -1;
	if (OurUrl.IsNumeric()) {
		ScreenNum = FCString::Atoi(*OurUrl);
	}
	else {
		if (!OurUrl.Contains(TEXT("?"))) {
			UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("Invalid Url : screen num must be a number."));
			return false;
		}

		TArray<FString> OurParts;
		OurUrl.ParseIntoArray(OurParts, TEXT("?"), false);
		if (!OurParts[0].IsNumeric()) {
			UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("Invalid Url : screen num must be a number."));
			return false;
		}
		else {
			ScreenNum = FCString::Atoi(*OurParts[0]);
			if (OurParts.Num() > 1) {
				ParseArgs(OurParts[1]);
			}
		}
	}

	if (ScreenNum < 0 || ScreenNum > m_Screens.Num()) {
		UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("%d is not a valid screen number (must be between 0 and %d)"), ScreenNum, m_Screens.Num());
		return false;
	}

	m_DstHdc = CreateCompatibleDC(NULL);
	if (!m_DstHdc) {
		UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("Unable to create compatible destination DC."));
		return false;
	}


	if (ScreenNum == 0) {
		float x_min = 0;
		float y_min = 0;
		float x_max = 0;
		float y_max = 0;
		for (auto Screen : m_Screens) {
			if (Screen.m_Coords.Min.X < x_min)
				x_min = Screen.m_Coords.Min.X;
			if (Screen.m_Coords.Min.Y < y_min)
			    y_min = Screen.m_Coords.Min.Y;
			if (Screen.m_Coords.Max.X > x_max)
				x_max = Screen.m_Coords.Max.X;
			if (Screen.m_Coords.Max.Y > y_max)
				y_max = Screen.m_Coords.Max.Y;
		}
		m_X = x_min;
		m_Y = y_min;
		m_Height = y_max - y_min;
		m_Width = x_max - x_min;
		m_MediaName = FString("Desktop");
	}
	else {
		m_X = m_Screens[ScreenNum - 1].m_Coords.Min.X;
		m_Y = m_Screens[ScreenNum - 1].m_Coords.Min.Y;
		FVector2D size = m_Screens[ScreenNum - 1].m_Coords.GetSize();
		m_Height = size.Y;
		m_Width = size.X;
		m_MediaName = FString::Printf(TEXT("Monitor %02d"), ScreenNum);
	}
	UE_LOG(LogScreenCapMediaPlayer, Verbose, TEXT("Using x: %d, y: %d, w: %d, h: %d"), m_X, m_Y, m_Width, m_Height);

	const BITMAPINFO bi = { sizeof(BITMAPINFOHEADER), m_Width, m_Height, 1, 32, BI_RGB, 0, 0, 0, 0, 0 };

	m_Bitmap = CreateDIBSection(m_DstHdc, &bi, DIB_RGB_COLORS, (void **)&m_FrameBuffer, NULL, NULL);
	if (!m_Bitmap) {
		UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("Unable to create bitmap."));
		DeleteDC(m_DstHdc);
		return false;
	}

	SelectObject(m_DstHdc, m_Bitmap);

	m_CurrentState = EMediaState::Playing;

	return true;
}

bool FScreenCapPlayer::Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* Options)
{
	return false;
}

bool FScreenCapPlayer::CanControl(EMediaControl Control) const
{
	return Control == EMediaControl::Pause || Control == EMediaControl::Resume;
}

FTimespan FScreenCapPlayer::GetDuration() const
{
	return m_CurrentTime;
}

float FScreenCapPlayer::GetRate() const
{
	return m_PlayRate;
}

EMediaState FScreenCapPlayer::GetState() const
{
	return m_CurrentState;
}

EMediaStatus FScreenCapPlayer::GetStatus() const
{
	return EMediaStatus::None;
}

TRangeSet<float> FScreenCapPlayer::GetSupportedRates(EMediaRateThinning Thinning) const
{
	TRangeSet<float> SupportedRates;
	SupportedRates.Add(TRange<float>(0.0));
	SupportedRates.Add(TRange<float>(1.0));
	return SupportedRates;
}

FTimespan FScreenCapPlayer::GetTime() const
{
	return m_CurrentTime;
}

bool FScreenCapPlayer::IsLooping() const
{
	return false;
}

bool FScreenCapPlayer::Seek(const FTimespan& Time)
{
	return false;
}

bool FScreenCapPlayer::SetLooping(bool Looping)
{
	return false;
}

bool FScreenCapPlayer::SetRate(float Rate)
{
	m_PlayRate = Rate;
	if (Rate == 1.0) {
		m_CurrentState = EMediaState::Playing;
		return true;
	}
	else if (Rate == 0.0) {
		m_CurrentState = EMediaState::Paused;
		return true;
	}
	return false;
}

bool FScreenCapPlayer::GetAudioTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaAudioTrackFormat& OutFormat) const
{
	return false;
}

int32 FScreenCapPlayer::GetNumTracks(EMediaTrackType TrackType) const
{
	if (TrackType == EMediaTrackType::Video) {
		return 1;
	}
	else {
		return 0;
	}
}

int32 FScreenCapPlayer::GetNumTrackFormats(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if (TrackType == EMediaTrackType::Video) {
		return 1;
	}
	return 0;
}

int32 FScreenCapPlayer::GetSelectedTrack(EMediaTrackType TrackType) const
{
	if (TrackType == EMediaTrackType::Video) {
		return 1;
	}
	return INDEX_NONE;
}

FText FScreenCapPlayer::GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return FText();
}

int32 FScreenCapPlayer::GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const
{
	if (TrackType == EMediaTrackType::Video)
		return 0;
	return int32();
}

FString FScreenCapPlayer::GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return FString();
}

FString FScreenCapPlayer::GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const
{
	return FString();
}

bool FScreenCapPlayer::GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaVideoTrackFormat& OutFormat) const
{
	if (TrackIndex == 1 && FormatIndex == 0) {
		FMediaVideoTrackFormat format;
		format.Dim = FIntPoint(m_Width, m_Height);
		format.FrameRate = m_FrameRate;
		format.FrameRates = TRange<float>(0.0, 60.0);
		format.TypeName = FString(TEXT("ScreenCap format"));
		OutFormat = format;
		return true;
	}
	return false;
}

bool FScreenCapPlayer::SelectTrack(EMediaTrackType TrackType, int32 TrackIndex)
{
	if (TrackType == EMediaTrackType::Video)
		return true;
	return false;
}

bool FScreenCapPlayer::SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex)
{
	if (TrackType == EMediaTrackType::Video)
		return true;
	return false;
}

bool FScreenCapPlayer::FetchVideo(TRange<FTimespan> TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample)
{
	if (m_CurrentState != EMediaState::Playing && m_CurrentState != EMediaState::Paused)
		return false;

	if (m_FrameReady) {
		auto sample = MakeShared<ScreenCapMediaTextureSample, ESPMode::ThreadSafe>(m_FrameBuffer,
			m_Width, m_Height,
			m_CurrentTime,
			FTimespan::FromSeconds(1)/m_FrameRate);
		OutSample = sample;
		m_LastFrameTime = m_CurrentTime;
		m_FrameReady = false;
		m_FrameIdx++;
		return true;
	}

	return false;
}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
bool FScreenCapPlayer::PeekVideoSampleTime(FMediaTimeStamp& TimeStamp)
{
	return false;
}
#endif

void FScreenCapPlayer::ParseArgs(FString Args)
{
	TArray<FString> ArgArray;
	Args.ParseIntoArray(ArgArray, TEXT("&"), false);
	for (FString Arg : ArgArray) {
		FString Var;
		FString Val;
		if (Arg.Split(TEXT("="), &Var, &Val)) {
			if (Var.ToLower() == TEXT("fps") && Val.IsNumeric()) {
				float fps = FCString::Atof(*Val);
				if (fps > 0 && fps <= 60) {
					m_FrameRate = fps;
				}
			}

			if (Var.ToLower() == TEXT("capture_cursor")) {
				if (Val.ToLower() == "true")
					m_CaptureCursor = true;

				if (Val.ToLower() == "false")
					m_CaptureCursor = false;
			}

			if (Var.ToLower() != TEXT("fps") && Var.ToLower() != TEXT("capture_cursor")) {
				UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("Unrecognized argument %s"), *Var);
			}
		}
	}
}

void FScreenCapPlayer::CopyBuffer()
{
	HDC Hdc = GetDC(NULL);
	if (Hdc) {
		BitBlt(m_DstHdc, 0, 0, m_Width, m_Height, Hdc, m_X, m_Y, SRCCOPY);
		ReleaseDC(NULL, Hdc);
	} else {
		UE_LOG(LogScreenCapMediaPlayer, Error, TEXT("Unable to get source DC."));
	}
}

float FScreenCapPlayer::GetScaleAtPoint(FVector2D Point) 
{
	if (!m_ScaleScreen)
		return 1.0f;

	for(auto Screen : m_Screens) {
		if (Screen.m_Coords.IsInside(Point)) {
			return Screen.m_Scale;
		}
	}
	return 1.0f;
	
}

void FScreenCapPlayer::DrawCursor()
{
	CURSORINFO CurInfos;
	CurInfos.cbSize = sizeof(CURSORINFO);
	if (GetCursorInfo(&CurInfos)) {
		if (CurInfos.flags > 0) {
			HICON icon = CopyIcon(CurInfos.hCursor);
			float scale = GetScaleAtPoint(FVector2D(CurInfos.ptScreenPos.x, CurInfos.ptScreenPos.y));
			if (icon) {
				DrawIcon(m_DstHdc,
						scale*(CurInfos.ptScreenPos.x - m_X),
						scale*(CurInfos.ptScreenPos.y - m_Y),
						icon);
				DestroyIcon(icon);
			}
		}
	}
}
