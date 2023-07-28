#include <server.h>
#include <base64.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpc_cpp_example.grpc.pb.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/strings/str_format.h>
#include <iostream>
#include <filesystem>
#include <string>
#include <chrono>
#include <fstream>

ABSL_FLAG(uint16_t, port, 50501, "Server port for grpc_cpp_example");

using grpc_cpp_example::grpc_cpp_example_service;
using grpc_cpp_example::StatRequest;
using grpc_cpp_example::StatResponse;
using grpc_cpp_example::FileRequest;
using grpc_cpp_example::FileResponse;
using grpc_cpp_example::PushMessage;

namespace grpc_cpp_example {
	class ExampleServer final : public grpc_cpp_example_service::CallbackService {
		grpc::ServerUnaryReactor* GetFile(
				grpc::CallbackServerContext* context,
				const FileRequest* request,
				FileResponse* reply
				) override {
			std::cout << "GetFile: " << request->name() << std::endl;
			if (std::filesystem::exists(request->name())) {
				// pure bytestream
				std::string filepath = request->name();
     				uint64_t filesize = std::filesystem::file_size(filepath);
     				std::ifstream stream(filepath, std::ifstream::binary);
     				if (stream) {
     				        char* buffer = new char[filesize];
     				        stream.read(buffer, filesize);
     				        stream.close();
					std::string content(buffer, filesize);
					reply->set_data(content);
				}
				reply->set_stat(OK);
				/*
				// BASE64
				std::string base64 = ReadFileAsBase64(request->name());
				reply->set_stat(OK);
				reply->set_data(base64);
				*/
			} else {
				reply->set_stat(DOES_NOT_EXIST);
				reply->set_data("empty");
			}
			grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
			reactor->Finish(grpc::Status::OK);
			return reactor;
		}	

		grpc::ServerUnaryReactor* GetStat(
				grpc::CallbackServerContext* context,
				const StatRequest* request,
				StatResponse* reply
				) override {
			std::cout << "GetStat: " << request->command() << std::endl;
			reply->set_data("response");
			grpc::ServerUnaryReactor* reactor = context->DefaultReactor();
			reactor->Finish(grpc::Status::OK);
			return reactor;
		}	
	};
	
	void RunServer(uint16_t port) {
		std::string serverAddress = absl::StrFormat("0.0.0.0:%d", port);	
		ExampleServer service;

		grpc::EnableDefaultHealthCheckService(true);
		grpc::reflection::InitProtoReflectionServerBuilderPlugin();
		grpc::ServerBuilder builder;

		builder.SetMaxMessageSize(INT32_MAX);
		builder.SetMaxSendMessageSize(INT32_MAX);
		builder.SetMaxReceiveMessageSize(INT32_MAX);
		builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());
		builder.RegisterService(&service);
		std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

		std::cout << "Server listening on: " << serverAddress << std::endl;
		server->Wait();
	}
}

int main(int argc, char** argv) {
	absl::ParseCommandLine(argc, argv);
	grpc_cpp_example::RunServer(absl::GetFlag(FLAGS_port));
	return 0;
}
