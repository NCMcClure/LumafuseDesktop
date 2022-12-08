// Fill out your copyright notice in the Description page of Project Settings.


#include "Classes/LumafuseStreamingUtilities.h"

#include "SocketServerBPLibrary.h"
#include "LowEntryExtendedStandardLibrary/Public/Classes/LowEntryExtendedStandardLibrary.h"
#include "LowEntryCompression/Public/Classes/LowEntryCompressionLibrary.h"

void ULumafuseStreamingUtilities::BuildFrameDataChunk(uint8 frameID, int32 splitSize, int32 chunkIndex,
	const TArray<uint8>& bufferChunk, TArray<FLumafuseFramePacket>& frameData)
{
	//Initialize loop parameters to ensure that the frame data gets constructed properly
	const int32 bufferChunkSize = bufferChunk.Num();
	const int32 firstIndex = chunkIndex * (bufferChunkSize / splitSize);
	const int32 lastIndex = firstIndex + (bufferChunkSize / splitSize);

	//Initializing increment index used for getting the byte sub array from the pixel buffer chunk
	int32 chunkSubArrayIndex = 0;

	//Looping through the pixel buffer chunk to pull out sub arrays and add them to the frame data
	for (int i = firstIndex; i <  lastIndex; i++)
	{
		FLumafuseFramePacket tempFramePacket;
		tempFramePacket.frameID = frameID;
		tempFramePacket.index = chunkSubArrayIndex;

		//Efficiently extract byte sub array from the pixel buffer chunk
		tempFramePacket.pixelPacket = ULowEntryExtendedStandardLibrary::BytesSubArray(bufferChunk, chunkSubArrayIndex, splitSize);

		//Add frame packet struct to the frame data array
		frameData.Add(tempFramePacket);
		
		//Increment the index that's used to determine the starting index for the new pixel buffer packet
		chunkSubArrayIndex += splitSize;
	}
	
}

//Optimize And Send Frame Chunk Packets

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
//    [6-9] PayloadSize (int32->TArray<uint8> of size 4)
//   ]
//   Payload
//   [
//    [10...] Compressed PixelPacket (TArray<uint8>)
//   ]
// }

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
