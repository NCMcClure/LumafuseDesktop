// Copyright 2020 Antoine Reversat All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Runtime/Launch/Resources/Version.h" 
#include "Containers/UnrealString.h"
#include "Serialization/Archive.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/prewindowsapi.h"
#pragma push_macro("WINVER")
#pragma push_macro("_WIN32_WINNT")
#undef WINVER
#undef _WIN32_WINNT
#define WINVER 0x0603
#define _WIN32_WINNT 0x0603
#include <WinUser.h>
#include <ShellScalingApi.h>
#pragma pop_macro("_WIN32_WINNT")
#pragma pop_macro("WINVER")
#include "Windows/PostWindowsApi.h"
#include "Windows/HideWindowsPlatformTypes.h"

#include "IMediaPlayer.h"
#include "IMediaCache.h"
#include "IMediaControls.h"
#include "IMediaview.h"
#include "IMediaTracks.h"
#include "IMediaSamples.h"

DECLARE_LOG_CATEGORY_EXTERN(LogScreenCapMediaPlayer, Log, All)

class IMediaEventSink;

struct Screen {
	FBox2D m_Coords;
	float m_Scale;

	Screen(FBox2D coords, float scale) : m_Coords(coords), m_Scale(scale) {};
};

class FScreenCapPlayer :
	public IMediaPlayer,
	protected IMediaCache,
	protected IMediaControls,
	protected IMediaView,
	protected IMediaTracks,
	protected IMediaSamples
{

public:

	FScreenCapPlayer(IMediaEventSink& InEventSink);
	virtual ~FScreenCapPlayer();
	void AddScreen(int x, int y, int width, int height, DEVICE_SCALE_FACTOR scale);

	// Inherited via IMediaPlayer
	virtual void Close() override;
	virtual IMediaCache& GetCache() override;
	virtual IMediaControls& GetControls() override;
	virtual FString GetInfo() const override;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
	virtual FName GetPlayerName() const override;
#else
	virtual FGuid GetPlayerPluginGUID() const override;
#endif
	virtual IMediaSamples& GetSamples() override;
	virtual FString GetStats() const override;
	virtual IMediaTracks& GetTracks() override;
	virtual FString GetUrl() const override;
	virtual IMediaView& GetView() override;
	virtual FText GetMediaName() const override;
	virtual void TickFetch(FTimespan DeltaTime, FTimespan Timecode) override;
	virtual bool Open(const FString& Url, const IMediaOptions* Options) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* Options) override;

protected:

	// Inherited via IMediaControls
	virtual bool CanControl(EMediaControl Control) const override;
	virtual FTimespan GetDuration() const override;
	virtual float GetRate() const override;
	virtual EMediaState GetState() const override;
	virtual EMediaStatus GetStatus() const override;
	virtual TRangeSet<float> GetSupportedRates(EMediaRateThinning Thinning) const override;
	virtual FTimespan GetTime() const override;
	virtual bool IsLooping() const override;
	virtual bool Seek(const FTimespan& Time) override;
	virtual bool SetLooping(bool Looping) override;
	virtual bool SetRate(float Rate) override;

	// Inherited via IMediaTracks
	virtual bool GetAudioTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaAudioTrackFormat& OutFormat) const override;
	virtual int32 GetNumTracks(EMediaTrackType TrackType) const override;
	virtual int32 GetNumTrackFormats(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual int32 GetSelectedTrack(EMediaTrackType TrackType) const override;
	virtual FText GetTrackDisplayName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual int32 GetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackLanguage(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual FString GetTrackName(EMediaTrackType TrackType, int32 TrackIndex) const override;
	virtual bool GetVideoTrackFormat(int32 TrackIndex, int32 FormatIndex, FMediaVideoTrackFormat& OutFormat) const override;
	virtual bool SelectTrack(EMediaTrackType TrackType, int32 TrackIndex) override;
	virtual bool SetTrackFormat(EMediaTrackType TrackType, int32 TrackIndex, int32 FormatIndex) override;

	// Inherited via IMediaSamples
	virtual bool  FetchVideo(TRange<FTimespan> TimeRange, TSharedPtr<IMediaTextureSample, ESPMode::ThreadSafe>& OutSample) override;
#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
	virtual int32 NumVideoSamples() const override { return 1; };
	virtual bool PeekVideoSampleTime(FMediaTimeStamp& TimeStamp) override;
#endif

private:
	void ParseArgs(FString Args);
	void CopyBuffer();
	float GetScaleAtPoint(FVector2D Point);
	void DrawCursor();

private:
	EMediaState m_CurrentState;
	FString m_MediaUrl;
	FString m_MediaName;
	FTimespan m_CurrentTime;
	FTimespan m_LastFrameTime;
	float m_FrameRate;
	bool m_FrameReady;
	float m_PlayRate;
	TArray<Screen> m_Screens;
	void* m_FrameBuffer;
	int m_FrameIdx;
	bool m_TaskRunning;
	bool m_CaptureCursor;
	bool m_ScaleScreen;

	HDC m_DstHdc;
	int m_Height;
	int m_Width;
	int m_X, m_Y;
	HBITMAP m_Bitmap;
};