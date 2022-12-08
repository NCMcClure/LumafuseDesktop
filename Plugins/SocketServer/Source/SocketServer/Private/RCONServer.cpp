// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#include "RCONServer.h"

URCONServer::URCONServer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
}

void URCONServer::startRCONServer(FString serverID, ERCONPasswordType passwordTypeP, FString passwordOrFileP
	, bool& success, FString& errorMessage) {

	success = false;
	errorMessage = "";

	if (serverID.IsEmpty() || USocketServerBPLibrary::getSocketServerTarget()->getTcpServerMap().Find(serverID) == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): ServerID not found: %s"), *serverID);
		success = false;
		errorMessage = "ServerID not found: "+serverID;
		return;
	}
	passwordType = passwordTypeP;

	if (passwordType != ERCONPasswordType::E_parameter) {

		EFileFunctionsSocketServerDirectoryType passwordDirectoryType = EFileFunctionsSocketServerDirectoryType::E_gd;
		if (passwordType == ERCONPasswordType::E_ad) {
			passwordDirectoryType = EFileFunctionsSocketServerDirectoryType::E_ad;
		}

		passwordOrFile = UFileFunctionsSocketServer::getCleanDirectory(passwordDirectoryType, passwordOrFileP);
		if (FPaths::DirectoryExists(FPaths::GetPath(passwordOrFile)) == false) {
			UFileFunctionsSocketServer::createDirectory(passwordDirectoryType, FPaths::GetPath(passwordOrFile));
		}


		if (FPaths::FileExists(passwordOrFile) == false) {
			UFileFunctionsSocketServer::writeStringToFile(passwordDirectoryType, "", passwordOrFile, EFileFunctionsSocketServerEncodingOptions::E_AutoDetect, success);
		}

		if (FPaths::FileExists(passwordOrFile) == false) {
			UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): Can't read the password file: %s"), *passwordOrFile);
			success = false;
			errorMessage = "Can't read the password file: " + passwordOrFile;
			return;
		}
	}
	else {
		passwordOrFile = passwordOrFileP;
	}

	success = true;
}


void URCONServer::receiveTCPMessageEvent(const FString sessionID, const FString message, const TArray<uint8>& byteArray, const FString serverID) {

	USocketServerPluginTCPServer* tcpServer = *USocketServerBPLibrary::getSocketServerTarget()->getTcpServerMap().Find(serverID);
	if (tcpServer == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): Server not found: %s"), *serverID);
		return;
	}
	
	//https://developer.valvesoftware.com/wiki/Source_RCON_Protocol

	//packet to small
	if (byteArray.Num() < 14) {
		tcpServer->removeClientSession(sessionID);
		return;
	}

	//packet to big
	if (byteArray.Num() > 4096) {
		tcpServer->removeClientSession(sessionID);
		return;
	}

	int32 size = 0;
	FMemory::Memcpy(&size, byteArray.GetData(), 4);

	//packet to small
	if (size < 10) {
		tcpServer->removeClientSession(sessionID);
		return;
	}

	int32 id = 0;
	FMemory::Memcpy(&id, byteArray.GetData() + 4, 4);

	int32 type = 0;
	FMemory::Memcpy(&type, byteArray.GetData() + 8, 4);

	TArray<uint8> bodyTextArray;
	bodyTextArray.AddUninitialized(byteArray.Num()-12);
	FMemory::Memcpy(bodyTextArray.GetData(), byteArray.GetData() + 12, byteArray.Num() - 12);

	FString bodyText = FString(UTF8_TO_TCHAR(bodyTextArray.GetData()));
	bodyTextArray.Empty();

	UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): size:%i id:%i type:%i body:%s"),size, id,type,*bodyText);

	

	switch (type)
	{
	case 0:
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): Server response sent to the server? Request will be ignored."));
		break;
	case 2:
		USocketServerBPLibrary::getSocketServerTarget()->onreceiveRCONRequestEventDelegate.Broadcast(sessionID,serverID,id, bodyText);
		break;
	case 3:
		authResponse(sessionID, serverID, bodyText, id);
		break;
	default:
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): Unsupported request. Connection is closed."));
		tcpServer->removeClientSession(sessionID);
		return;
	}


}



void URCONServer::authResponse(FString sessionID, FString serverID, FString password, int32 rconID){

	//check password
	if (password.IsEmpty() || passwordOrFile.IsEmpty()) {
		rconID = -1;
		sendResponse(sessionID, serverID, -1, 2, FString());
		return;
	}

	bool found = false;

	if (passwordType == ERCONPasswordType::E_parameter) {
		if (password.Equals(passwordOrFile)) {
			found = true;
		}
	}
	else {
		FString passwordData = FString();
		FFileHelper::LoadFileToString(passwordData, *passwordOrFile);

		TArray<FString> passwords;
		passwordData.ParseIntoArray(passwords, TEXT("\n"), true);

		for (int32 i = 0; i < passwords.Num(); i++)
		{
			if (passwords[i].TrimEnd().Equals(password)) {
				found = true;
				break;
			}
		}
	}



	if (found) {
		sendResponse(sessionID, serverID, rconID, 2, FString());
	}
	else {
		sendResponse(sessionID, serverID, -1, 2, FString());
		USocketServerPluginTCPServer* tcpServer = *USocketServerBPLibrary::getSocketServerTarget()->getTcpServerMap().Find(serverID);
		if (tcpServer != nullptr) {
			tcpServer->removeClientSession(sessionID);
			UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): Wrong password. Connection is closed."));
		}
	}

}

bool URCONServer::sendResponse(FString sessionID, FString serverID, int32 id, int32 type, FString body){
	USocketServerPluginTCPServer* tcpServer = *USocketServerBPLibrary::getSocketServerTarget()->getTcpServerMap().Find(serverID);
	if (tcpServer == nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): Server not found. Can't send response: %s"), *serverID);
		return false;
	}



	FTCHARToUTF8 Convert(*body);

	//4 (id) + 4 (type) + 1 (minimum body) + 1 (null terminator) = 10
	int32 size = 10;

	TArray<uint8> response;
	size += Convert.Length();

	response.AddZeroed(size + 4); // 4 =  the size itself as int32

	FMemory::Memcpy(response.GetData(), &size, 4);
	FMemory::Memcpy(response.GetData() + 4, &id, 4);
	FMemory::Memcpy(response.GetData() + 8, &type, 4);
	if (Convert.Length() > 0) {
		FMemory::Memcpy(response.GetData() + 12, Convert.Get(), Convert.Length());
	}

	//packet to big
	if (response.Num() > 4096) {
		UE_LOG(LogTemp, Warning, TEXT("SimpleSocketServer Plugin (RCON): Answer too big. A data packet may have a maximum size of 4096 bytes. Size is %i"), response.Num());
		return false;
	}

	tcpServer->sendTCPMessageToClient(sessionID, FString(), response, false);

	return true;
}
