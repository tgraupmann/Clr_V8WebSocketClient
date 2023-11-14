#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libplatform/libplatform.h"
#include "v8.h"
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

// Define a simple C++ function to be called from JavaScript
void ConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Check the number of arguments passed
    if (args.Length() < 1) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "Invalid number of arguments")
            .ToLocalChecked());
        return;
    }

    printf("log:");
    for (int i = 0; i < args.Length(); ++i)
    {

        // Check if the first argument is a string
        if (!args[i]->IsString()) {
            args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(args.GetIsolate(), "Argument must be a string")
                .ToLocalChecked());
            return;
        }

        // Convert the first argument to a string
        v8::Local<v8::String> str = args[i].As<v8::String>();
        v8::String::Utf8Value utf8(args.GetIsolate(), str);

        // Print the string to the console
        printf(" %s", *utf8);
    }
    printf("\r\n");
}

// Define a simple C++ function to be called from JavaScript
void MyCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Check the number of arguments passed
    if (args.Length() < 1) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "Invalid number of arguments")
            .ToLocalChecked());
        return;
    }

    printf("MyCallback:");
    for (int i = 0; i < args.Length(); ++i)
    {

        // Check if the first argument is a string
        if (!args[i]->IsString()) {
            args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(args.GetIsolate(), "Argument must be a string")
                .ToLocalChecked());
            return;
        }

        // Convert the first argument to a string
        v8::Local<v8::String> str = args[i].As<v8::String>();
        v8::String::Utf8Value utf8(args.GetIsolate(), str);

        // Print the string to the console
        printf(" %s", *utf8);
    }
    printf("\r\n");
}

// Function to read the contents of a file and return them as a string
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

#pragma region Load WebAssembly

v8::Local<v8::Value> runWASM(v8::Local<v8::Context> context, v8::Isolate* isolate)
{
    // Load and run WebAssembly module
    std::string wasmCode = readFile("goclient\\main.wasm");
    printf("Source: (length=%d)\r\n", wasmCode.size());

    // Create a string containing the JavaScript source code with the Wasm code
    v8::Local<v8::String> source = v8::String::NewFromUtf8(isolate, wasmCode.c_str()).ToLocalChecked();


    // Compile the source code.
    v8::Local<v8::Script> script =
        v8::Script::Compile(context, source).ToLocalChecked();

    v8::String::Utf8Value strScript(isolate, script);
    printf("Script:\r\n%s\r\n", *strScript);

    // Run the script to get the result.
    return script->Run(context).ToLocalChecked();
}

#pragma endregion Load WebAssembly

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

#pragma region Create a console.log callback
        {
            // Expose the C++ function to JavaScript
            v8::Local<v8::Object> global = context->Global();
            v8::Local<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New(isolate);
            global_template->Set(
                isolate,
                "log",
                v8::FunctionTemplate::New(isolate, ConsoleLog));

            // Create an instance of the template and add it to the global object
            v8::Local<v8::Object> global_instance = global_template->NewInstance(context).ToLocalChecked();
            global->Set(
                context,
                v8::String::NewFromUtf8(isolate, "console").ToLocalChecked(),
                global_instance);
        }
#pragma endregion Create a console.log callback

#pragma region Create C++ MyCallback
        {
            // Expose the C++ function to JavaScript
            v8::Local<v8::Object> global = context->Global();
            v8::Local<v8::ObjectTemplate> global_template = v8::ObjectTemplate::New(isolate);
            global_template->Set(
                isolate,
                "myCallback",
                v8::FunctionTemplate::New(isolate, MyCallback));

            // Create an instance of the template and add it to the global object
            v8::Local<v8::Object> global_instance = global_template->NewInstance(context).ToLocalChecked();
            global->Set(
                context,
                v8::String::NewFromUtf8(isolate, "MyNamespace").ToLocalChecked(),
                global_instance);
        }
#pragma endregion Create C++ MyCallback


        {
            /*

            // Create a string containing the JavaScript source code.
            v8::Local<v8::String> source =
                v8::String::NewFromUtf8Literal(isolate, R"(

// const ws = new TCPConnectSocket("wss://example.org");
// <unknown>:64: Uncaught ReferenceError: WebSocket is not defined
//
// #
// # Fatal error in v8::ToLocalChecked
// # Empty MaybeLocal
// #
console.log('WebSocket is not available in V8!!!');

MyNamespace.myCallback('hello');
'Hello, World!';
                    )");
            */

#pragma region Add Callback for WebAssembly

            v8::Local<v8::Value> result = runJS(context, isolate, R"(

function updateDOM(text) {
    console.log('updateDOM:', text);
}

"Hello from JS" // last line of the JS script, this will be the returned result
                    )");

            // Convert the result to an UTF8 string.
            v8::String::Utf8Value utf8(isolate, result);
            // Print the result using printf.
            printf("Result: %s\n", *utf8);            

#pragma endregion Add Callback for WebAssembly

#pragma region Use WebAssembly

            runWASM(context, isolate);

#pragma endregion Use WebAssembly

            Sleep(1000);

            runJS(context, isolate, "goMyFunc()");

            Sleep(1000);

        }
    }
    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}
