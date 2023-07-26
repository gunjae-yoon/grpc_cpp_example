#include <base64.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace grpc_cpp_example {
	std::string EncodeBase64(char* stream, uint64_t streamLength) {
		static const char* base64EncodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		uint8_t input[3] = {0, };
		uint8_t output[4] = {0, };
		uint64_t iInput = 0;
		uint64_t iResult = 0;
		char* cur = stream;
		char* end = stream + streamLength - 1;
		char* result = new char[(4 * (streamLength / 3)) + (streamLength % 3 ? 4 : 0) + 1];
	
		uint64_t counter = 0;	
		while (cur <= end) {
			// step 1. pre-processing
			iInput = counter % 3;
			input[iInput] = *(cur);
			
			// step 2. transformations
			if ((iInput == 2) || (cur == end)) {
				output[0] = ((input[0] & 0xfc) >> 2);
				output[1] = ((input[0] & 0x3) << 4) | ((input[1] & 0xf0) >> 4);
				output[2] = ((input[1] & 0xf) << 2) | ((input[2] & 0xc0) >> 6);
				output[3] = (input[2] & 0x3f);
				result[iResult++] = base64EncodingTable[output[0]];
				result[iResult++] = base64EncodingTable[output[1]];
				result[iResult++] = (iInput == 0 ? '=' : base64EncodingTable[output[2]]);
				result[iResult++] = (iInput == 0 ? '=' : base64EncodingTable[output[3]]);
				input[0] = 0;
				input[1] = 0;
				input[2] = 0;
			}

			// step 3. next turn 
			counter++;
			cur++;
		}
		result[iResult] = '\0';

		std::string resultStr(result);
		delete(result);
		return resultStr;
	}

	uint64_t DecodeBase64(char* buffer, uint64_t bufferLength, char* base64Text, uint64_t base64TextLength) {
		static const char base64DecodingTable[256] = {
			-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    		52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    		-1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    		15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    		-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    		41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    		-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
			};
		const char* cur = base64Text;
		uint64_t bufferIdx = 0;
		uint64_t phase = 0;
		char digitOld = 0;
		char digitCur = -1;

		while (*(cur) != '/0') {
			digitCur = base64DecodingTable[*(cur)];
			if (digitCur != -1) {
				switch (phase) {
				case 0:
					break;
				case 1:
					if (bufferIdx > bufferLength) {
						// error
						return 0;
					}
					buffer[bufferIdx++] = ((digitOld << 2) | ((digitCur & 0x30) >> 4));
					break;
				case 2:
					if (bufferIdx > bufferLength) {
						// error
						return 0;
					}
					buffer[bufferIdx++] = (((digitOld & 0xf) << 4) | ((digitCur & 0x3c) >> 2));
					break;
				case 3:
					if (bufferIdx > bufferLength) {
						// error
						return 0;
					}
					buffer[bufferIdx++] = (((digitOld & 0x03) << 6) | digitCur);
					break;
				default:
					// error
					return 0;
				}
				digitOld = digitCur;
			}

			// next turn
			phase = (phase + 1) % 4;
			cur++;
		}
		return bufferIdx;
	}

	std::string ReadFileAsBase64(std::string filepath) {
		std::string base64;
		if (std::filesystem::exists(filepath) == false) {
			return base64;
		}

		uint64_t filesize = std::filesystem::file_size(filepath);
		std::ifstream stream(filepath, std::ifstream::binary);
		if (stream) {
			char* buffer = new char[filesize];
			stream.read(buffer, filesize);
			stream.close();
			base64 = EncodeBase64(buffer, filesize);
			delete buffer;
		}

		return base64;
	}

	uint64_t GetOriginalLengthOfBase64(std::string base64) {
		if (base64.length() == 0) {
			return 0;
		}

		uint64_t last = base64.length() - 4;
		uint64_t posPad = base64.find('=', last);
		uint64_t padLength = (posPad == 0 ? 0 : posPad - last);
		return ((base64.length() / 4) * 3) - padLength;
	}
}
