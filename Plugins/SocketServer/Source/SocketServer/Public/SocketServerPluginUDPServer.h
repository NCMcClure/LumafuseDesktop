// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "SocketServer.h"
#include "SocketServerPluginUDPServer.generated.h"


class USocketServerBPLibrary;
class FServerUDPThread;
class FUDPClientSendDataToServerThread;

struct FSendUDPMessageStruct {
	FString			ip;
	int32			port;
	FString			message;
	TArray<uint8>	bytes;
	FSocket* socketUDP = nullptr;
};


UCLASS(Blueprintable, BlueprintType)
class SOCKETSERVER_API USocketServerPluginUDPServer : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void startUDPServer(IpAndPortStruct ipStruct,FString IP, int32 port, bool multicast, EReceiveFilterServer receiveFilter, FString serverID, int32 maxPacketSize);
	void stopUDPServer();
	void sendUDPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool asynchronous, ESocketServerUDPSocketType socketType);
	void sendUDPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool asynchronous, ESocketServerUDPSocketType socketType);
	void sendUDPMessageTo(FString ip, int32 port, FString message, TArray<uint8> byteArray, bool asynchronous);
	//do not work with ipv6
	//void UDPReceiver(const FArrayReaderPtr& ArrayReaderPtr, const FIPv4Endpoint& EndPt);
	void UDPReceiverSocketServerPlugin(FArrayReaderPtr& ArrayReaderPtr, TSharedRef<FInternetAddr> remoteAddress);

	IpAndPortStruct getServerIpAndPortStruct();
	FString getIP();
	int32 getPort();
	void setSocketReceiver(FUdpSocketReceiver* socketReceiverP, FSocket* socket);
	FUdpSocketReceiver* getSocketReceiver();
	FSocket* getSocket();
	FString getServerID();

	void addClientSession(FClientSocketSession session);
	FClientSocketSession* getClientSession(FString key);
	void removeClientSession(FString key);
	TMap<FString, FClientSocketSession> getClientSessions();
	void sendBytes(FSocket*& socket, TArray<uint8>& bytes, int32& sent, TSharedRef<FInternetAddr>& addr);

private:

	FString serverID;
	IpAndPortStruct ipAndPortStruct;
	FString serverIP;
	int32  serverPort = -1;
	int32 maxPacketSize = 65507;
	FSocket* socket= nullptr;
	FUdpSocketReceiver* socketReceiver = nullptr;
	EReceiveFilterServer receiveFilter;
	FServerUDPThread* serverThread = nullptr;
	FUDPClientSendDataToServerThread* sendThread = nullptr;

	TMap<FString, FClientSocketSession> clientSessions;
};


class SOCKETSERVER_API FServerUDPThread : public FRunnable {

public:

	FServerUDPThread(USocketServerPluginUDPServer* udpServerP, bool multicastP) :
		udpServer(udpServerP),
		multicast(multicastP) {
		FString threadName = "FServerUDPThread_" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override {

		IpAndPortStruct ipAndPortStruct = udpServer->getServerIpAndPortStruct();
		FSocket* listenerSocket = udpServer->getSocket();
		FUdpSocketReceiver* udpSocketReceiver = nullptr;


		FString ip = ipAndPortStruct.ip;
		FString adress = ip + ":" + FString::FromInt(udpServer->getPort());
		FString serverID = udpServer->getServerID();

		// create the socket
		FString socketName;
		ISocketSubsystem* socketSubsystem = USocketServerBPLibrary::getSocketSubSystem();


		if (multicast) {
			TSharedPtr<class FInternetAddr> addr = socketSubsystem->CreateInternetAddr();
			addr->SetAnyAddress();
			addr->SetPort(udpServer->getPort());


			listenerSocket = socketSubsystem->CreateSocket(NAME_DGram, *socketName, addr->GetProtocolType());
			if (listenerSocket == nullptr || listenerSocket == NULL) {
				const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
				AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | " + SocketErr, serverID);
					});
				thread = nullptr;
				return 0;
			}


			if (!listenerSocket->Bind(*addr)) {
				UE_LOG(LogTemp, Error, TEXT("Unable to open UDP Server"));
				const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
				AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | " + SocketErr, serverID);
					});
				thread = nullptr;
				return 0;
			}

			if (!listenerSocket->SetBroadcast(true)) {
				UE_LOG(LogTemp, Error, TEXT("Unable to set Broadcast"));
				const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
				AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to set Broadcast: " + adress + " | " + SocketErr, serverID);
					});
				thread = nullptr;
				return 0;
			}

			if (!listenerSocket->SetMulticastLoopback(true)) {
				UE_LOG(LogTemp, Error, TEXT("Unable to set Multicast Loopback"));
				const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
				AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to set Multicast Loopback: " + adress + " | " + SocketErr, serverID);
					});
				thread = nullptr;
				return 0;
			}
			bool validIP = true;
			addr->SetIp(*ip, validIP);

			if (!validIP) {
				UE_LOG(LogTemp, Error, TEXT("SocketServer UDP. Can't set ip"));
				AsyncTask(ENamedThreads::GameThread, [adress, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | Can't set ip.", serverID);
					});
				thread = nullptr;
				return 0;
			}


			if (!listenerSocket->JoinMulticastGroup(*addr)) {
				UE_LOG(LogTemp, Error, TEXT("Unable to join Multicast Group"));
				const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
				AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to join Multicast Group: " + adress + " | " + SocketErr, serverID);
					});
				thread = nullptr;
				return 0;
			}
		}
		else {

			TSharedPtr<class FInternetAddr> addr = socketSubsystem->CreateInternetAddr();
			bool validIP = true;
			addr->SetPort(udpServer->getPort());
			addr->SetIp(*ip, validIP);


			listenerSocket = socketSubsystem->CreateSocket(NAME_DGram, *socketName, addr->GetProtocolType());
			if (listenerSocket == nullptr || listenerSocket == NULL) {
				const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
				AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | " + SocketErr, serverID);
					});
				thread = nullptr;
				return 0;
			}

			if (!validIP) {
				UE_LOG(LogTemp, Error, TEXT("SocketServer UDP. Can't set ip"));
				AsyncTask(ENamedThreads::GameThread, [adress, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | Can't set ip.", serverID);
					});
				thread = nullptr;
				return 0;
			}

			listenerSocket->SetReuseAddr();
			listenerSocket->SetNonBlocking();
			if (!listenerSocket->Bind(*addr)) {
				UE_LOG(LogTemp, Error, TEXT("Unable to open UDP Server"));
				const TCHAR* SocketErr = socketSubsystem->GetSocketError(SE_GET_LAST_ERROR_CODE);
				AsyncTask(ENamedThreads::GameThread, [adress, SocketErr, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "Unable to open UDP Server: " + adress + " | " + SocketErr, serverID);
					});
				thread = nullptr;
				return 0;
			}
		}

		//do not work with ipv6
		FTimespan ThreadWaitTime = FTimespan::FromMilliseconds(100);
		//FString threadName = "SocketServerBPLibUDPReceiverThread_" + FString::FromInt(FDateTime::Now().GetTicks());

		//udpSocketReceiver = new FUdpSocketReceiver(listenerSocket, ThreadWaitTime, *threadName);
		//udpSocketReceiver->OnDataReceived().BindUObject(udpServer, &USocketServerPluginUDPServer::UDPReceiver);
		//udpSocketReceiver->Start();
		udpServer->setSocketReceiver(udpSocketReceiver, listenerSocket);

		//udpServer->initUDPClientThreads(listenerSocket);

		AsyncTask(ENamedThreads::GameThread, [adress, serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerUDPConnectionEventDelegate.Broadcast(true, "UDP Server started: " + adress, serverID);
		});


		//copy of FUdpSocketReceiver.h to get IPv6 working
		while (run) {
			if (!listenerSocket->Wait(ESocketWaitConditions::WaitForRead, ThreadWaitTime)){
				continue;
			}

			TSharedRef<FInternetAddr> Sender = socketSubsystem->CreateInternetAddr();
			uint32 Size;

			while (listenerSocket->HasPendingData(Size)){
				FArrayReaderPtr Reader = MakeShared<FArrayReader, ESPMode::ThreadSafe>(true);
				Reader->SetNumUninitialized(FMath::Min(Size, 65507u));

				int32 Read = 0;

				if (listenerSocket->RecvFrom(Reader->GetData(), Reader->Num(), Read, *Sender))
				{
					Reader->RemoveAt(Read, Reader->Num() - Read, false);
					udpServer->UDPReceiverSocketServerPlugin(Reader, Sender);
				//	UE_LOG(LogTemp, Error, TEXT("%s_%s"), *Sender.Get().ToString(true), *Sender->GetProtocolType().ToString());

				}
			}
		}

		if (listenerSocket != nullptr) {
			listenerSocket->Close();
			socketSubsystem->DestroySocket(listenerSocket);
			listenerSocket = nullptr;
		}

		if (thread != nullptr)
			thread = nullptr;

		return 0;
	}

	void stopThread() {
		run = false;
	}


protected:
	FString message;
	USocketServerPluginUDPServer* udpServer;
	bool multicast;
	FRunnableThread* thread = nullptr;
	bool run = true;

};



/*Send data asynchronous Thread*/
class SOCKETSERVER_API FUDPClientSendDataToServerThread : public FRunnable {

public:

	FUDPClientSendDataToServerThread(USocketServerPluginUDPServer* udpServerP) :
		udpServer(udpServerP) {
		FString threadName = "FUDPClientSendDataToServerThread_" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override {
		//FString serverID = tcpServer->getServerID();
		//socket = session.socket;
		//FString sessionID = session.sessionID;

		while (run && thread == nullptr) {
				FPlatformProcess::Sleep(0.1);
		}

		TSharedRef<FInternetAddr> addr = USocketServerBPLibrary::getSocketSubSystem()->CreateInternetAddr();
		int32 sent = 0;
		bool bIsValid = false;

		while (run) {	

			while (messageQueue.IsEmpty() == false) {
				FSendUDPMessageStruct messageStuct;
				messageQueue.Dequeue(messageStuct);


				addr->SetIp(*messageStuct.ip, bIsValid);
				addr->SetPort(messageStuct.port);

				if (bIsValid && messageStuct.socketUDP != nullptr) {

					if (messageStuct.bytes.Num() > 0) {
						udpServer->sendBytes(messageStuct.socketUDP, messageStuct.bytes, sent, addr);
					}

					if (messageStuct.message.Len() > 0) {
						FTCHARToUTF8 Convert(*messageStuct.message);
						messageStuct.bytes.Append((uint8*)Convert.Get(), Convert.Length());
						udpServer->sendBytes(messageStuct.socketUDP, messageStuct.bytes, sent, addr);
					}

				}
				else {
					UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Can't send data. Wrong adress."));
				}
					
			}
			

			if (run) {
				pauseThread(true);
				//workaround. suspend do not work on all platforms. lets sleep
				while (paused && run) {
					FPlatformProcess::Sleep(0.01);
				}
			}
		}

		run = false;
		thread = nullptr;


		return 0;
	}

	FRunnableThread* getThread() {
		return thread;
	}

	void setThread(FRunnableThread* threadP) {
		thread = threadP;
	}

	void stopThread() {
		run = false;
		if (thread != nullptr) {
			pauseThread(false);
		}
	}

	bool isRun() {
		return run;
	}


	void sendMessage(FString ip, int32	port, FString message,TArray<uint8>	bytes, FSocket* socketUDP) {
		FSendUDPMessageStruct messageStuct;
		messageStuct.ip = ip;
		messageStuct.port = port;
		messageStuct.message = message;
		messageStuct.bytes = bytes;
		messageStuct.socketUDP = socketUDP;
		messageQueue.Enqueue(messageStuct);
		pauseThread(false);
	}
	

	void pauseThread(bool pause) {
		paused = pause;
		thread->Suspend(pause);
	}

protected:
	USocketServerPluginUDPServer* udpServer;
	FRunnableThread* thread = nullptr;
	bool					run = true;
	bool					paused;
	TQueue<FSendUDPMessageStruct> messageQueue;
};