// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.
#pragma once

#include "SocketServer.h"
#include "SocketServerPluginTCPServer.generated.h"


class USocketServerBPLibrary;
class FServerTCPThread;
class FTCPFileHandlerThread;

UCLASS(Blueprintable, BlueprintType)
class SOCKETSERVER_API USocketServerPluginTCPServer : public UObject
{
	GENERATED_UCLASS_BODY()

public:

	void startTCPServer(IpAndPortStruct ipStructP,FString IP, int32 port, EReceiveFilterServer receiveFilter, FString serverID, bool isFileServer, FString Aes256bitKey, bool resumeFiles);
	void stopTCPServer();
	void sendTCPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak);
	void sendTCPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak);

	IpAndPortStruct getServerIpAndPortStruct();
	FString getIP();
	int32 getPort();
	FString getServerID();
	bool hasResume();

	void initTCPClientThreads(FClientSocketSession session, EReceiveFilterServer receiveFilter);

	void addClientSession(FClientSocketSession session);
	FClientSocketSession* getClientSession(FString key);
	void removeClientSession(FString key);
	TMap<FString, FClientSocketSession> getClientSessions();
	//EHTTPSocketServerFileDownloadResumeType getifFileExistThen();


	FString encryptMessage(FString message);
	FString decryptMessage(FString message);

	struct FSocketServerToken getTokenStruct(FString token);
	void removeTokenFromStruct(FString token);
	FString getCleanDir(EFileFunctionsSocketServerDirectoryType directoryType, FString fileDirectory);
	void getMD5FromFile(FString filePathP, bool& success, FString& MD5);
	void deleteFile(FString filePathP);
	int64 fileSize(FString filePathP);
	FString int64ToString(int64 num);

	bool isRun();

private:
	bool run = true;
	FString serverID = FString();
	IpAndPortStruct ipAndPortStruct;
	FString serverIP = FString();
	int32  serverPort = -1;
	EReceiveFilterServer receiveFilter;
	bool fileServer = false;
	FString aesKey = FString();
	bool resumeFiles = false;
	//FString downloadDir;

	FServerTCPThread* serverThread = nullptr;
	FTCPFileHandlerThread* TCPFileHandlerThread = nullptr;

	TMap<FString, FClientSocketSession> clientSessions;
};


/*SERVER start and create client sockets asynchronous Thread*/
class SOCKETSERVER_API FServerTCPThread : public FRunnable {

public:

	FServerTCPThread(USocketServerPluginTCPServer* tcpServerP, EReceiveFilterServer receiveFilterP) :
		tcpServer(tcpServerP),
		receiveFilter(receiveFilterP) {
		FString threadName = "FServerTCPThread_" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override {

		IpAndPortStruct ipAndPortStruct = tcpServer->getServerIpAndPortStruct();
		int32 port = tcpServer->getPort();
		FString serverID = tcpServer->getServerID();

		bool createServer = true;
		FSocket* listenerSocket = nullptr;
		ISocketSubsystem* socketSubSystem = USocketServerBPLibrary::getSocketSubSystem();
		if (socketSubSystem != nullptr){
			
			TSharedRef<FInternetAddr> internetAddr = socketSubSystem->CreateInternetAddr();
			internetAddr->SetIp(*ipAndPortStruct.ip, ipAndPortStruct.success);
			internetAddr->SetPort(ipAndPortStruct.port);

			if (ipAndPortStruct.success) {
				listenerSocket = socketSubSystem->CreateSocket(NAME_Stream, *FString("USocketServerBPLibraryListenerSocket"), internetAddr->GetProtocolType());
			}

			if (listenerSocket != nullptr){
				listenerSocket->SetLinger(false, 0);	

				if (!listenerSocket->Bind(*internetAddr)) {
					if (listenerSocket != nullptr) {
						listenerSocket->Close();
						if (socketSubSystem != nullptr)
							socketSubSystem->DestroySocket(listenerSocket);
						listenerSocket = nullptr;
						thread = nullptr;
						UE_LOG(LogTemp, Error, TEXT("(211) TCP Server not started. Can't bind %s:%i. Please check IP,Port or your firewall."), *ipAndPortStruct.ip, port);
					}
					createServer = false;
				}


				if (createServer && !listenerSocket->Listen(8)) {
					if (listenerSocket != nullptr) {
						listenerSocket->Close();
						if (socketSubSystem != nullptr)
							socketSubSystem->DestroySocket(listenerSocket);
						listenerSocket = nullptr;
						thread = nullptr;
						UE_LOG(LogTemp, Error, TEXT("(212) TCP Server not started. Can't listen on %s:%i. Please check IP,Port or your firewall."), *ipAndPortStruct.ip, port);
					}
					createServer = false;
				}
			}

		}



		if (!createServer || listenerSocket == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("(210) TCP Server not started on %s:%i. Please check IP,Port or your firewall."), *ipAndPortStruct.ip, port);
			AsyncTask(ENamedThreads::GameThread, [serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, false, "TCP Server not started. Please check IP,Port or your firewall.", TEXT(""),serverID);
				USocketServerBPLibrary::getSocketServerTarget()->stopTCPServer(serverID);
			});
		}
		else {
			//switch to gamethread
			AsyncTask(ENamedThreads::GameThread, [serverID]() {
				USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, true, "TCP Server started.", TEXT(""), serverID);
			});

			while (run) {
				bool pending;
				listenerSocket->WaitForPendingConnection(pending, FTimespan::FromSeconds(1));
				if (pending) {
					//UE_LOG(LogTemp, Display, TEXT("TCP Client: Pending connection"));

					FClientSocketSession session;
					session.sessionID = FGuid::NewGuid().ToString();
					session.serverID = serverID;

					TSharedRef<FInternetAddr> remoteAddress = USocketServerBPLibrary::getSocketSubSystem()->CreateInternetAddr();
					FSocket* clientSocket = listenerSocket->Accept(*remoteAddress, session.sessionID);

					session.ip = remoteAddress.Get().ToString(false);
					session.port = remoteAddress.Get().GetPort();
					session.socket = clientSocket;
					session.protocol = EServerSocketConnectionProtocol::E_TCP;
					tcpServer->addClientSession(session);
					tcpServer->initTCPClientThreads(session, receiveFilter);
				}
			}
		}


		if (listenerSocket && listenerSocket != nullptr) {
			listenerSocket->Close();
			FPlatformProcess::Sleep(1);
			if (socketSubSystem != nullptr)
				socketSubSystem->DestroySocket(listenerSocket);
		}

		listenerSocket = nullptr;
		thread = nullptr;

		//switch to gamethread
		AsyncTask(ENamedThreads::GameThread, [serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Server, false, "TCP Server stopped. Depending on the operating system it can take some time until the port is free again.", TEXT(""),serverID);
		});
		return 0;
	}

	void stopThread() {
		run = false;
	}


protected:
	USocketServerPluginTCPServer* tcpServer;
	EReceiveFilterServer	receiveFilter;
	FRunnableThread* thread = nullptr;
	bool run = true;
};


/*Send data asynchronous Thread*/
class SOCKETSERVER_API FTCPClientSendDataToServerThread : public FRunnable {

public:

	FTCPClientSendDataToServerThread(USocketServerPluginTCPServer* tcpServerP, FClientSocketSession sessionP) :
		tcpServer(tcpServerP),
		session(sessionP) {
		FString threadName = "FTCPClientSendDataToServerThread_" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override {
		FString serverID = tcpServer->getServerID();
		socket = session.socket;
		FString sessionID = session.sessionID;

		//message wrapping
		FString tcpMessageHeader = FString();
		FString tcpMessageFooter = FString();
		ESocketServerTCPMessageWrapping messageWrapping = ESocketServerTCPMessageWrapping::E_None;

		USocketServerBPLibrary::socketServerBPLibrary->getTcpWrapping(tcpMessageHeader, tcpMessageFooter, messageWrapping);

		while (run) {

			//Wait a bit in case someone tries to send something right after the connection is established to avoid hitting a dead connection.
			if (waitForInit) {
				waitForInit = false;
				FPlatformProcess::Sleep(0.5);
			}

		/*	if (thread == nullptr) {
				FPlatformProcess::Sleep(0.1);
				continue;
			}*/

			if (socket == nullptr) {
				UE_LOG(LogTemp, Error, TEXT("Socket not found."));
				return 0;
			}


			// try to connect to the server
			if (socket == nullptr || run == false) {
				UE_LOG(LogTemp, Error, TEXT("Connection not exist."));
				//switch to gamethread
				AsyncTask(ENamedThreads::GameThread, [sessionID, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, false, "Connection not exist", sessionID, serverID);
					});
				return 0;
			}


			int32 sent = 0;
			if (socket != nullptr && socket->GetConnectionState() == ESocketConnectionState::SCS_Connected) {

				

				while (messageQueue.IsEmpty() == false) {
					FString m;
					messageQueue.Dequeue(m);
					FTCHARToUTF8 Convert(*m);
					sent = 0;

					if (messageWrapping == ESocketServerTCPMessageWrapping::E_Byte) {
						TArray<uint8> byteCache;
						if (FGenericPlatformProperties::IsLittleEndian()) {
							byteCache.Add(0x00);
						}
						else {
							byteCache.Add(0x01);
						}
						byteCache.SetNum(5);
						int32 dataLength = Convert.Length();
						FMemory::Memcpy(byteCache.GetData()+1,&dataLength,4);
						byteCache.Append((uint8*)Convert.Get(), Convert.Length());
						socket->Send(byteCache.GetData(), byteCache.Num(), sent);
						byteCache.Empty();
					}
					else {
						socket->Send((uint8*)Convert.Get(), Convert.Length(), sent);
					}
				}

				while (byteArrayQueue.IsEmpty() == false) {
					TArray<uint8> ba;
					byteArrayQueue.Dequeue(ba);
					sent = 0;
					if (messageWrapping == ESocketServerTCPMessageWrapping::E_Byte) {
						TArray<uint8> byteCache;
						if (FGenericPlatformProperties::IsLittleEndian()) {
							byteCache.Add(0x00);
						}
						else {
							byteCache.Add(0x01);
						}
						byteCache.SetNum(5);
						int32 dataLength = ba.Num();
						FMemory::Memcpy(byteCache.GetData() + 1, &dataLength, 4);
						byteCache.Append(ba.GetData(), ba.Num());
						socket->Send(byteCache.GetData(), byteCache.Num(), sent);
					}
					else {
						socket->Send(ba.GetData(), ba.Num(), sent);
					}
					ba.Empty();
				}
		

			}
			else {
				//UE_LOG(LogTemp, Error, TEXT("Connection lost"));
				AsyncTask(ENamedThreads::GameThread, [sessionID, serverID]() {
					USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, false, "Connection lost", sessionID, serverID);
				});
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


	void setMessage(FString messageP, TArray<uint8> byteArrayP) {
		if (messageP.Len() > 0)
			messageQueue.Enqueue(messageP);
		if (byteArrayP.Num() > 0)
			byteArrayQueue.Enqueue(byteArrayP);
	}

	void sendMessage(FString messageP, TArray<uint8> byteArrayP) {
		if (messageP.Len() > 0)
			messageQueue.Enqueue(messageP);
		if (byteArrayP.Num() > 0)
			byteArrayQueue.Enqueue(byteArrayP);
		pauseThread(false);
	}



	void pauseThread(bool pause) {
		paused = pause;
		thread->Suspend(pause);
	}

protected:
	USocketServerPluginTCPServer* tcpServer = nullptr;
	FClientSocketSession session;
	FRunnableThread* thread = nullptr;
	FSocket* socket = nullptr;
	bool					run = true;
	bool					paused;
	bool waitForInit = true;
	TQueue<FString> messageQueue;
	TQueue<TArray<uint8>> byteArrayQueue;
};


/*Receive Messages or bytes from client. Thread*/
class SOCKETSERVER_API FTCPClientReceiveDataFromServerThread : public FRunnable {

public:

	FTCPClientReceiveDataFromServerThread(USocketServerPluginTCPServer* tcpServerP,FClientSocketSession sessionP, EReceiveFilterServer receiveFilterP) :
		tcpServer(tcpServerP),
		session(sessionP),
		receiveFilter(receiveFilterP){
		FString threadName = "FTCPClientReceiveDataFromServerThread" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override {
		FString serverID = tcpServer->getServerID();
		FSocket* clientSocket = session.socket;
		FString sessionID = session.sessionID;

		//message wrapping
		FString tcpMessageHeader = FString();
		FString tcpMessageFooter = FString();
		ESocketServerTCPMessageWrapping messageWrapping = ESocketServerTCPMessageWrapping::E_None;

		USocketServerBPLibrary::socketServerBPLibrary->getTcpWrapping(tcpMessageHeader, tcpMessageFooter, messageWrapping);


		FString tcpMessageFooterLineBreak = tcpMessageFooter + "\r\n";
		FString tcpMessageFooterLineBreak2 = tcpMessageFooter + "\r";;
				

		//switch to gamethread
		AsyncTask(ENamedThreads::GameThread, [sessionID, serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, true, "Client connected", sessionID, serverID);
		});

		uint32 DataSize = 0;
		//FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));
		TArray<uint8> dataFromSocket;
		int64 ticks1;
		int64 ticks2;
		TArray<uint8> byteDataArray;
		TArray<uint8> byteDataArrayCache;
		FString mainMessage;
		bool inCollectMessageStatus = false;
		bool deathConnection = false;
		bool hasData = false;
		int32 lastDataLengthFromHeader = 0;

		while (run && clientSocket != nullptr && tcpServer->isRun()) {
			//ESocketConnectionState::SCS_Connected does not work https://issues.unrealengine.com/issue/UE-27542
			//Compare ticks is a workaround to get a disconnect. clientSocket->Wait() stop working after disconnect. (Another bug?)
			//If it doesn't wait any longer, ticks1 and ticks2 should be the same == disconnect.
			ticks1 = FDateTime::Now().GetTicks();
			clientSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(1));
			ticks2 = FDateTime::Now().GetTicks();


			hasData = false;
			
			if (tcpServer->isRun()) {
				hasData = clientSocket->HasPendingData(DataSize);
			}
			else {
				deathConnection = true;
			}

			if (!hasData && ticks1 == ticks2) {
				//UE_LOG(LogTemp, Display, TEXT("TCP End xxx: %s:%i"), *session.ip, session.port);
				break;
			}
			if (hasData) {

				dataFromSocket.SetNumUninitialized(DataSize);
				int32 BytesRead = 0;
				if (clientSocket->Recv(dataFromSocket.GetData(), dataFromSocket.Num(), BytesRead)) {
					switch (messageWrapping)
					{
					case ESocketServerTCPMessageWrapping::E_None:
						triggerMessageEvent(dataFromSocket, sessionID, serverID);
						break;
					case ESocketServerTCPMessageWrapping::E_String:
						if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_S) {
							dataFromSocket.Add(0x00);// null-terminator
							char* Data = (char*)dataFromSocket.GetData();


							FString recvMessage = FString(UTF8_TO_TCHAR(Data));
							if (recvMessage.StartsWith(tcpMessageHeader)) {
								inCollectMessageStatus = true;
								recvMessage.RemoveFromStart(tcpMessageHeader);
							}
							if (recvMessage.EndsWith(tcpMessageFooter) || recvMessage.EndsWith(tcpMessageFooterLineBreak) || recvMessage.EndsWith(tcpMessageFooterLineBreak2)) {
								inCollectMessageStatus = false;
								if (!recvMessage.RemoveFromEnd(tcpMessageFooter)) {
									if (!recvMessage.RemoveFromEnd(tcpMessageFooterLineBreak)) {
										if (recvMessage.RemoveFromEnd(tcpMessageFooterLineBreak2)) {
											recvMessage.Append("\r");
										}
									}
									else {
										recvMessage.Append("\r\n");
									}
								}

								//splitt merged messages
								if (recvMessage.Contains(tcpMessageHeader)) {
									TArray<FString> lines;
									int32 lineCount = recvMessage.ParseIntoArray(lines, *tcpMessageHeader, true);
									for (int32 i = 0; i < lineCount; i++) {
										mainMessage = lines[i];
										if (mainMessage.EndsWith(tcpMessageFooter) || mainMessage.EndsWith(tcpMessageFooterLineBreak) || mainMessage.EndsWith(tcpMessageFooterLineBreak2)) {
											if (!mainMessage.RemoveFromEnd(tcpMessageFooter)) {
												if (!mainMessage.RemoveFromEnd(tcpMessageFooterLineBreak)) {
													if (mainMessage.RemoveFromEnd(tcpMessageFooterLineBreak2)) {
														mainMessage.Append("\r");
													}
												}
												else {
													mainMessage.Append("\r\n");
												}
											}
										}

										//switch to gamethread
										AsyncTask(ENamedThreads::GameThread, [mainMessage, sessionID, byteDataArray, serverID]() {
											//UE_LOG(LogTemp, Display, TEXT("TCP:%s"), *recvMessage);
											USocketServerBPLibrary::socketServerBPLibrary->onserverReceiveTCPMessageEventDelegate.Broadcast(sessionID, mainMessage, byteDataArray, serverID);
											if (USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID) != nullptr) {
												USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID)->onregisteredEventDelegate.Broadcast(mainMessage, byteDataArray);
											}
											});

										dataFromSocket.Empty();
										mainMessage.Empty();
									}
									continue;
								}
								else {
									mainMessage.Append(recvMessage);
								}


							}
							if (inCollectMessageStatus) {
								mainMessage.Append(recvMessage);
								continue;
							}
							if (mainMessage.IsEmpty()) {
								continue;
							}

						}
						break;
					case ESocketServerTCPMessageWrapping::E_Byte:
	

						if (lastDataLengthFromHeader == 0 && dataFromSocket.Num() >= 5) {

							readDataLength(dataFromSocket, lastDataLengthFromHeader);

							if (dataFromSocket.Num() == 5) {
								dataFromSocket.Empty();
								continue;
							}

							byteDataArrayCache.Append(dataFromSocket.GetData() + 5, dataFromSocket.Num() - 5);
							dataFromSocket.Empty();
						}
						else {
							byteDataArrayCache.Append(dataFromSocket.GetData(), dataFromSocket.Num());
						}

						int32 maxLoops = 1000;//to prevent endless loop

						while (byteDataArrayCache.Num() > 0 && byteDataArrayCache.Num() >= lastDataLengthFromHeader && maxLoops > 0) {
							maxLoops--;

							byteDataArray.Append(byteDataArrayCache.GetData(), lastDataLengthFromHeader);
							byteDataArrayCache.RemoveAt(0, lastDataLengthFromHeader, true);


							triggerMessageEvent(byteDataArray, sessionID, serverID);
							//UE_LOG(LogTemp, Display, TEXT("%s"), *mainMessage);
							byteDataArray.Empty();

							if (byteDataArrayCache.Num() == 0) {
								lastDataLengthFromHeader = 0;
								break;
							}

							if (byteDataArrayCache.Num() > 5) {
								readDataLength(byteDataArrayCache, lastDataLengthFromHeader);
								byteDataArrayCache.RemoveAt(0, 5, true);
							}

						}
						break;
					}
				}
				dataFromSocket.Empty();
				mainMessage.Empty();
			}
		}
			

		//UE_LOG(LogTemp, Display, TEXT("TCP Connected: %s:%i"), *session.ip, session.port);

		//switch to gamethread
		USocketServerPluginTCPServer* tcpServerGlobal = tcpServer;
		AsyncTask(ENamedThreads::GameThread, [sessionID, serverID, tcpServerGlobal]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, false, "Client disconnected", sessionID, serverID);
			//clean up socket in main thread because race condition
			tcpServerGlobal->removeClientSession(sessionID);
		});

		thread = nullptr;

		if (run && tcpServer->isRun() == false && deathConnection) {
			return 0;
		}

		if (clientSocket != nullptr) {
			clientSocket->Close();			
		}

		ISocketSubsystem* sSS = USocketServerBPLibrary::socketServerBPLibrary->getSocketSubSystem();
		if (sSS != nullptr) {
			sSS->DestroySocket(clientSocket);
			sSS = nullptr;
		}

		clientSocket = nullptr;

		return 0;
	}

	void triggerMessageEvent(TArray<uint8>& byteDataArray, FString& sessionID, FString& serverID) {
		//if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_B) {
		//	byteDataArray.Append(dataFromSocket.GetData(), dataFromSocket.Num());
		//}

		FString mainMessage = FString();
		if (receiveFilter == EReceiveFilterServer::E_SAB || receiveFilter == EReceiveFilterServer::E_S) {
			byteDataArray.Add(0x00);// null-terminator
			char* Data = (char*)byteDataArray.GetData();
			mainMessage = FString(UTF8_TO_TCHAR(Data));
			if (receiveFilter == EReceiveFilterServer::E_S) {
				byteDataArray.Empty();
			}
		}



		//switch to gamethread
		AsyncTask(ENamedThreads::GameThread, [mainMessage, sessionID, byteDataArray, serverID]() {
			//UE_LOG(LogTemp, Display, TEXT("TCP:%s"), *recvMessage);
			USocketServerBPLibrary::socketServerBPLibrary->onserverReceiveTCPMessageEventDelegate.Broadcast(sessionID, mainMessage, byteDataArray, serverID);
			if (USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID) != nullptr) {
				USocketServerBPLibrary::socketServerBPLibrary->getResiteredClientEvent(sessionID)->onregisteredEventDelegate.Broadcast(mainMessage, byteDataArray);
			}
		});
		mainMessage.Empty();
	}

	void readDataLength(TArray<uint8>& byteDataArray, int32& byteLenght) {
		if (FGenericPlatformProperties::IsLittleEndian() && byteDataArray[0] == 0x00) {
			FMemory::Memcpy(&byteLenght, byteDataArray.GetData() + 1, 4);
		}
		else {
			//endian fits not. swap bytes that contains the length
			byteDataArray.SwapMemory(1, 4);
			byteDataArray.SwapMemory(2, 3);
			FMemory::Memcpy(&byteLenght, byteDataArray.GetData() + 1, 4);
		}
	}

	void stopThread() {
		run = false;
	}

protected:
	USocketServerPluginTCPServer* tcpServer = nullptr;
	FClientSocketSession session;
	EReceiveFilterServer receiveFilter;
	FRunnableThread* thread = nullptr;
	bool run = true;
};



//send files to client
class SOCKETSERVER_API FTCPClientSendFileToClientThread : public FRunnable {

public:

	FTCPClientSendFileToClientThread(FSocket* socketP ,FString filePathP,int64 fileSizeP,int64 startPositionP, FClientSocketSession sessionP) :
		socket(socketP),
		filePath(filePathP),
		fileSize(fileSizeP),
		startPosition(startPositionP),
		session(sessionP) {
		FString threadName = "FTCPClientSendFileToClientThread_" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override {

		if (socket == nullptr) {

			run = false;
			thread = nullptr;
			return 0;
		}

		FArchive* reader = IFileManager::Get().CreateFileReader(*filePath);
		if (reader == nullptr || reader->TotalSize() == 0) {
			if (reader != nullptr) {
				reader->Close();
			}
			delete reader;
			return false;
		}


		FString sessionID = session.sessionID;
		fileSize = reader->TotalSize();
		int64 readSize = 0;
		int64 lastPosition = startPosition;
		int64 bytesSentSinceLastTick = 0;

		int32 bufferSize = 1024 * 64;
		int32 dataSendBySocket = 0;

		float percent = 0.f;
		float mbit = 0.f;

		TArray<uint8> buffer;

		int64 lastTimeTicks = FDateTime::Now().GetTicks();

		if (bufferSize > fileSize) {
			bufferSize = fileSize;
		}

		if (lastPosition > 0) {
			reader->Seek(lastPosition);
		}

		while (run && lastPosition < fileSize) {

			if ((lastPosition + bufferSize) > fileSize) {
				bufferSize = fileSize - lastPosition;
			}

			//buffer.Reset(bufferSize);
			buffer.Empty();
			buffer.AddUninitialized(bufferSize);

			reader->Serialize(buffer.GetData(), buffer.Num());
			lastPosition += buffer.Num();

			//socket->Send((uint8*)((ANSICHAR*)Convert.Get()), Convert.Length(), dataSendBySocket);
			socket->Send(buffer.GetData(), buffer.Num(), dataSendBySocket);

			//slowdown for tests
			//FPlatformProcess::Sleep(0.01f);

			//fire event every second
			//one second = 10000000 ticks
			if (((FDateTime::Now().GetTicks()) - lastTimeTicks) >= 10000000) {
				mbit = ((float)lastPosition - (float)bytesSentSinceLastTick) / 1024 / 1024 * 8;
				lastTimeTicks = FDateTime::Now().GetTicks();
				bytesSentSinceLastTick = lastPosition;

				percent = ((float)lastPosition / fileSize) * 100;
				triggerFileOverTCPProgress(sessionID, filePath, percent, mbit, lastPosition, fileSize);
			}

		}

		if (lastPosition == fileSize) {

			mbit = ((float)lastPosition - (float)bytesSentSinceLastTick) / 1024 / 1024 * 8;
			percent = ((float)lastPosition / fileSize) * 100;

			triggerFileOverTCPProgress(sessionID, filePath, percent, mbit, lastPosition, fileSize);
		}
		else {
			triggerFileTransferOverTCPInfoEvent("Error while sending the file.", sessionID, filePath, false);
		}

		buffer.Empty();
		if (reader != nullptr) {
			reader->Close();
			reader = nullptr;
		}

		run = false;
		thread = nullptr;

		return 0;
	}


	void triggerFileOverTCPProgress(FString sessionIDP, FString filePathP, float percentP, float mbitP, int64 bytesReceivedP, int64 fileSizeP) {
		AsyncTask(ENamedThreads::GameThread, [sessionIDP, filePathP, percentP, mbitP, bytesReceivedP, fileSizeP]() {
			USocketServerBPLibrary::socketServerBPLibrary->onfileTransferOverTCPProgressEventDelegate.Broadcast(sessionIDP, filePathP, percentP, mbitP, bytesReceivedP, fileSizeP);
			});
	}

	void triggerFileTransferOverTCPInfoEvent(FString messageP, FString sessionIDP, FString filePathP, bool successP) {
		AsyncTask(ENamedThreads::GameThread, [messageP, sessionIDP, filePathP, successP]() {
			USocketServerBPLibrary::socketServerBPLibrary->onfileTransferOverTCPInfoEventDelegate.Broadcast(messageP, sessionIDP, filePathP, successP);
			});
	}

	void stopThread() {
		run = false;
	}

	bool isRun() {
		return run;
	}


protected:
	FSocket* socket = nullptr;
	FString filePath = FString();
	int64 fileSize = 0;
	int64 startPosition = 0;
	FClientSocketSession session;
	FRunnableThread* thread		= nullptr;
	bool					run = true;
	
};


/*Receive Files from client. Thread*/
class SOCKETSERVER_API FTCPFileHandlerThread : public FRunnable {

public:

	FTCPFileHandlerThread(USocketServerPluginTCPServer* tcpServerP, FClientSocketSession sessionP) :
		tcpServer(tcpServerP),
		session(sessionP) {
		FString threadName = "FTCPFileHandlerThread" + FGuid::NewGuid().ToString();
		thread = FRunnableThread::Create(this, *threadName, 0, EThreadPriority::TPri_Normal);
	}

	virtual uint32 Run() override {
		FString serverID = tcpServer->getServerID();
		FSocket* clientSocket = session.socket;
		FString sessionID = session.sessionID;


		AsyncTask(ENamedThreads::GameThread, [sessionID, serverID]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, true, "Client connected", sessionID, serverID);
		});

		int64 ticksDownload = FDateTime::Now().GetTicks();
		int64 lastByte = 0;
		int64 bytesDownloaded = 0;
		FArchive* writer = nullptr;
		uint32 DataSize;
		FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));
		int64 ticks1;
		int64 ticks2;
		FString md5Client = FString();
		FString fullFilePath = FString();
		FString token = FString();
		TArray<uint8> byteArray;
		FString recvMessage;
		bool isConnected = true;
		while (run && clientSocket != nullptr && tcpServer->getClientSession(sessionID) != nullptr) {
			//ESocketConnectionState::SCS_Connected does not work https://issues.unrealengine.com/issue/UE-27542
			//Compare ticks is a workaround to get a disconnect. clientSocket->Wait() stop working after disconnect. (Another bug?)
			//If it doesn't wait any longer, ticks1 and ticks2 should be the same == disconnect.
			ticks1 = FDateTime::Now().GetTicks();
			clientSocket->Wait(ESocketWaitConditions::WaitForRead, FTimespan::FromSeconds(3));
			ticks2 = FDateTime::Now().GetTicks();
			bool hasData = clientSocket->HasPendingData(DataSize);
			if (!hasData) {
				if (ticks1 == ticks2) {
					break;//disconnect
				}
				else {
					if (sendFileThread == nullptr) {
						triggerFileTransferOverTCPInfoEvent("Timeout", sessionID, fullFilePath, false);
						break;//timeout
					}
				}
			}
			if (hasData) {
				int32 bytesRead = 0;

				switch (commandProgress)
				{
				case 0:
					Datagram->SetNumUninitialized(DataSize);
					if (clientSocket->Recv(Datagram->GetData(), Datagram->Num(), bytesRead)) {

						Datagram->Add(0x00);// null-terminator
						char* Data = (char*)Datagram->GetData();
						recvMessage = FString(UTF8_TO_TCHAR(Data));
						recvMessage = (tcpServer->decryptMessage(recvMessage)).TrimStartAndEnd();
						/*UE_LOG(LogTemp, Display, TEXT("TCP End xxx: %s: "), *recvMessage);*/
						TArray<FString> lines;
						recvMessage.ParseIntoArray(lines, TEXT("_|_"), true);


						if (lines.Num() == 5) {
							//client send data to this server
							if (lines[0].Equals("SEND_FILE_TO_SERVER") && lines[1].Len() > 0 && lines[2].Len() > 0 && lines[3].Len() > 0 && lines[4].Len() > 0) {
								
								md5Client = lines[2];

								token = lines[1];
								struct FSocketServerToken tokenStruct = tcpServer->getTokenStruct(token);

					

								if (tokenStruct.token.IsEmpty() && tokenStruct.token.Equals(token) == false) {
									triggerFileTransferOverTCPInfoEvent("Token not found.", sessionID, fullFilePath, false);
									run = false;
									break;
								}

								if (tokenStruct.deleteAfterUse) {
									tcpServer->removeTokenFromStruct(token);
								}

								FString downloadDir = tcpServer->getCleanDir(tokenStruct.directoryType, tokenStruct.fileDirectory);
								FString fileName = lines[3];
								if (downloadDir.EndsWith("/")) {
									fullFilePath = downloadDir + fileName;
								}
								else {
									fullFilePath = downloadDir + "/" + fileName;
								}


								if (md5Client.IsEmpty()) {
									triggerFileTransferOverTCPInfoEvent("MD5 string from client is missing.", sessionID, fullFilePath, false);
									run = false;
									break;
								}
								

						

							
								if (FPaths::DirectoryExists(downloadDir) == false) {
									triggerFileTransferOverTCPInfoEvent("Directory not found.", sessionID, fullFilePath, false);
									run = false;
									break;
								}


								fileSize = FCString::Atoi64(*lines[4]);


								if (fileSize <= 0) {
									triggerFileTransferOverTCPInfoEvent("Client has reported a file size of 0 or lower.", sessionID, fullFilePath, false);
									run = false;
									break;
								}
								
	/*							if (tcpServer->hasResume() && lines[4].Equals("0") == false) {
									if (FPaths::FileExists(fullFilePath)) {
										lastByte = FCString::Atoi64(*lines[4]);
									}
								}*/

								if (tcpServer->hasResume()) {
									writer = IFileManager::Get().CreateFileWriter(*fullFilePath, EFileWrite::FILEWRITE_Append);
								}
								else {
									writer = IFileManager::Get().CreateFileWriter(*fullFilePath);
								}
								
								
							
								

								if (!writer) {
									triggerFileTransferOverTCPInfoEvent("Can't create file.", sessionID, fullFilePath, false);
									run = false;
									break;
								}

								
								
								bytesDownloaded = writer->TotalSize();


								if (bytesDownloaded > fileSize) {
									triggerFileTransferOverTCPInfoEvent("File on server bigger than on client. Cancel.", sessionID, fullFilePath, false);
									run = false;
									break;
								}

								if (bytesDownloaded == fileSize) {
									writer->Close();
									FPlatformProcess::Sleep(1);
									sendEndMessage(fullFilePath, token, md5Client, sessionID, clientSocket);
									run = false;
									break;
								}
								else {
									commandProgress =1;
									FString response = "SEND_FILE_TO_SERVER_ACCEPTED_|_" + token + "_|_" + FString::FromInt(bytesDownloaded) + "\r\n";
									response = tcpServer->encryptMessage(response);

									FTCHARToUTF8 Convert(*response);
									int32 bytesSendOverSocket = 0;
									clientSocket->Send((uint8*)Convert.Get(), Convert.Length(), bytesSendOverSocket);

								}
							}
							else {
								triggerFileTransferOverTCPInfoEvent("File request incorrect (0).", sessionID, fullFilePath, false);

								recvMessage.Empty();
								Datagram->Empty();
								byteArray.Empty();

								run = false;
								break;
							}
						}
						else{
							if (lines.Num() == 2) {
								//client request file
								if (lines[0].Equals("REQUEST_FILE_FROM_SERVER") && lines[1].Len() > 0) {
									token = lines[1];
									struct FSocketServerToken tokenStruct = tcpServer->getTokenStruct(token);

									if (tokenStruct.token.IsEmpty() && tokenStruct.token.Equals(token) == false) {
										triggerFileTransferOverTCPInfoEvent("Token not found.", sessionID, fullFilePath, false);
										run = false;
										break;
									}

									fullFilePath = tcpServer->getCleanDir(tokenStruct.directoryType, tokenStruct.fileDirectory);

									if (FPaths::FileExists(fullFilePath) == false) {
										triggerFileTransferOverTCPInfoEvent("File not found,", sessionID, fullFilePath, false);
										run = false;
										break;
									}


									bool md5okay = false;
									FString md5Server = FString();
									tcpServer->getMD5FromFile(fullFilePath, md5okay, md5Server);

									FString response = "REQUEST_FILE_FROM_SERVER_ACCEPTED_|_" + token + "_|_" + md5Server + "_|_" + tcpServer->int64ToString(tcpServer->fileSize(fullFilePath)) + "_|_" + FPaths::GetCleanFilename(fullFilePath) +"\r\n";
									response = tcpServer->encryptMessage(response);

									FTCHARToUTF8 Convert(*response);
									int32 bytesSendOverSocket = 0;
									clientSocket->Send((uint8*)Convert.Get(), Convert.Length(), bytesSendOverSocket);

								
									
								}
								else {
	
									triggerFileTransferOverTCPInfoEvent("File request incorrect (1).", sessionID, fullFilePath, false);

									recvMessage.Empty();
									Datagram->Empty();
									byteArray.Empty();

									run = false;
									break;
								}
							}
							else {

								if (lines.Num() == 3) {
									//server send data to client
									if (lines[0].Equals("REQUEST_FILE_FROM_SERVER_ACCEPTED") && lines[1].Equals(token) && lines[2].Len() > 0) {
										int64 startPosition = FCString::Atoi64(*lines[2]);
							
										sendFileThread = new FTCPClientSendFileToClientThread(clientSocket, fullFilePath,fileSize, startPosition, session);
										//FPlatformProcess::Sleep(3);

										//if (!sendFile(fullFilePath, startPosition, clientSocket, sessionID)) {
										//	triggerFileTransferOverTCPInfoEvent("Error while sending the file.", sessionID, fullFilePath, false);

										//	recvMessage.Empty();
										//	Datagram->Empty();
										//	byteArray.Empty();

										//	run = false;
										//	break;
										//}
									}
									else {

										if (lines[0].Equals("REQUEST_FILE_FROM_SERVER_END") && lines[1].Equals(token) && lines[2].Equals("OKAY")) {
											triggerFileTransferOverTCPInfoEvent("File transfer successful.", sessionID, fullFilePath, true);
											run = false;
											break;
																					}
										else {

											triggerFileTransferOverTCPInfoEvent("File transfer failed.", sessionID, fullFilePath, false);

											recvMessage.Empty();
											Datagram->Empty();
											byteArray.Empty();

											run = false;
											break;
										}
									}
								}
								else {

									triggerFileTransferOverTCPInfoEvent("File request incorrect (3).", sessionID, fullFilePath, false);

									recvMessage.Empty();
									Datagram->Empty();
									byteArray.Empty();

									run = false;
									break;
								}
							}
						}
					}
					break;
				case 1:
					//downlnoad
					bytesRead = 0;
					Datagram->Empty();
					Datagram->SetNumUninitialized(DataSize);
					if (clientSocket->Recv(Datagram->GetData(), Datagram->Num(), bytesRead)) {
						writer->Serialize(const_cast<uint8*>(Datagram->GetData()), Datagram->Num());

						//show progress each second
						if ((ticksDownload + 10000000) <= FDateTime::Now().GetTicks()) {
							writer->Flush();
							int64 bytesSendLastSecond = bytesDownloaded - lastByte;
							//float speed = ((float)bytesSendLastSecond) / 125000;
							float mbit = ((float)bytesSendLastSecond) / 1024 / 1024 * 8;
							float sent = ((float)bytesDownloaded) / 1048576;
							float left = 0;
							float percent = 0;
							if (fileSize > 0) {
								left = ((float)(fileSize - bytesDownloaded)) / 1048576;
								percent = ((float)bytesDownloaded / (float)fileSize * 100);
							}


							triggerFileOverTCPProgress(sessionID, fullFilePath, percent, mbit, lastByte, fileSize);

							ticksDownload = FDateTime::Now().GetTicks();
							lastByte = bytesDownloaded;
						}
						bytesDownloaded += bytesRead;
					}
					if (bytesDownloaded >= fileSize) {
						//receive file finish
						triggerFileOverTCPProgress(sessionID, fullFilePath, 100, 0, bytesDownloaded, fileSize);

						if (writer != nullptr) {
							writer->Close();
						}

						FPlatformProcess::Sleep(1);
						sendEndMessage(fullFilePath, token, md5Client, sessionID, clientSocket);

						run = false;
					}

				break;

				}
			}
		}

		Datagram->Empty();


		run = false;


		if (sendFileThread != nullptr) {
			sendFileThread->stopThread();
			sendFileThread = nullptr;
		}

		//UE_LOG(LogTemp, Display, TEXT("TCP Connected: %s:%i"), *session.ip, session.port);


		if (thread && thread != nullptr) {
			thread = nullptr;
		}

		if (writer != nullptr) {
			writer->Close();
			delete writer;
		}

		FPlatformProcess::Sleep(3);
		if (clientSocket != nullptr) {
			clientSocket->Close();
		
			ISocketSubsystem* sSS = USocketServerBPLibrary::socketServerBPLibrary->getSocketSubSystem();
			if (sSS != nullptr) {
				sSS->DestroySocket(clientSocket);
				clientSocket = nullptr;
			}
		}

		USocketServerPluginTCPServer* tcpServerGlobal = tcpServer;
		AsyncTask(ENamedThreads::GameThread, [sessionID, serverID, tcpServerGlobal]() {
			USocketServerBPLibrary::socketServerBPLibrary->onsocketServerConnectionEventDelegate.Broadcast(EServerSocketConnectionEventType::E_Client, false, "Client disconnected", sessionID, serverID);
			//clean up socket in main thread because race condition
			tcpServerGlobal->removeClientSession(sessionID);
		});

		return 0;
	}

	void triggerFileOverTCPProgress(FString sessionIDP, FString filePathP, float percentP, float mbitP, int64 bytesReceivedP, int64 fileSizeP) {
		AsyncTask(ENamedThreads::GameThread, [sessionIDP, filePathP, percentP, mbitP, bytesReceivedP, fileSizeP]() {
			USocketServerBPLibrary::socketServerBPLibrary->onfileTransferOverTCPProgressEventDelegate.Broadcast(sessionIDP, filePathP, percentP, mbitP, bytesReceivedP, fileSizeP);
		});
	}

	void triggerFileTransferOverTCPInfoEvent(FString messageP, FString sessionIDP, FString filePathP, bool successP) {
		AsyncTask(ENamedThreads::GameThread, [messageP, sessionIDP, filePathP, successP]() {
			USocketServerBPLibrary::socketServerBPLibrary->onfileTransferOverTCPInfoEventDelegate.Broadcast(messageP, sessionIDP, filePathP, successP);
		});
	}

	void sendEndMessage(FString fullFilePathP, FString tokenP, FString md5ClientP, FString sessionIDP, FSocket* clientSocketP) {
		bool md5okay = false;
		FString md5Server = FString();
		tcpServer->getMD5FromFile(fullFilePathP, md5okay, md5Server);

		FString response = "SEND_FILE_TO_SERVER_END_|_" + tokenP + "_|_";

		if (md5okay && md5ClientP.Equals(md5Server)) {
			triggerFileTransferOverTCPInfoEvent("File successfully received.", sessionIDP, fullFilePathP, true);

			response += "OKAY\r\n";
		}
		else {
			triggerFileTransferOverTCPInfoEvent("File received but MD5 does not match. Corrupted file will be deleted if resume is not disabled.", sessionIDP, fullFilePathP, false);
			response += "MD5ERROR\r\n";
			if (tcpServer->hasResume() == false) {
				tcpServer->deleteFile(fullFilePathP);
			}
		}

		response = tcpServer->encryptMessage(response);

		FTCHARToUTF8 Convert(*response);
		int32 bytesSendOverSocket = 0;
		clientSocketP->Send((uint8*)Convert.Get(), Convert.Length(), bytesSendOverSocket);
	}


	void stopThread() {
		run = false;
	}


protected:
	USocketServerPluginTCPServer* tcpServer = nullptr;
	FClientSocketSession session;
	bool fileServer = false;
	FRunnableThread* thread = nullptr;
	bool run = true;
	int32 commandProgress = 0;
	int64 fileSize;
	FTCPClientSendFileToClientThread* sendFileThread = nullptr;
};



