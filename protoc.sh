#!/bin/bash
if [ ! -d "src" ]
then 
	mkdir "src"
fi
	
protoc --proto_path=$PWD --cpp_out=$PWD/src --grpc_out=$PWD/src --plugin=protoc-gen-grpc=$GRPC_BIN_PATH/grpc_cpp_plugin grpc_cpp_example.proto
