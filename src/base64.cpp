#include <base64.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace grpc_cpp_example {
	
	void MakeBase64Chunk(std::string& base64, char b1, char b2, char b3) {
		static const char* base64EncodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		uint32_t concatBits = ((b1 << 16) | (b2 << 8) | (b3));
		char b64[4] = {};
		b64[0] = base64EncodingTable[(concatBits >> 18) & 0b0011'1111];
		b64[1] = base64EncodingTable[(concatBits >> 12) & 0b0011'1111];
		if (b3 != 0) {
			b64[2] = base64EncodingTable[(concatBits >> 6) & 0b0011'1111];
			b64[3] = base64EncodingTable[(concatBits) & 0b0011'1111];
		} else if (b2 != 0) {
			b64[2] = base64EncodingTable[(concatBits >> 6) & 0b0011'1111];
			b64[3] = '=';
		} else {
			b64[2] = '=';
			b64[3] = '=';
		}

		std::copy(b64 + 0, b64 + 4, std::back_inserter(base64));
	}
	
	std::string EncodeBase64Simple(char* stream, uint64_t streamLength) {
		uint64_t tripples = streamLength / 3;
		uint64_t remains = streamLength % 3;
		
		std::string result;
		result.reserve((tripples + 2) * 4);
		
		char* cur = nullptr;
		for (uint64_t i = 0; i < tripples; i++) {
			cur = stream + (i * 3);
			MakeBase64Chunk(result, cur[0], cur[1], cur[2]);
		}
		
		cur = stream + (tripples * 3);
		if (remains == 2) {
			MakeBase64Chunk(result, cur[0], cur[1], 0);
		} else if (remains == 1) {
			MakeBase64Chunk(result, cur[0], 0, 0);
		}
		
		return result;
	}

	std::string EncodeBase64Efficient(char* stream, uint64_t streamLength) {
		static const char* base64EncodingTable = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		uint32_t concatBits = 0;
		char b64[4] = {};

		uint64_t tripples = streamLength / 3;
		uint64_t remains = streamLength % 3;
		
		std::string result;
		result.reserve((tripples + 2) * 4);
		
		char* cur = nullptr;
		for (uint64_t i = 0; i < tripples; i++) {
			cur = stream + (i * 3);
			concatBits = ((cur[0] << 16) | (cur[1] << 8) | (cur[2]));
			b64[0] = base64EncodingTable[(concatBits >> 18) & 0b0011'1111];
			b64[1] = base64EncodingTable[(concatBits >> 12) & 0b0011'1111];
			b64[2] = base64EncodingTable[(concatBits >> 6) & 0b0011'1111];
			b64[3] = base64EncodingTable[(concatBits) & 0b0011'1111];
			std::copy(b64 + 0, b64 + 4, std::back_inserter(result));
		}
		
		cur = stream + (tripples * 3);
		if (remains == 2) {
			concatBits = ((cur[0] << 16) | (cur[1] << 8));
			b64[0] = base64EncodingTable[(concatBits >> 18) & 0b0011'1111];
			b64[1] = base64EncodingTable[(concatBits >> 12) & 0b0011'1111];
			b64[2] = base64EncodingTable[(concatBits >> 6) & 0b0011'1111];
			b64[3] = '=';
			std::copy(b64 + 0, b64 + 4, std::back_inserter(result));
		} else if (remains == 1) {
			concatBits = (cur[0] << 16);
			b64[0] = base64EncodingTable[(concatBits >> 18) & 0b0011'1111];
			b64[1] = base64EncodingTable[(concatBits >> 12) & 0b0011'1111];
			b64[2] = '=';
			b64[3] = '=';
			std::copy(b64 + 0, b64 + 4, std::back_inserter(result));
		}
		
		return result;
	}

	std::string EncodeBase64(char* stream, uint64_t streamLength) {
		return EncodeBase64Efficient(stream, streamLength);
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
		uint64_t padLength = 0;
		std::string::reverse_iterator rit = base64.rbegin();
		while (*(rit) == '=') {
			padLength++;
			rit++;
		}
		return ((base64.length() * 3) / 4) - padLength;
	}
}
