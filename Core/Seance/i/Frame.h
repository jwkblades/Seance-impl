#ifndef __SEANCE_FRAME_H
#define __SEANCE_FRAME_H

#include <cstdint>

/**
 * NOTE - All values are in network-byte-order and MUST be properly converted to
 *        host-byte-order.
 *
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *     +-+-+-+-+-+-+-+-+---------------+-------------------------------+
 *     |F|R|M|R|R|R|R|R| Opcode        | Length (16/64 bits)           |
 *     |I|S|A|S|S|S|S|S|               |                               |
 *     |N|P|S|V|V|V|V|V|               |                               |
 *     | | |K|0|1|2|3|4|               |                               |
 *     +-+-+-+-+-+-+-+-+---------------+ - - - - - - - - - - - - - - - +
 *     | Extended Length 8 bytes, if Length == 65535                   |
 *     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 *     | Extended Length continued.                                    |
 *     +---------------------------------------------------------------+
 *     | Message ID, 4 bytes                                           |
 *     +---------------------------------------------------------------+
 *     | Response to Message ID, 4 bytes,                              |
 *     | if RSP == 1                                                   |
 *     +---------------------------------------------------------------+
 *     | Masking Key, if Mask == 1                                     |
 *     +---------------------------------------------------------------+
 *     | CRC32, if CRC == 1                                            |
 *     +---------------------------------------------------------------+
 *     | Payload Data ...                                              |
 *     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 *     |                                                           ... |
 *     +---------------------------------------------------------------+
 *
 * FIN: The final packet of a message. This indicates that the previous
 *     message has been completed.
 *
 * RSP: 1 if this message is a response, 0 otherwise.
 *
 * MASK: 1 if there is a mask on the message payload, 0 otherwise. The client
 *     _must_ always mask their messages with a suitably difficult to guess
 *     random 4-byte value. The server _may_ mask its messages as well.
 *
 * RSV0-RSV4: Reserved for future versions of Seance. _Must_ be 0, or the
 *     recipient _must_ close the connection.
 *
 * Opcode: The opcode for the message.
 *
 * Length: Unsigned 16-bit value specifying the payload length.
 *     If Length == 65535, then the following 8 bytes provide the payload
 *     length as an unsigned 64-bit value.
 *
 * Extended Length: Unsigned 64-bit value, only present if Length == 65535.
 *
 * Message ID: Unsigned 32-bit value specifying a unique identifier for the
 *     message that has been sent. Message IDs _must_ be monotonically
 *     increasing values (within their session (read socket connection),
 *     wrapping as appropriate.
 *     Server Message IDs and Client Message IDs vary slightly based off of
 *     their usable range of the 32 bit value.
 *     Server Message IDs _must_ have the first (most significant bit) set.
 *     Client Message IDs _must_ not have the first (most significant bit) set.
 *
 * Response to Message ID: Unsigned 32-bit value providing the Message ID to
 *     which this message is a response. Only present if RSP is set to 1.
 *
 * Masking Key: Unsigned 32-bit value in which each byte of the value, in
 *     order, is XORed against each byte of the payload, in order and wrapping
 *     as necessary. Only present if MASK is set to 1.
 *
 * CRC32: The 32-bit CRC of the payload and header combined (with the CRC32
 *     section being all zeroes during the CRC generation). _Must_ always be
 *     present. See [ISO 3309] for the CRC specification, or [RFC 1952] for a
 *     reference implementation.
 *
 * Payload Data: Length, or Extended Length bytes long. The Payload Data's
 *     purpose is determined by the opcode provided, and may be outside this
 *     specification's scope.
 *
 *
 * ----------------------------------------------------------------------------
 *
 * Valid Opcodes
 *
 * 0x0 A continuation frame
 *
 * 0x1 A text frame
 *
 * 0x2 A binary frame
 *
 * 0x3 An extension frame
 *
 * 0x4-0x7 Reserved for future non-control frames
 *
 * 0x8 Close connection
 *
 * 0x9 A ping/ pong frame
 *
 * 0xA Establish semi-persistent session
 *
 * 0xB-0xF Reserved for future control frames
 *
 * ----------------------------------------------------------------------------
 * Establishing the opening handshake:
 *
 * When an initial Seance connection is being opened, a packet must be sent from
 * the client to the server (initiator to target) specifying the version of the
 * Seance protocol that the user (client/ initiator) wishes to use. The server
 * (target) then responds with the highest protocol version which is less than
 * or equal to the one requested by the client (initiator).
 *
 * If the server (target) doesn't support any version LTE the client's requested
 * protocol version, then the server MUST close the connection at this point.
 *
 * For more information on version strings, see `Version Strings` below.
 *
 * The version handshake MUST be enclosed in a binary frame. And MUST NOT
 * contain any other data.
 *
 * Examples:
 *                                      +----------------------------------+
 *                                      | KEY                              |
 *                                      +----------------------------------+
 *                                      | > Received by server from client |
 *                                      | < Response from server to client |
 *                                      +----------------------------------+
 *
 * In the case where the server only supports a lower version of the protocol
 * than the client does:
 * > MAJOR=1 MINOR=2 MICRO=3 BUILD=4
 * < MAJOR=1 MINOR=2 MICRO=2 BUILD=0
 *
 * If the server supports the same version as the client:
 * > MAJOR=1 MINOR=2 MICRO=3 BUILD=4
 * < MAJOR=1 MINOR=2 MICRO=3 BUILD=4
 *
 * If the server only supports protocol versions greater than the client does:
 * > MAJOR=1 MINOR=2 MICRO=3 BUILD=4
 * *Server closes connection*
 *
 * ----------------------------------------------------------------------------
 * Version Strings:
 *
 * The Seance protocol stores version strings as an eight byte value consisting
 * of 4 16-bit (2-byte) unsigned integral values. The 4 values contain
 * information as follows:
 *      0                   1                   2                   3
 *      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2
 *     +---------------------------------+-------------------------------+
 *     | MAJOR (2 bytes)                 | MINOR (2 bytes)               |
 *     +---------------------------------+-------------------------------+
 *     | MICRO (2 bytes)                 | BUILD (2 bytes)               |
 *     +---------------------------------+-------------------------------+
 *
 *
 */

const uint16_t FRAME_LENGTH_MAX = 65535;

union FrameHeader
{
public:
	uint8_t fullHeader[4];
	struct
	{
		uint8_t  FIN:1, // Final frame in message.
			     RSP:1, // Response to previous message.
			     MASK:1, // If a mask exists on the payload.
			     RSV:5; // Reserved
		uint8_t  Opcode; // Operation.
		uint16_t Length; // Length, if < 65535
	} headerParts __attribute__((packed));
};

class Frame
{
public:
	/**
	 * Frame constructor. It is expected that the FrameHeader is being passed in
	 * RAW (in network byte order, and untouched) as the Frame will take care of
	 * byte-order swapping where necessary.
	 */
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
