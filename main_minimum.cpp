// Ref: https://chromium.googlesource.com/v8/v8/+/main/samples/hello-world.cc

#include "Helpers.h"

// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <iostream>
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

using namespace std;

// The C++ function to be called from JavaScript
void updateDOMCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
	// Extract the argument (assuming it's a string)
	if (args.Length() > 0 && args[0]->IsString()) {
		v8::String::Utf8Value str(args.GetIsolate(), args[0].As<v8::String>());
		std::cout << "UpdateDOM Callback: " << *str << std::endl;
	}
}

void ExposeUpdateDOMCallback(v8::Local<v8::Context> context, v8::Isolate* isolate, v8::Local<v8::Object> global) {
	// Create a new function template with the callback
	v8::Local<v8::FunctionTemplate> functionTemplate =
		v8::FunctionTemplate::New(isolate, updateDOMCallback);

	// Get the function object from the template
	v8::Local<v8::Function> function =
		functionTemplate->GetFunction(isolate->GetCurrentContext()).ToLocalChecked();

	// Set the function name to "updateDOM" in the global object
	global->Set(
		context,
		v8::String::NewFromUtf8(isolate, "updateDOM").ToLocalChecked(),
		function
	);
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
		SetCallbackConsoleLog(context, isolate);
		v8::Context::Scope context_scope(context);

		// Get the global object
		v8::Local<v8::Object> global = context->Global();

		// Expose the updateDOMCallback to JavaScript
		ExposeUpdateDOMCallback(context, isolate, global);

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
			string cSource = R"(
	//load missing functions
	console.warn = console.log;
	console.warn('THIS SHOULD WARN SOMETHING');
	console.error = console.log;
	globalThis.crypto = {
		getRandomValues: function(array) {
		  if (!(array instanceof Uint8Array)) {
			throw new TypeError('Argument should be a Uint8Array');
		  }

		  // Use Math.random to generate random values
		  for (let i = 0; i < array.length; i++) {
			array[i] = Math.floor(Math.random() * 256);
		  }

		  return array;
		}
	};

	const performance_now_startTime = Date.now();
	performance = {
		now: function() {
			return Date.now() - performance_now_startTime;
		}
	};
)";
			cSource += readFile("goclient/js/encoding.min.js");
			cSource += readFile("goclient/js/wasm_exec.js");
			cSource += readWASM("goclient/main.wasm");
			cSource += R"(
	//load wasm
	const goInstance = new Go();
	var module = new WebAssembly.Module(bytes);
	let instance = new WebAssembly.Instance(module, goInstance.importObject);
	console.log('Instance loaded');

	if (goInstance.run) {
		console.log('Run method found');
		goInstance.run(instance);
		console.log('Called run()');
	} else {
		console.log('Run method not found');
	}

	//console.log('instance.exports: ' + JSON.stringify(instance.exports, null, 2));

	/*
	if (instance.exports.main) {
		console.log('Calling WebAssembly function "main"...');
		instance.exports.main();
	} else {
		console.log('WebAssembly function "main" not found.');
	}
	*/

	updateDOM("Call updateDOM from JS");

	//goMyFunc();
)";

			printf("Source:\r\n%s ... %s\r\n",
				cSource.substr(0, 100).c_str(),
				cSource.substr(cSource.size() - 200).c_str());
	
			// Create a string containing the JavaScript source code.
			v8::Local<v8::String> source =
				v8::String::NewFromUtf8(isolate, cSource.c_str()).ToLocalChecked();
			// Compile the source code.
			v8::Local<v8::Script> script =
				v8::Script::Compile(context, source).ToLocalChecked();
			// Run the script to get the result.
			v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
		}
	}

	//printf("Wait to exit...\r\n");
	//Sleep(3000);

	// Dispose the isolate and tear down V8.
	isolate->Dispose();
	v8::V8::Dispose();
	v8::V8::DisposePlatform();
	delete create_params.array_buffer_allocator;
	return 0;
}
