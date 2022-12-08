// Copyright 2020 Antoine Reversat All Rights Reserved.
#pragma once

#include "IMediaTextureSample.h"


class ScreenCapMediaTextureSample : public IMediaTextureSample {
public:
	ScreenCapMediaTextureSample(void* FrameBuffer, int Width, int Height, FTimespan Time, FTimespan Duration) : m_FrameBuffer(FrameBuffer), m_Dimensions(Width, Height), m_Time(Time), m_Duration(Duration) {};
	virtual const void* GetBuffer() override { return m_FrameBuffer; };
	virtual FTimespan GetDuration() const override { return FTimespan::Zero(); };
	virtual FIntPoint GetDim() const override { return m_Dimensions; };
	virtual EMediaTextureSampleFormat GetFormat() const override { return EMediaTextureSampleFormat::CharBMP; };
	virtual FIntPoint GetOutputDim() const override { return m_Dimensions; };
	virtual uint32 GetStride() const override { return m_Dimensions.X * 4; };
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
	virtual FTimespan GetTime() const override { return m_Time; };
#else
	virtual FMediaTimeStamp GetTime() const override { return FMediaTimeStamp(m_Time); };
#endif
	virtual bool IsCacheable() const override { return false; };
	virtual bool IsOutputSrgb() const override { return true; };
#if WITH_ENGINE
	virtual FRHITexture* GetTexture() const override { return nullptr; };
#endif
private:
	void* m_FrameBuffer;
	FIntPoint m_Dimensions;
	FTimespan m_Time;
	FTimespan m_Duration;
};