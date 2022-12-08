// Copyright Low Entry. All Rights Reserved.

#pragma once


#include "ILowEntryCompressionModule.h"


class FLowEntryCompressionModule : public ILowEntryCompressionModule
{
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
