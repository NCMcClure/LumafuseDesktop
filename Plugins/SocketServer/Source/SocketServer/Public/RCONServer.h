// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"
#include "RCONServer.generated.h"



UCLASS(Blueprintable, BlueprintType)
class SOCKETSERVER_API URCONServer : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	UFUNCTION()
		void receiveTCPMessageEvent(const FString sessionID, const FString message, const TArray<uint8>& byteArray, const FString serverID);

	void startRCONServer(FString serverID, ERCONPasswordType passwordType, FString passwordOrFile,
		bool& success, FString& errorMessage);

	bool sendResponse(FString sessionID, FString serverID, int32 id, int32 type, FString body);

private:
	FString passwordOrFile = FString();
	ERCONPasswordType passwordType = ERCONPasswordType::E_parameter;
	TArray<FString> commandList;

	void authResponse(FString sessionID, FString serverID, FString password, int32 rconID);
	
};