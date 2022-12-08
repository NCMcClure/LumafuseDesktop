// Copyright Low Entry. All Rights Reserved.

#include "LowEntryCompressionLzfLibrary.h"
#include "LowEntryCompressionByteArrayBuffer.h"


int32* ULowEntryCompressionLzfLibrary::cachedHashTab = NULL;


int32 ULowEntryCompressionLzfLibrary::UintByteCount(int32 Value)
{
	return ((Value <= 127) ? 1 : 4);
}

void ULowEntryCompressionLzfLibrary::UintToBytes(ULowEntryCompressionByteArrayBuffer& Array, int32 ArrayOffset, int32 Value)
{
	if(Value <= 127)
	{
		Array.set(ArrayOffset, (uint8) (Value));
	}
	else
	{
		Array.set(ArrayOffset, (uint8) ((Value >> 24) | (1 << 7)));
		Array.set(ArrayOffset + 1, (uint8) (Value >> 16));
		Array.set(ArrayOffset + 2, (uint8) (Value >> 8));
		Array.set(ArrayOffset + 3, (uint8) (Value));
	}
}

int32 ULowEntryCompressionLzfLibrary::BytesToUint(const TArray<uint8>& Array, int32 ArrayOffset)
{
	if((Array.Num() - 1) < ArrayOffset)
	{
		return -1;
	}
	uint8 B = Array[ArrayOffset];
	if(((B >> 7) & 1) == 0)
	{
		return B;
	}
	if((Array.Num() - 4) < ArrayOffset)
	{
		return -1;
	}
	int32 Value = ((B & ~(1 << 7)) << 24) | (Array[ArrayOffset + 1] << 16) | (Array[ArrayOffset + 2] << 8) | Array[ArrayOffset + 3];
	if(Value <= 127)
	{
		return -1;
	}
	return Value;
}


TArray<uint8> ULowEntryCompressionLzfLibrary::Compress(const TArray<uint8>& Bytes, const bool ThreadSafe)
{
	int64 inLen = Bytes.Num();
	if((inLen < SKIP_LENGTH) || (inLen > 0x7fffffff))
	{
		TArray<uint8> Result;
		Result.Reserve(inLen + 1);
		Result.Add(0);
		Result.Append(Bytes);
		return Result;
	}

	ULowEntryCompressionByteArrayBuffer out(inLen + 20, 512);
	out.set(0, 1);
	UintToBytes(out, 1, inLen);
	int64 outPos = 2 + UintByteCount(inLen);
	int64 inPos = 0;
	int32 literals = 0;
	int32 future = (Bytes[0] << 8) | (Bytes[1] & 255);

	int32 hashSize = 0;
	int32* hashTab = NULL;
	if(ThreadSafe)
	{
		hashSize = NONCACHED_HASH_SIZE;
		hashTab = new int32[hashSize];
	}
	else
	{
		hashSize = CACHED_HASH_SIZE;
		if(cachedHashTab == nullptr)
		{
			cachedHashTab = new int32[hashSize];
		}
		hashTab = cachedHashTab;
	}
	while(inPos < (inLen - 4))
	{
		uint8 p2 = Bytes[inPos + 2];
		future = (future << 8) + (p2 & 255);
		int32 off = ((future * 2777) >> 9) & (hashSize - 1);
		int32 ref = hashTab[off];
		hashTab[off] = inPos;
		if((ref < inPos) && (ref > 0) && ((off = inPos - ref - 1) < MAX_OFF) && (Bytes[ref + 2] == p2) && (Bytes[ref + 1] == (uint8) (future >> 8)) && (Bytes[ref] == (uint8) (future >> 16)))
		{
			int32 maxLen = inLen - inPos - 2;
			if(maxLen > MAX_REF)
			{
				maxLen = MAX_REF;
			}
			if(literals == 0)
			{
				outPos--;
			}
			else
			{
				out.set(outPos - literals - 1, (uint8) (literals - 1));
				literals = 0;
			}
			int32 len = 3;
			while((len < maxLen) && (Bytes[ref + len] == Bytes[inPos + len]))
			{
				len++;
			}
			len -= 2;
			if(len < 7)
			{
				out.set(outPos++, (uint8) ((off >> 8) + (len << 5)));
			}
			else
			{
				out.set(outPos++, (uint8) ((off >> 8) + (7 << 5)));
				out.set(outPos++, (uint8) (len - 7));
			}
			out.set(outPos++, (uint8) off);
			outPos++;
			inPos += len;
			future = (Bytes[inPos] << 8) | (Bytes[inPos + 1] & 255);
			future = (future << 8) | (Bytes[inPos + 2] & 255);
			hashTab[((future * 2777) >> 9) & (hashSize - 1)] = inPos++;
			future = (future << 8) | (Bytes[inPos + 2] & 255);
			hashTab[((future * 2777) >> 9) & (hashSize - 1)] = inPos++;
		}
		else
		{
			out.set(outPos++, Bytes[inPos++]);
			literals++;
			if(literals == MAX_LITERAL)
			{
				out.set(outPos - literals - 1, (uint8) (literals - 1));
				literals = 0;
				outPos++;
			}
		}
	}
	if(ThreadSafe)
	{
		delete hashTab;
	}

	while(inPos < inLen)
	{
		out.set(outPos++, Bytes[inPos++]);
		literals++;
		if(literals == MAX_LITERAL)
		{
			out.set(outPos - literals - 1, (uint8) (literals - 1));
			literals = 0;
			outPos++;
		}
	}
	out.set(outPos - literals - 1, (uint8) (literals - 1));
	if(literals == 0)
	{
		outPos--;
	}

	if(outPos >= inLen)
	{
		TArray<uint8> Result;
		Result.Reserve(inLen + 1);
		Result.Add(0);
		Result.Append(Bytes);
		return Result;
	}
	return out.getData(outPos);
}


TArray<uint8> ULowEntryCompressionLzfLibrary::Decompress(const TArray<uint8>& Bytes)
{
	int64 inLen = Bytes.Num();
	if(inLen < 2) // bool (1 byte), (optional: size (minimum of 1 byte)), data (minimum of 1 byte)
	{
		return TArray<uint8>();
	}
	if(Bytes[0] == 0)
	{
		TArray<uint8> Result;
		Result.Reserve(inLen - 1);
		Result.Append((Bytes.GetData() + 1), (inLen - 1));
		return Result;
	}
	if(Bytes[0] != 1)
	{
		return TArray<uint8>();
	}

	int64 outLen = BytesToUint(Bytes, 1);
	if(outLen <= 0)
	{
		return TArray<uint8>();
	}
	int64 inPos = 1 + UintByteCount(outLen);
	ULowEntryCompressionByteArrayBuffer out(outLen, 512);
	int64 outPos = 0;

	do
	{
		if(inPos >= inLen)
		{
			return TArray<uint8>();
		}
		int32 ctrl = Bytes[inPos++] & 255;
		if(ctrl < MAX_LITERAL)
		{
			ctrl++;
			if((inLen - inPos) < ctrl)
			{
				return TArray<uint8>();
			}
			out.set(outPos, Bytes, inPos, ctrl);
			outPos += ctrl;
			inPos += ctrl;
		}
		else
		{
			int32 len = ctrl >> 5;
			if(len == 7)
			{
				if(inPos >= inLen)
				{
					return TArray<uint8>();
				}
				len += Bytes[inPos++] & 255;
			}
			len += 2;
			ctrl = -((ctrl & 0x1f) << 8) - 1;
			if(inPos >= inLen)
			{
				return TArray<uint8>();
			}
			ctrl -= Bytes[inPos++] & 255;
			ctrl += outPos;
			for(int32 i = 0; i < len; i++)
			{
				if((outPos < 0) || (outPos >= outLen))
				{
					return TArray<uint8>();
				}
				if((ctrl < 0) || (ctrl >= outLen))
				{
					return TArray<uint8>();
				}
				out.set(outPos++, out.get(ctrl++));
			}
		}
	} while(outPos < outLen);
	return out.getData();
}
