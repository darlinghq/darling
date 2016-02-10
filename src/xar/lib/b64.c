/*
 * Copyright (c) 2004 Rob Braun
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Rob Braun nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * 1-Oct-2004
 * DRI: Rob Braun <bbraun@synack.net>
 */

#include <stdlib.h>
#include <string.h>

#ifdef _BTEST_
int main(int argc, char* argv[]) {
    unsigned char* enc = benc(argv[1], strlen(argv[1]));
    printf("%s", enc);
    printf("%s\n", bdec(enc, strlen(enc)));
}
#endif


/*
 * The code below derives from "Secure Programming Cookbook for C and
 * C++"* and adapted by Kogule, Ryo (kogule@opendarwin.org).
 *
 * *John Viega and Matt Messier, O'Reilly, 2003
 *  http://www.secureprogramming.com/
 *
 * Readability improvements by Luke Bellandi, 2007 (luke@apple.com)
 */

typedef enum _B64CommandCodes {
	B64_NullTerminator		= -3,
	B64_PaddingCharacter	= -2,
	B64_IgnorableCharacter	= -1
} B64CommandCodes;

static char b64revtb[256] = {
  -3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*0-15*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*16-31*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, /*32-47*/
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1, /*48-63*/
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, /*64-79*/
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, /*80-95*/
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, /*96-111*/
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, /*112-127*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*128-143*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*144-159*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*160-175*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*176-191*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*192-207*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*208-223*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, /*224-239*/
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  /*240-255*/
};

typedef enum _B64CodecErrorCodes {
	B64_noError			= 0,
	B64_decodeError		= 1,
	B64_bufferOverrun	= 2
} B64CodecErrorCodes;

#define B64_INPUT_BLOCK_OFFSET	((inputIndex - 1 - ignorableCharacterCount) % 4)

static unsigned int raw_base64_decode(
  const unsigned char *input, unsigned char *output, size_t inLengthToDecode,
  size_t *outputDecodedLength)
{
    int currentBase64Value;
	unsigned int inputIndex = 0;
	unsigned int ignorableCharacterCount = 0;
	unsigned int i;
    unsigned char decodedBuffer[3];
	unsigned char currentInputBlockPaddingCharacterCount = 0;
	size_t *decodedCharacterCount;
	size_t dummyValue;
	
	if (outputDecodedLength == NULL) {
		// do this so that if caller passes in NULL for outputDecodedLength
		// it can still be handled easily
		decodedCharacterCount = &dummyValue;
	} else {
		decodedCharacterCount = outputDecodedLength;
	}
	*decodedCharacterCount = 0;
	
    while ( (inputIndex <= inLengthToDecode) &&
			(currentInputBlockPaddingCharacterCount == 0) )
	{
        currentBase64Value = b64revtb[input[inputIndex]];
		inputIndex++;
		
        switch (currentBase64Value) {
			// [1] Handle control characters
            case B64_NullTerminator:
                if (B64_INPUT_BLOCK_OFFSET != 0) return B64_decodeError;
				// copy remaining characters with padding
				if (currentInputBlockPaddingCharacterCount > 0) {
				    for (i = 0; i < (3 - currentInputBlockPaddingCharacterCount); i++) {
						*output++ = decodedBuffer[i];
						(*decodedCharacterCount)++;
					}
				}
                return B64_noError;
			
            case B64_PaddingCharacter:
                if (B64_INPUT_BLOCK_OFFSET < 2) {
                    /* Invalid here -- only characters 3 and/or 4 of the
					   input block can be padding */
                    return B64_decodeError;
                } else if (B64_INPUT_BLOCK_OFFSET == 2) {
                    /* Make sure there's appropriate padding */
                    if (input[inputIndex] != '=') return B64_decodeError;
                    decodedBuffer[2] = 0;
                    currentInputBlockPaddingCharacterCount = 2;
                    break;
                } else {
                    currentInputBlockPaddingCharacterCount = 1;
                    break;
                }
                return B64_noError;
			
            case B64_IgnorableCharacter:
                ignorableCharacterCount++;
                break;
			
            default:
				// [2] Handle encoded data
                switch (B64_INPUT_BLOCK_OFFSET) {
                    case 0:
                        decodedBuffer[0] = currentBase64Value << 2;
                        break;
                    case 1:
                        decodedBuffer[0] |= (currentBase64Value >> 4);
                        decodedBuffer[1] = currentBase64Value << 4;
                        break;
                    case 2:
                        decodedBuffer[1] |= (currentBase64Value >> 2);
                        decodedBuffer[2] = currentBase64Value << 6;
                        break;
                    case 3:
                        decodedBuffer[2] |= currentBase64Value;
                        for (i = 0; i < (3 - currentInputBlockPaddingCharacterCount); i++) {
							*output++ = decodedBuffer[i];
							(*decodedCharacterCount)++;
						}
                        break;
                }
                break;
        }
    }
	
    if (inputIndex > inLengthToDecode) return B64_bufferOverrun;
	
    for (i = 0; i < (3 - currentInputBlockPaddingCharacterCount); i++) {
		*output++ = decodedBuffer[i];
		(*decodedCharacterCount)++;
	}
	
    return B64_noError;
}

unsigned char* xar_from_base64(const unsigned char* input, size_t inputLength, size_t *outputLength)
{
    int err;
    unsigned char *output;

	// N.B.: This is a conservative estimate of space needed.  It is NOT
	// an exact value -- the exact length of the decoded data will be
	// calculated during the decode operation.
    output = malloc(3 * (inputLength / 4 + 1));
    if (output == NULL) return NULL;

    err = raw_base64_decode(input, output, inputLength, outputLength);

    if (err != B64_noError) {
        free(output);
        return NULL;
    }
	
    return output;
}
