// Copyright 2017-2020 David Romanski (Socke). All Rights Reserved.

#include "FileFunctionsSocketServer.h"

UFileFunctionsSocketServer* UFileFunctionsSocketServer::fileFunctionsSocketServer;

UFileFunctionsSocketServer::UFileFunctionsSocketServer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	fileFunctionsSocketServer = this;
}

UFileFunctionsSocketServer* UFileFunctionsSocketServer::getFileFunctionsSocketServerTarget() {
	return fileFunctionsSocketServer;
}

FString UFileFunctionsSocketServer::getCleanDirectory(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	if (directoryType == EFileFunctionsSocketServerDirectoryType::E_ad) {
		return FPaths::ConvertRelativePathToFull(filePath);
	}
	else {
		FString ProjectDir = FPaths::ProjectDir();
		return FPaths::ConvertRelativePathToFull(ProjectDir + filePath);
	}
}

void UFileFunctionsSocketServer::writeBytesToFile(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success) {
	success = FFileHelper::SaveArrayToFile(bytes, *getCleanDirectory(directoryType, filePath));
}

void UFileFunctionsSocketServer::addBytesToFileAndCloseIt(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, TArray<uint8> bytes, bool& success) {
	FArchive* writer = IFileManager::Get().CreateFileWriter(*getCleanDirectory(directoryType, filePath), EFileWrite::FILEWRITE_Append);
	if (!writer) {
		success = false;
		return;
	}
	writer->Seek(writer->TotalSize());
	writer->Serialize(bytes.GetData(), bytes.Num());
	writer->Close();
	delete writer;

	success = true;
}

//void UFileFunctionsSocketServer::splittFile(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, int32 parts, bool& success){
//	if (parts <= 0)
//		parts = 1;
//	FArchive* reader = IFileManager::Get().CreateFileReader(*getCleanDirectory(directoryType, filePath));
//	if (!reader) {
//		success = false;
//		return;
//	}
//	
//	int64 splittAfterBytes = reader->TotalSize()/ ((int64)parts);
//	TArray<uint8> bytes;
//
//	for (int32 i = 0; i < parts; i++){
//		bytes.AddUninitialized(splittAfterBytes);
//		reader->Serialize(bytes.GetData(), splittAfterBytes);
//		if (FFileHelper::SaveArrayToFile(bytes, *getCleanDirectory(directoryType, filePath)) == false) {
//			success = false;
//			return;
//		}
//		splittAfterBytes =
//		reader->Seek();
//	}
//
//}

TArray<uint8> UFileFunctionsSocketServer::readBytesFromFile(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, bool& success) {
	TArray<uint8> result;
	success = FFileHelper::LoadFileToArray(result, *getCleanDirectory(directoryType, filePath));
	return result;
}

void UFileFunctionsSocketServer::readStringFromFile(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, bool& success, FString& data) {
	data.Empty();
	success = FFileHelper::LoadFileToString(data, *getCleanDirectory(directoryType, filePath));
}

void UFileFunctionsSocketServer::writeStringToFile(EFileFunctionsSocketServerDirectoryType directoryType, FString data, FString filePath, EFileFunctionsSocketServerEncodingOptions fileEncoding, bool& success) {
	success = FFileHelper::SaveStringToFile(data, *getCleanDirectory(directoryType, filePath), (FFileHelper::EEncodingOptions)fileEncoding);
}



void UFileFunctionsSocketServer::getMD5FromFile(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, bool& success, FString& MD5) {
	getMD5FromFileAbsolutePath(getCleanDirectory(directoryType, filePath), success, MD5);
}

void UFileFunctionsSocketServer::getMD5FromFileAbsolutePath(FString filePath, bool& success, FString& MD5) {
	MD5.Empty();
	FArchive* reader = IFileManager::Get().CreateFileReader(*filePath);
	if (!reader) {
		success = false;
		return;
	}

	TArray<uint8> byteArrayTmp;
	int64 totalSize = reader->TotalSize();
	int64 loadedBytes = 0;
	int64 leftUploadBytes = 1024;


	if (totalSize < leftUploadBytes)
		leftUploadBytes = totalSize;


	uint8 Digest[16];
	FMD5 Md5Gen;

	while ((loadedBytes + leftUploadBytes) <= totalSize) {
		byteArrayTmp.Reset(leftUploadBytes);
		byteArrayTmp.AddUninitialized(leftUploadBytes);
		reader->Serialize(byteArrayTmp.GetData(), byteArrayTmp.Num());
		loadedBytes += leftUploadBytes;
		reader->Seek(loadedBytes);

		Md5Gen.Update(byteArrayTmp.GetData(), byteArrayTmp.Num());
	}

	leftUploadBytes = totalSize - loadedBytes;
	if (leftUploadBytes > 0) {
		byteArrayTmp.Reset(leftUploadBytes);
		byteArrayTmp.AddUninitialized(leftUploadBytes);
		reader->Serialize(byteArrayTmp.GetData(), byteArrayTmp.Num());
		loadedBytes += leftUploadBytes;
		Md5Gen.Update(byteArrayTmp.GetData(), byteArrayTmp.Num());
	}

	if (reader != nullptr) {
		reader->Close();
		delete reader;
	}

	if (totalSize != loadedBytes) {
		success = false;
		return;
	}

	Md5Gen.Final(Digest);
	for (int32 i = 0; i < 16; i++) {
		MD5 += FString::Printf(TEXT("%02x"), Digest[i]);
	}

	success = true;
}

void UFileFunctionsSocketServer::stringToBase64String(FString string, FString& base64String) {
	base64String.Empty();
	FTCHARToUTF8 Convert(*string);
	TArray<uint8> bytes;
	bytes.Append(((uint8*)((ANSICHAR*)Convert.Get())), Convert.Length());
	base64String = FBase64::Encode(bytes);
}

void UFileFunctionsSocketServer::base64StringToString(FString& string, FString base64String) {
	string.Empty();
	TArray<uint8> bytes;
	if (FBase64::Decode(*base64String, bytes)) {
		bytes.Add(0x00);// null-terminator
		char* Data = (char*)bytes.GetData();
		string = FString(UTF8_TO_TCHAR(Data));
	}
}


void UFileFunctionsSocketServer::bytesToBase64String(TArray<uint8> bytes, FString& base64String) {
	base64String.Empty();
	base64String = FBase64::Encode(bytes);
}

TArray<uint8> UFileFunctionsSocketServer::base64StringToBytes(FString base64String, bool& success) {
	TArray<uint8> fileData;
	if (base64String.Len() % 2 != 0 || base64String.Len() < 4) {
		success = false;
		return fileData;
	}
	success = FBase64::Decode(*base64String, fileData);
	return fileData;
}

void UFileFunctionsSocketServer::fileToBase64String(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, bool& success, FString& base64String, FString& fileName) {
	base64String.Empty();
	fileName.Empty();
	TArray<uint8> fileData;
	if (!FFileHelper::LoadFileToArray(fileData, *getCleanDirectory(directoryType, filePath))) {
		success = false;
		return;
	}
	base64String = FBase64::Encode(fileData);
	success = true;
}

bool UFileFunctionsSocketServer::fileExists(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	return FPaths::FileExists(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketServer::fileExistsAbsolutePath(FString filePath) {
	return FPaths::FileExists(*filePath);
}

bool UFileFunctionsSocketServer::directoryExists(EFileFunctionsSocketServerDirectoryType directoryType, FString path) {
	return FPaths::DirectoryExists(*getCleanDirectory(directoryType, path));
}

int64 UFileFunctionsSocketServer::fileSize(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().FileSize(*getCleanDirectory(directoryType, filePath));
}

int64 UFileFunctionsSocketServer::fileSizeAbsolutePath(FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().FileSize(*filePath);
}

bool UFileFunctionsSocketServer::deleteFile(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketServer::deleteFileAbsolutePath(FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*filePath);
}

bool UFileFunctionsSocketServer::deleteDirectory(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().DeleteDirectory(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketServer::isReadOnly(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().IsReadOnly(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketServer::moveFile(EFileFunctionsSocketServerDirectoryType directoryTypeTo, FString filePathTo, EFileFunctionsSocketServerDirectoryType directoryTypeFrom, FString filePathFrom) {
	return FPlatformFileManager::Get().GetPlatformFile().MoveFile(*getCleanDirectory(directoryTypeTo, filePathTo), *getCleanDirectory(directoryTypeFrom, filePathFrom));
}

bool UFileFunctionsSocketServer::setReadOnly(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, bool bNewReadOnlyValue) {
	return FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*getCleanDirectory(directoryType, filePath), bNewReadOnlyValue);
}

FDateTime UFileFunctionsSocketServer::getTimeStamp(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetTimeStamp(*getCleanDirectory(directoryType, filePath));
}

void UFileFunctionsSocketServer::setTimeStamp(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, FDateTime DateTime) {
	FPlatformFileManager::Get().GetPlatformFile().SetTimeStamp(*getCleanDirectory(directoryType, filePath), DateTime);
}

FDateTime UFileFunctionsSocketServer::getAccessTimeStamp(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetAccessTimeStamp(*getCleanDirectory(directoryType, filePath));
}

FString UFileFunctionsSocketServer::getFilenameOnDisk(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	return FPlatformFileManager::Get().GetPlatformFile().GetFilenameOnDisk(*getCleanDirectory(directoryType, filePath));
}

bool UFileFunctionsSocketServer::createDirectory(EFileFunctionsSocketServerDirectoryType directoryType, FString path) {
	return FPlatformFileManager::Get().GetPlatformFile().CreateDirectory(*getCleanDirectory(directoryType, path));
}

void UFileFunctionsSocketServer::getAllFilesFromDirectory(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, int32& count, TArray<FString>& files, TArray<FString>& filePaths, FString fileType) {
	if (filePath.Len() < 1) {
		return;
	}
	files.Empty();
	filePaths.Empty();

	FString dir = getCleanDirectory(directoryType, filePath);
	FPaths::NormalizeDirectoryName(filePath);

	if (!FPaths::DirectoryExists(dir)) {
		return;
	}
	IFileManager& FileManager = IFileManager::Get();

	dir += "/" + fileType;

	FileManager.FindFiles(files, *dir, true, false);


	filePath += "/";
	for (int32 i = 0; i < files.Num(); i++) {
		filePaths.Add((filePath + files[i]));
		//UE_LOG(LogTemp, Display, TEXT("->%s"), *files[i]);
	}
	count = files.Num();
}

FFileFunctionsSocketServerOpenFile UFileFunctionsSocketServer::openFile(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {

	FArchive* writer = IFileManager::Get().CreateFileWriter(*getCleanDirectory(directoryType, filePath), EFileWrite::FILEWRITE_Append);
	FFileFunctionsSocketServerOpenFile file;
	file.writer = writer;
	return file;
}

int64 UFileFunctionsSocketServer::addBytesToFile(FFileFunctionsSocketServerOpenFile openFile, TArray<uint8> bytes) {
	//UE_LOG(LogTemp, Warning, TEXT("xxxxx WRITE: %i"), bytes.Num());
	if (openFile.writer != nullptr) {
		openFile.writer->Seek(openFile.writer->TotalSize());
		openFile.writer->Serialize(bytes.GetData(), bytes.Num());
		//openFile.writer->Flush();
		return openFile.writer->TotalSize();
	}
	return 0;
}

void UFileFunctionsSocketServer::closeFile(FFileFunctionsSocketServerOpenFile openFile) {
	if (openFile.writer != nullptr) {
		openFile.writer->Close();
		openFile.writer = nullptr;
	}
}


FString  UFileFunctionsSocketServer::encryptMessageWithAES(FString message, FString key) {

	if (message.IsEmpty() || key.Len() != 32) {
		return FString();
	}

	TArray<uint8> data = FStringToByteArray(message);
	data.Add(0x00);// null-terminator
	const int64 encryptedFileSize = Align(data.Num(), FAES::AESBlockSize);

	if (data.Num() < encryptedFileSize) {
		data.AddUninitialized(encryptedFileSize - data.Num());
	}

	FAES::EncryptData(data.GetData(), encryptedFileSize, TCHAR_TO_ANSI(*key));

	FString encryptedBase64String = FString();
	bytesToBase64String(data, encryptedBase64String);

	return encryptedBase64String;
}

FString  UFileFunctionsSocketServer::decryptMessageWithAES(FString message, FString key) {
	if (message.IsEmpty() || key.Len() != 32) {
		UE_LOG(LogTemp, Error, TEXT("decryptMessageFromAES: Wrong key length."));
		return FString();
	}

	bool success = false;
	TArray<uint8> data = base64StringToBytes(message, success);


	const int64 encryptedFileSize = Align(data.Num(), FAES::AESBlockSize);
	if (data.Num() < encryptedFileSize) {
		UE_LOG(LogTemp, Error, TEXT("decryptMessageFromAES: Wrong string length. This is not an AES encrypted string."));
		return FString();
	}

	FAES::DecryptData(data.GetData(), encryptedFileSize, TCHAR_TO_ANSI(*key));

	return FString(UTF8_TO_TCHAR((char*)data.GetData()));

}


TArray<uint8> UFileFunctionsSocketServer::FStringToByteArray(FString s) {
	FTCHARToUTF8 Convert(*s);
	TArray<uint8> data;
	data.Append((uint8*)Convert.Get(), Convert.Length());
	return data;
}


FString UFileFunctionsSocketServer::int64ToString(int64 Num) {
	FString str = FString();
	const TCHAR* DigitToChar = TEXT("9876543210123456789");
	constexpr int64 ZeroDigitIndex = 9;
	bool bIsNumberNegative = Num < 0;
	const int64 TempBufferSize = 32; // 32 is big enough
	TCHAR TempNum[TempBufferSize];
	int64 TempAt = TempBufferSize; // fill the temp string from the top down.

	// Convert to string assuming base ten.
	do
	{
		TempNum[--TempAt] = DigitToChar[ZeroDigitIndex + (Num % 10)];
		Num /= 10;
	} while (Num);

	if (bIsNumberNegative)
	{
		TempNum[--TempAt] = TEXT('-');
	}

	const TCHAR* CharPtr = TempNum + TempAt;
	const int64 NumChars = TempBufferSize - TempAt;
	str.Append(CharPtr, NumChars);

	return str;
}



void UFileFunctionsSocketServer::readBytesFromFileInPartsAsync(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, int32 bufferSize, float delayBetweenReadsInSeconds) {
	UFileFunctionsSocketServer::getFileFunctionsSocketServerTarget()->readBytesFromFileInPartsAsyncInternal(directoryType, filePath, bufferSize, delayBetweenReadsInSeconds);
}

void UFileFunctionsSocketServer::readBytesFromFileInPartsAsyncInternal(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath, int32 bufferSize, float delayBetweenReadsInSeconds) {

	FString dir = UFileFunctionsSocketServer::getCleanDirectory(directoryType, filePath);

	if (readFileInPartsThreads.Find(*dir) != nullptr) {
		UE_LOG(LogTemp, Warning, TEXT("ReadBytesFromFileInPartsAsync: %s is being read already. Operation canceled."), *dir);
		return;
	}

	FReadFileInPartsSocketServerThread* readThread = new FReadFileInPartsSocketServerThread(dir, bufferSize, delayBetweenReadsInSeconds);
	readFileInPartsThreads.Add(dir, readThread);
}

void UFileFunctionsSocketServer::cancelReadBytesFromFileInParts(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	UFileFunctionsSocketServer::getFileFunctionsSocketServerTarget()->cancelReadBytesFromFileInPartsInternal(directoryType, filePath);
}

void UFileFunctionsSocketServer::cancelReadBytesFromFileInPartsInternal(EFileFunctionsSocketServerDirectoryType directoryType, FString filePath) {
	FString dir = UFileFunctionsSocketServer::getCleanDirectory(directoryType, filePath);

	if (readFileInPartsThreads.Find(*dir) != nullptr) {
		(*readFileInPartsThreads.Find(*dir))->stopThread();
	}
}

void UFileFunctionsSocketServer::cleanReadBytesFromFileInParts(FString cleanDir) {
	if (readFileInPartsThreads.Find(*cleanDir) != nullptr) {
		readFileInPartsThreads.Remove(*cleanDir);
	}
}