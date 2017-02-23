#ifndef __SEANCE_FRAME_H
#define __SEANCE_FRAME_H

/*
  0                   1                   2                   3
  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+---------------+-------------+
 |F|R|M|C|R|R|R|R|R|R|R|R|R|R|R|R|R| Opcode        | Length      |
 |I|S|A|R|S|S|S|S|S|S|S|S|S|S|S|S|S|               | (8/64 bits) |
 |N|P|S|C|V|V|V|V|V|V|V|V|V|V|V|V|V|               |             |
 | | |K| |1|2|3|4|5|6|7|8|9|A|B|C|D|               |             |
 +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+---------------+ - - - - - - +
 | Extended Length 8 bytes, if Length == 255                     |
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 | Extended Length continued.                                    |
 +---------------------------------------------------------------+
 | Message ID, 4 bytes                                           |
 +---------------------------------------------------------------+
 | Response to Message ID, 4 bytes,                              |
 | if RSP == 1                                                   |
 +---------------------------------------------------------------+
 | Masking Key, if Mask == 1                                     |
 +---------------------------------------------------------------+
 | CRC32, if CRC == 1                                            |
 +---------------------------------------------------------------+
 | Payload Data ...                                              |
 + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 |                                                           ... |
 +---------------------------------------------------------------+
 */

union FrameHeader
{
public:
	char fullHeader[4];
	struct
	{
		char FIN:1, // Final frame in message.
			 RSP:1, // Response to previous message.
			 MASK:1, // If a mask exists on the payload.
			 CRC:1, // If a checksum has been provided.
			 RSV:4; // Reserved
		char RSV2; // More reserved
		char Opcode; // Operation.
		char Length; // Length, if < 255
	} headerParts;
};

class Frame
{
public:
	Frame(const FrameHeader& header);
	~Frame(void);

	void write(const char* buffer, std::size_t length);

	uint64_t size(void) const;
	void size(uint64_t newSize);

	// FIXME - Can we get this to not depend on the socket code?
	friend Socket& operator<<(Socket& sock, const Frame& frame);
private:
	FrameHeader mHeader;
	uint64_t mLength;
	uint32_t mMessageID;
	uint32_t mRespondingToID;
	uint32_t mMask;
	uint32_t mCRC;
	char* mPayload;

	bool mLengthSet;
	uint8_t mLengthBytesWritten;
	uint8_t mMessageIDBytesWritten;
	uint8_t mResponseToIDBytesWritten;
	uint8_t mMaskBytesWritten;
	uint8_t mCRCBytesWritten;
	uint64_t mPayloadBytesWritten;
};

#endif
