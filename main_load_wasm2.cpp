// Ref: https://chromium.googlesource.com/v8/v8/+/main/samples/hello-world.cc

// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <fstream>
#include <sstream>
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

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma comment(lib, "third_party_icu_icui18n.dll.lib")
#pragma comment(lib, "third_party_zlib.dll.lib")
#pragma comment(lib, "v8.dll.lib")
#pragma comment(lib, "v8_libbase.dll.lib")
#pragma comment(lib, "v8_libplatform.dll.lib")

using namespace std;

v8::Local<v8::Value> runJS(v8::Local<v8::Context> context, v8::Isolate* isolate, const char* js)
{
	// Create a string containing the JavaScript source code.
	v8::Local<v8::String> source =
		v8::String::NewFromUtf8(isolate, js).ToLocalChecked();

	// Compile the source code.
	v8::Local<v8::Script> script =
		v8::Script::Compile(context, source).ToLocalChecked();

	v8::String::Utf8Value strScript(isolate, script);
	printf("Script:\r\n%s\r\n", *strScript);

	// Run the script to get the result.
	return script->Run(context).ToLocalChecked();
}

std::string readFile(const char* filePath) {
	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return ""; // Return an empty string if the file cannot be opened
	}

	// Read the contents of the file into a stringstream
	std::stringstream buffer;
	buffer << file.rdbuf();

	// Close the file
	file.close();

	// Return the contents as a string
	return buffer.str();
}

// Function to read the contents of a file and return them as a uint8array buffer
void loadWASM(v8::Local<v8::Context> context, v8::Isolate* isolate, const char* filePath) {

	printf("Reading WASM file!\r\n");

	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return; // Return an empty vector if the file cannot be opened
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

	string content = "let bytes = new Uint8Array([";

	int i = 0;
	for (uint8_t value : buffer)
	{
		content += std::to_string(value);
		++i;
		if (i == buffer.size())
		{
			break;
		}
		content += ",";
	}

	content += "]);\n";

	printf("Script:\r\n%s ... %s\r\n",
		content.substr(0, 1000).c_str(),
		content.substr(content.size() - 1000).c_str());

	v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, content.c_str()).ToLocalChecked();

	// Compile the source code.
	v8::Local<v8::Script> script =
		v8::Script::Compile(context, source).ToLocalChecked();

	// Run the script to get the result.
	script->Run(context).ToLocalChecked();

	runJS(context, isolate, "let module = new WebAssembly.Module(bytes);\n");
	runJS(context, isolate, "let instance = new WebAssembly.Instance(module, {});\n");

	printf("WASM file loaded!\r\n");
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

		if (true)
		{
			runJS(context, isolate, R"(
	globalThis.crypto = 'ignore';
	globalThis.performance = 'ignore';

	class TextEncoder {
	  constructor() {
	  }
	}

	globalThis.TextEncoder = TextEncoder;

	class TextDecoder {
	  constructor() {
	  }
	}
	globalThis.TextDecoder = TextDecoder;
)");

			string wasmJS = readFile("goclient/js/wasm_exec.js");
			runJS(context, isolate, wasmJS.c_str());

			loadWASM(context, isolate, "goclient\\main.wasm");

			//runJS(context, isolate, "goMyFunc();\r\n");
		}
	}

	printf("Wait to exit...\r\n");

	Sleep(1000);

	// Dispose the isolate and tear down V8.
	isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::DisposePlatform();
	delete create_params.array_buffer_allocator;
	return 0;
}
