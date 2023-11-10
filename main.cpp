#include "pch.h"

#include <iostream>
#include <libplatform/libplatform.h>
#include <v8.h>

#pragma comment(lib, "third_party_icu_icui18n.dll.lib")
#pragma comment(lib, "third_party_zlib.dll.lib")
#pragma comment(lib, "v8.dll.lib")
#pragma comment(lib, "v8_libbase.dll.lib")
#pragma comment(lib, "v8_libplatform.dll.lib")

using namespace std;

int main() {
    // Initialize V8
    v8::V8::InitializeICUDefaultLocation(".");
    v8::V8::InitializeExternalStartupData(".");
    std::unique_ptr<v8::Platform> platform = v8::platform::NewDefaultPlatform();
    v8::V8::InitializePlatform(platform.get());
    v8::V8::Initialize();

    // Create a V8 isolate
    v8::Isolate::CreateParams create_params;
    create_params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    v8::Isolate* isolate = v8::Isolate::New(create_params);
    {
        v8::Isolate::Scope isolateScope(isolate);
        v8::HandleScope handleScope(isolate);

        // Create a context
        v8::Local<v8::Context> context = v8::Context::New(isolate);
        v8::Context::Scope contextScope(context);

        // Create a JavaScript function that returns a string
        v8::MaybeLocal<v8::String> script = v8::String::NewFromUtf8(isolate, R"(
            function getString() {
                return "Hello World";
            }
        )");

        v8::Local<v8::Script> compiledScript;
        if (!script.ToLocal(&compiledScript)) {
            std::cerr << "Failed to compile the script" << std::endl;
            return 1;
        }

        // Run the script with a TryCatch
        v8::TryCatch try_catch(isolate);
        v8::Local<v8::Value> result;
        if (compiledScript->Run(context).ToLocal(&result)) {
            if (try_catch.HasCaught()) {
                // Handle exception
                v8::String::Utf8Value exception(isolate, try_catch.Exception());
                std::cerr << "Exception in script: " << *exception << std::endl;
            }
            else {
                // Call the JavaScript function and get the result
                v8::Local<v8::String> functionName = v8::String::NewFromUtf8(isolate, "getString").ToLocalChecked();
                v8::Local<v8::Function> function = v8::Local<v8::Function>::Cast(context->Global()->Get(context, functionName).ToLocalChecked());
                function->Call(context, context->Global(), 0, nullptr).ToLocal(&result);

                // Check if the result is a string
                if (result->IsString()) {
                    v8::String::Utf8Value utf8(isolate, result.As<v8::String>());
                    std::cout << "JavaScript returned: " << *utf8 << std::endl;
                }
                else {
                    std::cerr << "JavaScript did not return a string" << std::endl;
                }
            }
        }
        else {
            std::cerr << "Failed to run the script" << std::endl;
            return 1;
        }
    }

    // Dispose of the isolate
    isolate->Dispose();

    // Dispose of the platform
    v8::V8::Dispose();

    return 0;
}
