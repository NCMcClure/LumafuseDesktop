// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"
#include "SocketServerBPLibrary.h"
#include "DNSClientSocketServer.generated.h"

UCLASS()
class SOCKETSERVER_API UDNSClientSocketServer : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FresolveDomainEventDelegate, FString, IP);
	UFUNCTION()
		void resolveDomainEventDelegate(const FString IP);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|Events|ResolveDomain")
		FresolveDomainEventDelegate onresolveDomainEventDelegate;

	void resolveDomain(ISocketSubsystem * socketSubSystem, FString domain, bool useDNSCache = true, FString dnsIP = FString("8.8.8.8"));
	
	void UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);

	FSocket* socket = nullptr;

	bool isResloving();
	FString getIP();

private:
	bool resolving;
	FString ip;
	FString domain;
	TMap<FString, FString> dnsCache;
};