#include "Frame.h"

Frame::Frame(const FrameHeader& header):
	mHeader(header),
	mLength(0),
	mMessageID(0),
	mRespondingToID(0),
	mMask(0),
	mCRC(0),
	mPayload(NULL),
	/* Internal values only beyond this point */
	mLengthSet(false),
	mLengthBytesWritten(0),
	mMessageIDBytesWritten(0),
	mResponseToIDBytesWritten(0),
	mMaskBytesWritten(0),
	mCRCBytesWritten(0),
	mPayloadBytesWritten(0)
{
	uint16_t headerLen =  ntohs(header.headerParts.Length);
	if (headerLen < FRAME_LENGTH_MAX)
	{
		mLength = headerLen;
		mPayload = new uint8_t[headerLen];
		mLengthSet = true;
	}
}

Frame::~Frame(void)
{
	delete [] mPayload;
	mPayload = NULL;
}

template<typename T, typename U>
std::size_t frameWriteHelper(uint8_t*& buffer, std::size_t& length, T& bytesWritten, const T maxBytes, U& destination, bool allowByteSwap = true)
{
	if (bytesWritten >= maxBytes)
	{
		return 0;
	}

	std::size_t i;
	for (i = 0; bytesWritten < maxBytes && i < length; i++, bytesWritten++)
	{
		destination |= buffer[i];
		destination <<= 8;
	}
	buffer += i;
	length -= i;

	if (allowByteSwap && bytesWritten == maxBytes)
	{
		switch (maxBytes)
		{
			case 1:
				break;
			case 2:
				destination = ntohs(destination);
				break;
			case 4:
				destination = ntohl(destination);
				break;
			case 8:
				destination = ntohll(destination);
				break;
			default:
				// WAT
				throw sizeof(destination);
		}
	}
	return i;
}

void Frame::write(const uint8_t* buffer, std::size_t length)
{
	uint8_t* cursor = buffer;
	if (!mLengthSet)
	{
		frameWriteHelper(cursor, length, mLengthBytesWritten, 8, mLength);

		if (mLengthBytesWritten == 8)
		{
			mPayload = new uint8_t[mLength];
			mLengthSet = true;
		}
	}

	frameWriteHelper(cursor, length, mMessageIDBytesWritten, 4, mMessageID);

	if (mHeader.headerParts.RSP)
	{
		frameWriteHelper(cursor, length, mResponseToIDBytesWritten, 4, mRespondingToID);
	}
	if (mHeader.headerParts.MASK)
	{
		frameWriteHelper(cursor, length, mMaskBytesWritten, 4, mMask);
	}

	frameWriteHelper(cursor, length, mCRCBytesWritten, 4, mCRC);

	frameWriteHelper(cursor, length, mPayloadBytesWritten, mLength, mPayload, false);

	if (mPayloadBytesWritten == mLength)
	{
		// The entire payload has been read. Time to verify the CRC...
		uint32_t calculatedCrc = 0;
		calculatedCrc = CRC32::calculate(calculatedCrc, &mHeader.fullHeader, 2); // Only CRC the flags and opcode.
		if (mLength >= uint16_t(-1))
		{
			// Only CRC the 64-bit length if needed.
			uint64_t networkLen = htonll(mLength);
			calculatedCrc = CRC32::calculate(calculatedCrc, &networkLen, sizeof(networkLen));
		}
		calculatedCrc = CRC32::calculate(calculatedCrc, &mMessageID, sizeof(mMessageID));
		if (mHeader.headerParts.RSP == 1)
		{
			calculatedCrc = CRC32::calculate(calculatedCrc, &mRespondingToID, sizeof(mRespondingToID));
		}
		if (mHeader.headerParts.MASK == 1)
		{
			calculatedCrc = CRC32::calculate(calculatedCrc, &mMask, sizeof(mMask));
		}
		const static uint32_t blankCRC = 0;
		calculatedCrc = CRC32::calculate(calculatedCrc, &blankCRC, sizeof(mCRC));
		calculatedCrc = CRC32::calculate(calculatedCrc, mPayload, mLength);
		if (htonl(calculatedCrc) != mCRC)
		{
			throw "CRC mismatch!";
		}

	}
}

uint64_t Frame::size(void) const
{
	return mLength;
}

void Frame::size(uint64_t newSize)
{
	mLength = newSize;
	if (mPayload)
	{
		delete [] mPayload;
	}
	mPayload = new uint8_t[mLength];
	mLengthSet = true;
}
