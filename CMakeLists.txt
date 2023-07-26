cmake_minimum_required(VERSION 3.8)
project(grpc_cpp_example CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

option(protobuf_MODULE_COMPATIBLE TRUE)
find_package(protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)

message(STATUS "Using protobuf ${protobuf_VERSION}")
message(STATUS "Using gRPC ${gRPC_VERSION}")
message(STATUS "Using absl ${absl_VERSION}")

include_directories("$ENV{GRPC_INC_PATH}" src)
link_directories("$ENV{GRPC_LIB_PATH}")

foreach(_target server client)
	add_executable(${_target}
		src/base64.cpp
		src/grpc_cpp_example.grpc.pb.cc
		src/grpc_cpp_example.pb.cc
		"src/${_target}.cpp"
		)
	target_link_libraries(${_target}
		absl::flags
		absl::flags_parse
		protobuf::libprotobuf
		gRPC::grpc++_reflection
		gRPC::grpc++
		)
endforeach()
