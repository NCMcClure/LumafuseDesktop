// Fill out your copyright notice in the Description page of Project Settings.


#include "Classes/LumafuseStreamingUtilities.h"

#include "SocketServerBPLibrary.h"
#include "LowEntryExtendedStandardLibrary/Public/Classes/LowEntryExtendedStandardLibrary.h"
#include "LowEntryCompression/Public/Classes/LowEntryCompressionLibrary.h"


//Distill Chunk Packets and Send To Client

// Looping through the frame buffer data chunk to:
// 1) Trim the alpha channels from the pixel buffer chunk to reduce packet size - these channels will get
//    re-added when packet is received on client-side to ensure frame is reconstructed back into RGBA
// 2) Compress the newly trimmed pixel buffer chunk using lossless LZF compression
// 3) Organize the packet header data into byte form and bundle everything into a newly created byte array
// 4) Send the constructed packet out to the client over UDP

// Packet structure:
// As TArray<uint8>
// { Header
//   [
//    [0] DisplayID (uint8)
//    [1] FrameID (uint8)
//    [2-5] ChunkIndex (int32->TArray<uint8> of size 4)
//    [6-9] ChunkSubArrayIndex (int32->TArray<uint8> of size 4)
//    [10-13] PayloadSize (int32->TArray<uint8> of size 4)
//   ]
//   Payload
//   [
//    [14...] Compressed PixelPacket (TArray<uint8>)
//   ]
// }

void ULumafuseStreamingUtilities::DistillChunkPacketsAndSendToClient(uint8 DisplayID, uint8 FrameID, int32 SplitSize,
	int32 ChunkIndex, const TArray<uint8>& BufferChunk, USocketServerBPLibrary* ServerTarget,
	FString ClientSessionID, FString OptionalServerID)
{
	//Initialize loop parameters to ensure that the frame data gets constructed properly
	const int32 bufferChunkSize = BufferChunk.Num();
	const int32 firstIndex = ChunkIndex * (bufferChunkSize / SplitSize);
	const int32 lastIndex = firstIndex + (bufferChunkSize / SplitSize);

	//Initializing increment index used for getting the byte sub array from the pixel buffer chunk
	int32 ChunkSubArrayIndex = 0;

	//Create an empty string to use as input when sending data to the client since we are only sending bytes
	const FString MessageToSend = "";

	//Looping through the pixel buffer chunk to pull out sub arrays, optimize them, and send them to the client as byte packets
	for (int i = firstIndex; i <  lastIndex; i++)
	{
		//Header for the frame data
		TArray<uint8> TotalPacket;
		TotalPacket.Add(DisplayID);
		TotalPacket.Add(FrameID);
		TotalPacket.Append(ULowEntryExtendedStandardLibrary::IntegerToBytes(ChunkIndex));
		TotalPacket.Append(ULowEntryExtendedStandardLibrary::IntegerToBytes(ChunkSubArrayIndex));

		//Payload

		//Efficiently extract byte sub array from the pixel buffer chunk, trim the alpha, and compress it
		TArray<uint8> TrimmedPixelPacket;
		TArray<uint8> PixelPacket = ULowEntryExtendedStandardLibrary::BytesSubArray(BufferChunk, ChunkSubArrayIndex, SplitSize);
		TrimAlphaFromPacket(PixelPacket, TrimmedPixelPacket);
		
		//Clear the original pixel packet to save memory
		PixelPacket.Empty();

		//Compress the trimmed pixel packet to prep it for sending
		TArray<uint8> CompressedPixelPacket = ULowEntryCompressionLibrary::CompressLzfThreadSafe(TrimmedPixelPacket);

		//Add payload size to packet header
		TotalPacket.Append(ULowEntryExtendedStandardLibrary::IntegerToBytes(CompressedPixelPacket.Num()));

		//Clear Trimmed pixel packet to save memory since we already added it to the total packet
		TrimmedPixelPacket.Empty();

		//Add payload to packet
		TotalPacket.Append(CompressedPixelPacket);

		//Send packet to client
		ServerTarget->socketServerSendUDPMessageToClient(ClientSessionID, MessageToSend, TotalPacket,false, true, ESocketServerUDPSocketType::E_SSS_CLIENT, OptionalServerID);
		
		//Increment the index that's used to determine the starting index for the new pixel buffer packet
		ChunkSubArrayIndex += SplitSize;
	}
	
}

void ULumafuseStreamingUtilities::OptimizeAndSendFrameChunkPackets(const TArray<FLumafuseFramePacket>& frameBufferDataChunk, USocketServerBPLibrary* serverTarget, FString clientSessionID, FString messageToSend, FString optionalServerID)
{
	
	for (int chunkIndex = 0; chunkIndex < frameBufferDataChunk.Num(); chunkIndex++)
	{
		//Convert header properties to byte arrays so that we can append the pixel buffer packet payload to them later
		TArray<uint8> packetIndexAsBytes = ULowEntryExtendedStandardLibrary::IntegerToBytes(frameBufferDataChunk[chunkIndex].index);
		
		TArray<uint8> trimmedPixelPacket;

		//Remove alpha channels from the pixel buffer packet
		for (int alphaIndex = 0; alphaIndex < frameBufferDataChunk[chunkIndex].pixelPacket.Num(); alphaIndex += 4)
		{
			TArray<uint8> byteArrayToAppend = ULowEntryExtendedStandardLibrary::BytesSubArray(frameBufferDataChunk[chunkIndex].pixelPacket, alphaIndex, 3);
			trimmedPixelPacket.Append(byteArrayToAppend.GetData(), 3); 
		}

		//Compressing pixel buffer packet using LZF after packet is finished getting alpha channels removed
		TArray<uint8> compressedPixelPacket = ULowEntryCompressionLibrary::CompressLzfThreadSafe(frameBufferDataChunk[chunkIndex].pixelPacket);
		TArray<uint8> payloadSizeAsBytes;
		
		TArray<uint8> totalPacket;
		
		//Header
		totalPacket.Add(frameBufferDataChunk[chunkIndex].displayID);
		totalPacket.Add(frameBufferDataChunk[chunkIndex].frameID);
		totalPacket.Append(packetIndexAsBytes);
		totalPacket.Append(ULowEntryExtendedStandardLibrary::IntegerToBytes(compressedPixelPacket.Num()));
		
		//Payload
		totalPacket.Append(compressedPixelPacket);
		
		//Send packet
		serverTarget->socketServerSendUDPMessageToClient(clientSessionID, messageToSend, totalPacket, false, true, ESocketServerUDPSocketType::E_SSS_CLIENT, optionalServerID);
	}

}

void ULumafuseStreamingUtilities::TrimAlphaFromPacket(const TArray<uint8>& OriginalPacket,
	TArray<uint8>& TrimmedPacket)
{
	for (int alphaIndex = 0; alphaIndex < OriginalPacket.Num(); alphaIndex += 4)
	{
		TArray<uint8> byteArrayToAppend = ULowEntryExtendedStandardLibrary::BytesSubArray(OriginalPacket, alphaIndex, 3);
		TrimmedPacket.Append(byteArrayToAppend.GetData(), 3);
	}
}







