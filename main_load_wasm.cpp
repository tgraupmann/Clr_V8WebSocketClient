// Ref: https://chromium.googlesource.com/v8/v8/+/main/samples/hello-world.cc

// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libplatform/libplatform.h"
#include "v8-context.h"
#include "v8-initialization.h"
#include "v8-isolate.h"
#include "v8-local-handle.h"
#include "v8-primitive.h"
#include "v8-script.h"

#pragma comment(lib, "third_party_icu_icui18n.dll.lib")
#pragma comment(lib, "third_party_zlib.dll.lib")
#pragma comment(lib, "v8.dll.lib")
#pragma comment(lib, "v8_libbase.dll.lib")
#pragma comment(lib, "v8_libplatform.dll.lib")

using namespace std;

// Function to read the contents of a file and return them as a uint8array buffer
v8::Local<v8::String> loadWASM(v8::Isolate* isolate, const char* filePath) {

	printf("Reading WASM file!\r\n");

	v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, "").ToLocalChecked();

	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return source; // Return an empty vector if the file cannot be opened
	}

	// Determine the size of the file
	file.seekg(0, std::ios::end);
	std::streamsize fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	// Read the contents of the file into a vector
	std::vector<uint8_t> buffer((int)fileSize);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

	// Close the file
	file.close();

	printf("Converting WASM file into string!\r\n");

	source = v8::String::Concat(isolate, source, v8::String::NewFromUtf8(isolate, "let bytes = new Uint8Array([").ToLocalChecked());

	for (uint8_t value : buffer)
	{
		source = v8::String::Concat(isolate,
			source,
			v8::String::NewFromUtf8(isolate, std::to_string(value).c_str()).ToLocalChecked());
		source = v8::String::Concat(isolate,
			source,
			v8::String::NewFromUtf8(isolate, ",").ToLocalChecked());
	}

	source = v8::String::Concat(isolate, source, v8::String::NewFromUtf8(isolate, "]);\n").ToLocalChecked());
	source = v8::String::Concat(isolate, source, v8::String::NewFromUtf8(isolate, "let module = new WebAssembly.Module(bytes);\n").ToLocalChecked());
	source = v8::String::Concat(isolate, source, v8::String::NewFromUtf8(isolate, "let instance = new WebAssembly.Instance(module, {});\n").ToLocalChecked());

	printf("WASM file loaded!\r\n");

	// Return the contents as a uint8array buffer
	return source;
}

int main(int argc, char* argv[]) {
	// Initialize V8.
	v8::V8::InitializeICUDefaultLocation(argv[0]);
	v8::V8::InitializeExternalStartupData(argv[0]);
	std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
	v8::V8::InitializePlatform(platform.get());
	v8::V8::Initialize();
	// Create a new Isolate and make it the current one.
	v8::Isolate::CreateParams create_params;
	create_params.array_buffer_allocator =
		v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	v8::Isolate* isolate = v8::Isolate::New(create_params);
	{
		v8::Isolate::Scope isolate_scope(isolate);
		// Create a stack-allocated handle scope.
		v8::HandleScope handle_scope(isolate);
		// Create a new context.
		v8::Local<v8::Context> context = v8::Context::New(isolate);
		// Enter the context for compiling and running the hello world script.
		v8::Context::Scope context_scope(context);
		{
			// Create a string containing the JavaScript source code.
			v8::Local<v8::String> source =
				v8::String::NewFromUtf8Literal(isolate, "'Hello' + ', World!'");
			// Compile the source code.
			v8::Local<v8::Script> script =
				v8::Script::Compile(context, source).ToLocalChecked();
			// Run the script to get the result.
			v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
			// Convert the result to an UTF8 string and print it.
			v8::String::Utf8Value utf8(isolate, result);
			printf("%s\n", *utf8);
		}
		{
			v8::Local<v8::String> source = loadWASM(isolate, "goclient\\main.wasm");

			source = v8::String::Concat(isolate, source, v8::String::NewFromUtf8(isolate, "goMyFunc();\r\n").ToLocalChecked());

			// Compile the source code.
			v8::Local<v8::Script> script =
				v8::Script::Compile(context, source).ToLocalChecked();

			//v8::String::Utf8Value strScript(isolate, script);
			//printf("Script:\r\n%s\r\n", *strScript);

			// Run the script to get the result.
			script->Run(context).ToLocalChecked();
		}
	}
	// Dispose the isolate and tear down V8.
	isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::DisposePlatform();
	delete create_params.array_buffer_allocator;
	return 0;
}
