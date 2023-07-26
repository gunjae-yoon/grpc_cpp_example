#include <stdint.h>
#include <string>

namespace grpc_cpp_example {
	std::string EncodeBase64(char* stream, uint64_t streamLength);
	uint64_t DecodeBase64(char* buffer, uint64_t bufferLength, char* base64Text, uint64_t base64TextLength);

	std::string ReadFileAsBase64(std::string filepath);

	uint64_t GetOriginalLengthOfBase64(std::string base64);
}
