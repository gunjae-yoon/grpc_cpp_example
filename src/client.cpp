#include <client.h>
#include <grpcpp/grpcpp.h>
#include <grpc_cpp_example.grpc.pb.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <mutex>
#include <condition_variable>
#include <base64.h>
#include <fstream>

ABSL_FLAG(std::string, target, "localhost:50501", "Server Address");
ABSL_FLAG(std::string, filename, "client", "Filename to request");

using grpc_cpp_example::FileRequest;
using grpc_cpp_example::FileResponse;
using grpc_cpp_example::grpc_cpp_example_service;
using grpc_cpp_example::PushMessage;
using grpc_cpp_example::StatRequest;
using grpc_cpp_example::StatResponse;

namespace grpc_cpp_example {
	class ExampleClient {
	public:
		ExampleClient(std::shared_ptr<grpc::Channel> channel) : stub(grpc_cpp_example_service::NewStub(channel)) {
		}

		FileStatus requestFileStatus(const std::string& filename) {
			FileRequest request;
			FileResponse response;

			request.set_name(filename);
			
			grpc::ClientContext context;
			
			std::mutex lock;
			std::condition_variable cv;
			bool done = false;
			grpc::Status status;
			stub->async()->GetFile(&context, &request, &response,
				[&lock, &cv, &done, &status](grpc::Status s) {
					status = std::move(s);
					std::lock_guard<std::mutex> guard(lock);
					done = true;
					cv.notify_one();
				});
			
			std::unique_lock<std::mutex> guard(lock);
			while (!done) {
				cv.wait(guard);
			}
			
			if (status.ok()) {
				std::string base64 = response.data();
				if (base64.length() > 0) {
					uint64_t originLength = GetOriginalLengthOfBase64(base64);
					char* buffer = new char[originLength];
					DecodeBase64(buffer, originLength, base64.c_str(), base64.length());

					std::ofstream stream;
					stream.open("output", std::ios::out | std::ios::binary);
					stream.write(buffer, originLength);
					stream.close();
					delete(buffer);
				}
				
				return response.stat();
			} else {
				return ERROR;
			}
		}

	private:
		std::unique_ptr<grpc_cpp_example_service::Stub> stub;
	};
}

int main(int argc, char **argv) {
	absl::ParseCommandLine(argc, argv);
	std::string targetStr = absl::GetFlag(FLAGS_target);
	std::string filenameStr = absl::GetFlag(FLAGS_filename);
	grpc_cpp_example::ExampleClient client(grpc::CreateChannel(targetStr, grpc::InsecureChannelCredentials()));
	grpc_cpp_example::FileStatus stat = client.requestFileStatus(filenameStr);

	std::string base64 = grpc_cpp_example::EncodeBase64(filenameStr.c_str(), filenameStr.length());
	
	std::string statStr;
	switch (stat) {
	case grpc_cpp_example::OK:
		statStr = "OK";
		break;
	case grpc_cpp_example::DOES_NOT_EXIST:
		statStr = "DOES_NOT_EXIST";
		break;
	case grpc_cpp_example::PERMISSION_DENIED:
		statStr = "PERMISSION_DENIED";
		break;
	case grpc_cpp_example::ERROR:
	default:
		statStr = "ERROR";
		break;
	}
	std::cout << statStr << std::endl;

	return 0;
}
