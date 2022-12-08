// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#include "SocketServerPluginTCPServer.h"


USocketServerPluginTCPServer::USocketServerPluginTCPServer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	this->AddToRoot();
}


void USocketServerPluginTCPServer::startTCPServer(IpAndPortStruct ipStructP,FString IPPP, int32 portP, EReceiveFilterServer receiveFilterP, FString serverIDP, bool isFileServer, FString Aes256bitKeyP, bool resumeFilesP) {
	ipAndPortStruct = ipStructP;
	serverPort = portP;
	receiveFilter = receiveFilterP;
	serverID = serverIDP;
	fileServer = isFileServer;
	aesKey = Aes256bitKeyP;
	resumeFiles = resumeFilesP;
	serverThread = new FServerTCPThread(this, receiveFilter);
}


void USocketServerPluginTCPServer::stopTCPServer() {

	run = false;

	TArray<FString> toRemoveSessionKeys;
	for (auto& element : getClientSessions()) {
		toRemoveSessionKeys.Add(element.Key);
	}
	for (int32 i = 0; i < toRemoveSessionKeys.Num(); i++) {
		removeClientSession(toRemoveSessionKeys[i]);
	}
	toRemoveSessionKeys.Empty();

	if (serverThread != nullptr) {
		serverThread->stopThread();
		serverThread = nullptr;
	}

}

void USocketServerPluginTCPServer::sendTCPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak){
	for (auto& sessionID : clientSessionIDs) {
		FClientSocketSession* sessionPointer = clientSessions.Find(sessionID);
		if (sessionPointer != nullptr) {
			FClientSocketSession session = *sessionPointer;
			if (session.protocol == EServerSocketConnectionProtocol::E_TCP) {

				if (session.sendThread == nullptr) {
					UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: The thread for sending data has not yet been initialized. Data is not sent."));
				}
				else {
					session.sendThread->sendMessage(message, byteArray);
				}
			}
		}
	}
}

void USocketServerPluginTCPServer::sendTCPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak) {
	FClientSocketSession* sessionPointer = clientSessions.Find(clientSessionID);
	if (sessionPointer != nullptr) {
		FClientSocketSession session = *sessionPointer;
		if (session.protocol == EServerSocketConnectionProtocol::E_TCP) {

			if (session.sendThread == nullptr) {
				UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: The thread for sending data has not yet been initialized. Data is not sent."));
			}
			else {
				session.sendThread->sendMessage(message, byteArray);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin: Session not found: %s"),*clientSessionID);
	}
}


IpAndPortStruct USocketServerPluginTCPServer::getServerIpAndPortStruct() {
	return ipAndPortStruct;
}

FString USocketServerPluginTCPServer::getIP() {
	return serverIP;
}

int32 USocketServerPluginTCPServer::getPort() {
	return serverPort;
}

FString USocketServerPluginTCPServer::getServerID() {
	return serverID;
}

bool USocketServerPluginTCPServer::hasResume() {
	return resumeFiles;
}


void USocketServerPluginTCPServer::initTCPClientThreads(FClientSocketSession sessionP, EReceiveFilterServer receiveFilterP){
	if (fileServer) {
		TCPFileHandlerThread = new FTCPFileHandlerThread(this, sessionP);
	}
	else {
		sessionP.recieverThread = new FTCPClientReceiveDataFromServerThread(this, sessionP, receiveFilterP);
		sessionP.sendThread = new FTCPClientSendDataToServerThread(this, sessionP);
	}
	
	addClientSession(sessionP);
}

void USocketServerPluginTCPServer::addClientSession(FClientSocketSession session) {
	if (session.sessionID.IsEmpty() == false) {
		//UE_LOG(LogTemp, Warning, TEXT("ADD Session:%s"), *session.sessionID);
		clientSessions.Add(session.sessionID, session);
	}
}

FClientSocketSession* USocketServerPluginTCPServer::getClientSession(FString key) {
	return clientSessions.Find(key);
}

void USocketServerPluginTCPServer::removeClientSession(FString key) {
	//close client socket
	if (clientSessions.Find(key) != nullptr) {
		FClientSocketSession session = *clientSessions.Find(key);

		if (session.recieverThread != nullptr) {
			session.recieverThread->stopThread();
			session.recieverThread = nullptr;
		}

		if (session.sendThread != nullptr) {
			session.sendThread->stopThread();
			session.sendThread = nullptr;
		}

	}
	if (clientSessions.Remove(key)) {
		//UE_LOG(LogTemp, Warning, TEXT("Remove Session:%s"), *key);
		USocketServerBPLibrary::socketServerBPLibrary->unregisterClientEvent(key);
	}

	if (TCPFileHandlerThread != nullptr) {
		TCPFileHandlerThread->stopThread();
		TCPFileHandlerThread = nullptr;
	}
}

TMap<FString, FClientSocketSession> USocketServerPluginTCPServer::getClientSessions(){
	return clientSessions;
}

//EHTTPSocketServerFileDownloadResumeType USocketServerPluginTCPServer::getifFileExistThen(){
//	return ifFileExistThen;
//}



FString USocketServerPluginTCPServer::encryptMessage(FString message) {
	return UFileFunctionsSocketServer::encryptMessageWithAES(message, aesKey);
}

FString USocketServerPluginTCPServer::decryptMessage(FString message) {
	return UFileFunctionsSocketServer::decryptMessageWithAES(message, aesKey);
}

struct FSocketServerToken USocketServerPluginTCPServer::getTokenStruct(FString token){
	FSocketServerToken sst;
	if (USocketServerBPLibrary::socketServerBPLibrary->fileTokenMap.Find(token) != nullptr) {
		sst = *USocketServerBPLibrary::socketServerBPLibrary->fileTokenMap.Find(token);
	}
	return sst;
}

void USocketServerPluginTCPServer::removeTokenFromStruct(FString token) {
	if (USocketServerBPLibrary::socketServerBPLibrary->fileTokenMap.Find(token) != nullptr) {
		USocketServerBPLibrary::socketServerBPLibrary->fileTokenMap.Remove(token);
	}
}

FString USocketServerPluginTCPServer::getCleanDir(EFileFunctionsSocketServerDirectoryType directoryType, FString fileDirectory) {
	return UFileFunctionsSocketServer::getCleanDirectory(directoryType, fileDirectory);
}

void USocketServerPluginTCPServer::getMD5FromFile(FString filePathP, bool& success, FString& MD5) {
	UFileFunctionsSocketServer::getMD5FromFileAbsolutePath(filePathP, success, MD5);
}

void USocketServerPluginTCPServer::deleteFile(FString filePathP) {
	UFileFunctionsSocketServer::deleteFileAbsolutePath(filePathP);
}

bool USocketServerPluginTCPServer::isRun(){
	return run;
}

int64 USocketServerPluginTCPServer::fileSize(FString filePathP) {
	return UFileFunctionsSocketServer::fileSizeAbsolutePath(filePathP);
}

FString USocketServerPluginTCPServer::int64ToString(int64 num) {
	return UFileFunctionsSocketServer::int64ToString(num);
}

//FString USocketServerPluginTCPServer::int64ToString(int64 num) {
//	return UFileFunctionsSocketServer::int64ToString(num);
//}
