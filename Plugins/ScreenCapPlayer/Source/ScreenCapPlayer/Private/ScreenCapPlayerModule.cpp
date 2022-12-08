// Copyright 2020 Antoine Reversat All Rights Reserved.

#include "ScreenCapPlayerModule.h"
#include "ScreenCapPlayer.h"

#define LOCTEXT_NAMESPACE "FScreenCapPlayerModule"

void FScreenCapPlayerModule::StartupModule()
{
	// supported platforms
	SupportedPlatforms.Add(TEXT("Windows"));

	// supported schemes
	SupportedUriSchemes.Add(TEXT("screencap"));

	// register media player info
	auto MediaModule = FModuleManager::LoadModulePtr<IMediaModule>("Media");

	if (MediaModule != nullptr)
	{
		MediaModule->RegisterPlayerFactory(*this);
	}
}

void FScreenCapPlayerModule::ShutdownModule()
{
	// unregister player factory
	auto MediaModule = FModuleManager::GetModulePtr<IMediaModule>("Media");

	if (MediaModule != nullptr)
	{
		MediaModule->UnregisterPlayerFactory(*this);
	}
}

bool FScreenCapPlayerModule::CanPlayUrl(const FString& Url, const IMediaOptions* Options, TArray<FText>* OutWarnings, TArray<FText>* OutErrors) const
{
	FString Scheme;
	FString Location;

	// check scheme
	if (!Url.Split(TEXT("://"), &Scheme, &Location, ESearchCase::CaseSensitive))
	{
		if (OutErrors != nullptr)
		{
			OutErrors->Add(LOCTEXT("NoSchemeFound", "No URI scheme found"));
		}

		return false;
	}

	if (!SupportedUriSchemes.Contains(Scheme))
	{
		if (OutErrors != nullptr)
		{
			OutErrors->Add(FText::Format(LOCTEXT("SchemeNotSupported", "The URI scheme '{0}' is not supported"), FText::FromString(Scheme)));
		}

		return false;
	}

	return true;
}

TSharedPtr<IMediaPlayer, ESPMode::ThreadSafe> FScreenCapPlayerModule::CreatePlayer(IMediaEventSink& EventSink)
{
	return MakeShared<FScreenCapPlayer, ESPMode::ThreadSafe>(EventSink);
}

FText FScreenCapPlayerModule::GetDisplayName() const
{
	return LOCTEXT("MediaPlayerDisplayName", "Screen Capture Player");
}

FName FScreenCapPlayerModule::GetPlayerName() const
{
	static FName PlayerName(TEXT("ScreenCapturePlayer"));
	return PlayerName;
}

#if !(ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26)
FGuid FScreenCapPlayerModule::GetPlayerPluginGUID() const
{
	static FGuid PlayerPluginGUID(0x7291E284, 0x4CB8EC1F, 0x4F9E8194, 0x2A6D9F73);
	return PlayerPluginGUID;
}
#endif

const TArray<FString>& FScreenCapPlayerModule::GetSupportedPlatforms() const
{
	return SupportedPlatforms;
}

bool FScreenCapPlayerModule::SupportsFeature(EMediaFeature Feature) const
{
	return Feature == EMediaFeature::VideoSamples;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FScreenCapPlayerModule, ScreenCapPlayer)