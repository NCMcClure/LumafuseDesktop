// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#pragma once

#include "SocketServer.h"
#include "SocketServerBPLibrary.generated.h"

class USocketServerPluginUDPServer;
class USocketServerPluginTCPServer;
class URCONServer;


UCLASS()
class SOCKETSERVER_API USocketServerBPLibrary : public UObject
{
	GENERATED_UCLASS_BODY()


public:

	static USocketServerBPLibrary *socketServerBPLibrary;
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketServer")
		static USocketServerBPLibrary* getSocketServerTarget();

	//Delegates
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FsocketServerConnectionEventDelegate, EServerSocketConnectionEventType, type, bool, success, FString, message, FString, sessionID, FString, serverID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FserverReceiveTCPMessageEventDelegate, FString, sessionID, FString, message, const TArray<uint8>&, byteArray, FString, serverID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FreadBytesFromFileInPartsEventDelegate, int64, fileSize, int64, position, bool, end, const TArray<uint8>&, byteArray);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FsocketServerUDPConnectionEventDelegate, bool, success, FString, message, FString, serverID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FserverReceiveUDPMessageEventDelegate, FString, sessionID, FString, message, const TArray<uint8>&, byteArray,FString, serverID);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FfileTransferOverTCPProgressEventDelegate, FString, sessionID, FString, filePath, float, percent, float, mbit, int64, bytesTransferred, int64, fileSize);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FfileTransferOverTCPInfoEventDelegate, FString, message, FString, sessionID, FString, filePath, bool, success);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FreceiveRCONRequestEventDelegate, FString, sessionID, FString, serverID, int32, requestID, FString, request);


	UFUNCTION()
		void socketServerConnectionEventDelegate(const EServerSocketConnectionEventType type, const bool success, const FString message, const FString sessionID, const FString serverID);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|TCP|Events|ConnectionInfo")
		FsocketServerConnectionEventDelegate onsocketServerConnectionEventDelegate;
	UFUNCTION()
		void serverReceiveTCPMessageEventDelegate(const FString sessionID, const FString message, const TArray<uint8>& byteArray, const FString serverID);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|TCP|Events|ReceiveMessage")
		FserverReceiveTCPMessageEventDelegate onserverReceiveTCPMessageEventDelegate;
	UFUNCTION()
		void socketServerUDPConnectionEventDelegate(const bool success, const FString message, const FString serverID);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|UDP|Events|ConnectionInfo")
		FsocketServerUDPConnectionEventDelegate onsocketServerUDPConnectionEventDelegate;
	UFUNCTION()
		void serverReceiveUDPMessageEventDelegate(const FString sessionID, const FString message, const TArray<uint8>& byteArray, const FString serverID);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|UDP|Events|ReceiveMessage")
		FserverReceiveUDPMessageEventDelegate onserverReceiveUDPMessageEventDelegate;
	UFUNCTION()
		void fileTransferOverTCPProgressEventDelegate(const FString sessionID, const FString filePath, const float percent, const float mbit, const int64 bytesTransferred, const int64 fileSize);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|TCP|Events|File|FileTransferOverTCPProgress")
		FfileTransferOverTCPProgressEventDelegate onfileTransferOverTCPProgressEventDelegate;
	UFUNCTION()
		void fileTransferOverTCPInfoEventDelegate(const FString message, const FString sessionID, const FString filePath, const bool success);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|TCP|Events|File|FileTransferOverTCPInfo")
		FfileTransferOverTCPInfoEventDelegate onfileTransferOverTCPInfoEventDelegate;
	UFUNCTION()
		void readBytesFromFileInPartsEventDelegate(const int64 fileSize, const int64 position, const bool end, const TArray<uint8>& byteArray);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|SpecialFunctions|File|Events|ReadBytesFromFileInPartsAsync")
		FreadBytesFromFileInPartsEventDelegate onreadBytesFromFileInPartsEventDelegate;
	UFUNCTION()
		void receiveRCONRequestEventDelegate(const FString sessionID, const FString serverID, const int32 requestID, const FString request);
	UPROPERTY(BlueprintAssignable, Category = "SocketServer|TCP|Events|RCON|ReceiveRCONRequest")
		FreceiveRCONRequestEventDelegate onreceiveRCONRequestEventDelegate;
	/**
	*Get all Session IDs
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		void serverPluginGetSocketSessionIds(const FString optionalServerID, TArray<FString>& sessionIDs);
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		void serverPluginGetSocketSessionInfo(const FString sessionID, bool &sessionFound, FString &IP, int32 &port, EServerSocketConnectionProtocol &connectionProtocol, FString& serverID);
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		void serverPluginGetSocketSessionInfoByServerID(const FString serverID, const FString sessionID, bool& sessionFound, FString& IP, int32& port, EServerSocketConnectionProtocol& connectionProtocol);
	/**
	*Close a connection and remove the session
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		void removeSessionAndCloseConnection(FString sessionId, FString optionalServerID);

	struct IpAndPortStruct checkIpAndPort(FString IP, int32 port);


	//UDP
	/**
	*Start UDP Server
	*@param domainOrIP IP or Domain to listen
	*@param port port to listen
	*@param multicast This allows several servers to be started on different computers in the LAN with the same IP.
	*@param receiveFilter This allows you to decide which data type you want to receive. If you receive files it makes no sense to convert them into a string.
	*@param customServerID Optionally you can assign your own ServerID like "myAuthentificationServer" or "fileServer"
	*@param maxPacketSize sets the maximum UDP packet size. More than 65507 is not possible.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|UDP", meta = (AdvancedDisplay = 7))
		void startUDPServer(FString& serverID, FString IP = FString("0.0.0.0"), int32 port = 8888, bool multicast = false, EReceiveFilterServer receiveFilter = EReceiveFilterServer::E_SAB, FString customServerID = FString(""), int32 maxPacketSize = 65507);

	/**
	*Stop UDP Server
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|UDP")
		void stopUDPServer(FString optionalServerID);
	UFUNCTION(BlueprintCallable, Category = "SocketServer|UDP")
		void stopAllUDPServers();

	/**
	*Sends data back to a client.
	*@param clientSessionIDs
	*@param message
	*@param byteArray
	*@param addLineBreak
	*@param socketType Some Thirdparty software expects the data to be sent back over the same socket, others expects a new socket.
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|UDP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketServerSendUDPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, ESocketServerUDPSocketType socketType, FString optionalServerID);

	/**
	*Sends data back to a client.
	*@param clientSessionIDs
	*@param message
	*@param byteArray
	*@param addLineBreak
	*@param socketType Some Thirdparty software expects the data to be sent back over the same socket, others expects a new socket.
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|UDP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketServerSendUDPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, ESocketServerUDPSocketType socketType, FString optionalServerID);

	/**
	*If you want to send data directly to a specific destination without getting data back.
	*@param ip
	*@param port
	*@param message
	*@param byteArray
	*@param addLineBreak
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|UDP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketServerSendUDPMessageTo(FString ip, int32 port, FString message, TArray<uint8> byteArray, bool addLineBreak, bool asynchronous, FString optionalServerID);

	

	//TCP

	void startTCPServerInternal(FString& serverID, FString IP, int32 port, EReceiveFilterServer receiveFilter, FString customServerID, bool isFileServer, FString Aes256bitKey, bool resumeFiles);

	/**
	* Start TCP Server
	* @param domainOrIP IP or Domain to listen
	* @param port port to listen
	* @param receiveFilter This allows you to decide which data type you want to receive. If you receive files it makes no sense to convert them into a string.
	* @param customServerID Optionally you can assign your own ServerID like "myAuthentificationServer" or "fileServer"
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		void startTCPServer(FString& serverID, FString IP = FString("0.0.0.0"), int32 port = 8888, EReceiveFilterServer receiveFilter = EReceiveFilterServer::E_SAB, FString customServerID = FString(""));


	/**
	* Starts a file server that can receive and send files in response to requests from clients. The files are streamed and do not consume RAM. To determine what can be uploaded or downloaded and how, the tokens are used.
	* @param domainOrIP IP or Domain to listen
	* @param port port to listen
	* @param receiveFilter This allows you to decide which data type you want to receive. If you receive files it makes no sense to convert them into a string.
	* @param customServerID Optionally you can assign your own ServerID like "myAuthentificationServer" or "fileServer"
	* @param Aes256bitKey The AES key must consist of 32 ASCII characters. The communication between client and server is encrypted via AES in 256bit. Therefore a key must be entered.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		void startTCPFileServer(FString& serverID, FString IP = FString("0.0.0.0"), int32 port = 8899, FString customServerID = FString(""), FString Aes256bitKey = FString(""), bool resumeFiles = false);

	/**
	* Tokens are used to determine what can be uploaded or downloaded and how. It can be used to specify a directory in which a file is stored or to specify a file that can be sent to a client.
	* @param deleteTokenAfterUse If true, the token will be deleted after one use.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		static void addFileToken(FString token, bool deleteTokenAfterUse, EFileFunctionsSocketServerDirectoryType directoryType, FString filePathOrDirectory);

	/**
	* Tokens are used to determine what can be uploaded or downloaded and how. It can be used to specify a directory in which a file is stored or to specify a file that can be sent to a client.
	* @param fileTokens Key= token, value = file path or directory
	* @param deleteTokenAfterUse If true, the token will be deleted after one use.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		static void addFileTokens(TMap<FString, FString> fileTokens, bool deleteAfterUse, EFileFunctionsSocketServerDirectoryType directoryType);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		static void removeFileToken(FString token);
	

	/**
	*Stop TCP Server
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		void stopTCPServer(FString optionalServerID);
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		void stopAllTCPServers();

	//UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP", meta = (AutoCreateRefTerm = "userIdsToDirectoriesMap"))
	//	void startTCPFileserver(FString& serverID, TMap<FString, FString> userIdsToDirectoriesMap, FString IP = FString("0.0.0.0"), int32 port = 9999, FString downloadDirectory = FString("Content/"), EFileFunctionsSocketServerDirectoryType directoryType = EFileFunctionsSocketServerDirectoryType::E_gd, EHTTPSocketServerFileDownloadResumeType ifFileExistThen = EHTTPSocketServerFileDownloadResumeType::E_RESUME, FString customServerID = FString(""), bool onlyWithToken = true);
	//UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
	//	void addFileserverToken(FString token, int32 lifeTimeSeconds,bool reusable);
	//UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
	//	void removeFileserverToken(FString token);

	/**
	*Send data to clients
	*@param clientSessionIDs array with client sessionIDs
	*@param message data as string
	*@param byteArray data as bytes
	*@param addLineBreak add linebreak to message
	*@param serverID Id of the server you want to send data from
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketServerSendTCPMessage(TArray<FString> clientSessionIDs, FString message, TArray<uint8> byteArray, bool addLineBreak = true, FString optionalServerID = FString(""));

	/**
	*Send data to client
	*@param clientSessionID array with client sessionIDs
	*@param message data as string
	*@param byteArray data as bytes
	*@param addLineBreak add linebreak to message
	*@param serverID Id of the server you want to send data from
	*@param optionalServerID With one server the field can remain empty. If there are several servers, the ServerID should be entered here or the newest server is automatically taken.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketServerSendTCPMessageToClient(FString clientSessionID, FString message, TArray<uint8> byteArray, bool addLineBreak = true, FString optionalServerID = FString(""));

	/**
	* Use before a connection has been established and do not use this node if byte arrays are used! It can happen that large messages are split at the receiver. Or that messages sent very quickly are combined into one. With this node the problem can be solved. If message wrapping is activated, a string is inserted before the message and antoher after the message. On receipt, the message must also contain both strings and is split or combined accordingly.
	*@param header This string is appended before the message.
	*@param footer This string is appended behind the message. If line break is selected, it will be placed after the footer.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		void activateTCPMessageWrappingOnServerPlugin(FString header = "([{UE4-Head}])", FString footer = "([{UE4-Foot}])");

	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		void activateTCPMessageAndBytesWrappingOnServerPlugin();

	/**
	* Use before a connection has been established!
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP")
		void deactivateTCPWrappingOnServerPlugin();

	/**
	*Send file to client
	*@param clientSessionID  client sessionID
	*@param directoryWithFileName 
	*@param directoryType 
	*@param serverID Id of the server you want to send file from
	*/
	/*UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP", meta = (AutoCreateRefTerm = "byteArray"))
		void socketServerSendTCPFile(FString clientSessionID, FString directoryWithFileName = FString("Content/image.png"), EFileFunctionsSocketServerDirectoryType directoryType = EFileFunctionsSocketServerDirectoryType::E_gd, int64 resumeFileSize =0,FString serverID = FString(""));*/

	/**
	*Creates a unique ID
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		static FString generateUniqueID();

	/**
	*Resolve Domain. Only Domains. Hostnames do not work.
	*@param domain
	*@param useDNSCache Domain and IP are stored in RAM. Starting from the second time the IP is taken from the RAM.
	*@param dnsIP
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		UDNSClientSocketServer* resolveDomain(FString domain, bool useDNSCache = true, FString dnsIP = FString("8.8.8.8"));


	/**
	*UE4 uses different socket connections. When Steam is active, Steam Sockets are used for all connections. This leads to problems if you want to use Steam but not Steam Sockets. Therefore you can change the sockets.
	*@param platform Auto =  UE4 decides. Thirdparty platforms (Steam, Playstore etc.) will be used if configured. System = UE4 determines the OS and selects a socket type accordingly.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		static void changeSocketPlatform(ESocketPlatformServer platform);


	/**
	* Checks if a server can listen on IP x and port y.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		bool checkPort(EServerSocketConnectionCheckPortType type, FString ip = FString("0.0.0.0"), int32 port = 8888);

	/**
	* Returns a random port on which the server can listen.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer")
		int32 getRandomPort(EServerSocketConnectionCheckPortType type, FString ip = FString("0.0.0.0"));

	/**
	* 
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|Register")
		void registerClientEvent(FString sessionID, UEventBean*& event);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|Register")
		void unregisterClientEvent(FString sessionID);

	UEventBean* getResiteredClientEvent(FString sessionID);

	//RCON

	/**
	* Turns a TCP server into an RCON server. 
	* @param passwordType If the type "As String Parameter" has been selected, the password can simply be entered as a parameter. Otherwise please select a directory incl. file.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP|RCON")
		static void registerRCONServer(FString serverID, ERCONPasswordType passwordType, FString passwordOrFile, bool& success, FString& errorMessage);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP|RCON")
		static void unregiserRCONServer(FString serverID);

	/**
	* Sends an RCON response to an RCON client. sessionID, serverID and requestID must be specified.
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|TCP|RCON")
		static void sendRCONResponse(FString sessionID, FString serverID, int32 requestID, FString response);


	//number stuff
	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToFloat(TArray<uint8> bytes, float& value);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToInteger(TArray<uint8> bytes, int32& value);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToInteger64(TArray<uint8> bytes, int64& value);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToFloatPure(TArray<uint8> bytes, float& value);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToIntegerPure(TArray<uint8> bytes, int32& value);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToInteger64Pure(TArray<uint8> bytes, int64& value);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToFloatEndian(TArray<uint8> bytes, float& littleEndian, float& bigEndian);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToIntegerEndian(TArray<uint8> bytes, int32& littleEndian, int32& bigEndian);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number")
		static void parseBytesToInteger64Endian(TArray<uint8> bytes, int64& littleEndian, int64& bigEndian);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number", meta = (AutoCreateRefTerm = "byteArray"))
		static void parseFloatToBytes(TArray<uint8>& byteArray, float value, bool switchByteOrder = false);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number", meta = (AutoCreateRefTerm = "byteArray"))
		static void parseIntegerToBytes(TArray<uint8>& byteArray, int32 value, bool switchByteOrder = false);

	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Number", meta = (AutoCreateRefTerm = "byteArray"))
		static void parseInteger64ToBytes(TArray<uint8>& byteArray, int64 value, bool switchByteOrder = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketServer|SpecialFunctions|Number", meta = (AutoCreateRefTerm = "byteArray"))
		static void parseFloatToBytesPure(TArray<uint8>& byteArray, float value, bool switchByteOrder = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketServer|SpecialFunctions|Number", meta = (AutoCreateRefTerm = "byteArray"))
		static void parseIntegerToBytesPure(TArray<uint8>& byteArray, int32 value, bool switchByteOrder = false);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SocketServer|SpecialFunctions|Number", meta = (AutoCreateRefTerm = "byteArray"))
		static void parseInteger64ToBytesPure(TArray<uint8>& byteArray, int64 value, bool switchByteOrder = false);


	/**
	*With this function you can start a UE game server at runtime. To close the server, you can simply start the server again on the same port. However, the default map will then be loaded.
	*@param Protocol i.e. "unreal" or "http"
	*@param Host Optional hostname, i.e. "204.157.115.40" or "unreal.epicgames.com", blank if local.
	*@param Port Optional host port
	*@param Map
	* @param name, i.e. "SkyCity", default is "Entry".
	* @param RedirectURL Optional place to download Map if client does not possess it
	* @param Portal Portal to enter through, default is ""
	*/
	UFUNCTION(BlueprintCallable, Category = "SocketServer|SpecialFunctions|Multiplayer", meta = (WorldContext = worldContextObject, AutoCreateRefTerm = "Options"))
		static bool startUEGameHost(UObject* worldContextObject, FString Protocol, FString Host, FString Map, FString RedirectURL, TArray<FString> Options, FString Portal, int32 Port = 7777);

	static ISocketSubsystem* getSocketSubSystem();




	//void addClientSession(FString key, FClientSocketSession session);
	//void removeClientSession(FString key);
	//TMap<FString, FClientSocketSession> getClientSessions();

	//void startTCPClientHandler(FClientSocketSession session, EReceiveFilterServer receiveFilter);
	//bool isTCPServerRun();
	//void setTCPServerRun(bool run);

	void getTcpWrapping(FString& header, FString& footer, ESocketServerTCPMessageWrapping& wrapping);

	TMap<FString, USocketServerPluginTCPServer*> getTcpServerMap();

	TMap<FString,struct FSocketServerToken> fileTokenMap;

private:

	ESocketPlatformServer systemSocketPlatform;
	TMap<FString, UEventBean*> messageEvents;

	TMap<FString, USocketServerPluginUDPServer*> udpServers;
	TMap<FString, USocketServerPluginTCPServer*> tcpServers;
	TMap<FString, URCONServer*> rconServers;


	FString lastUDPServerID;
	FString lastTCPServerID;

	FString tcpMessageHeader = "([{UE4-Head}])";
	FString tcpMessageFooter = "([{UE4-Foot}])";
	ESocketServerTCPMessageWrapping messageWrapping = ESocketServerTCPMessageWrapping::E_None;


	
};

