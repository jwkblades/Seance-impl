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
	header.headerParts.Length = ntohs(header.headerParts.Length);
	if (header.headerParts.Length < FRAME_LENGTH_MAX)
	{
		mLength = header.headerParts.Length;
		mPayload = new char[header.headerParts.Length];
		mLengthSet = true;
	}
}

Frame::~Frame(void)
{
	delete [] mPayload;
	mPayload = NULL;
}

template<typename T, typename U>
std::size_t frameWriteHelper(char*& buffer, std::size_t& length, T& bytesWritten, const T maxBytes, U& destination, bool allowByteSwap = true)
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

void Frame::write(const char* buffer, std::size_t length)
{
	char* cursor = buffer;
	if (!mLengthSet)
	{
		frameWriteHelper(cursor, length, mLengthBytesWritten, 8, mLength);

		if (mLengthBytesWritten == 8)
		{
			mPayload = new char[mLength];
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
	mPayload = new char[mLength];
	mLengthSet = true;
}
