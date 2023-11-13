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

#pragma comment(lib, "third_party_icu_icui18n.dll.lib")
#pragma comment(lib, "third_party_zlib.dll.lib")
#pragma comment(lib, "v8.dll.lib")
#pragma comment(lib, "v8_libbase.dll.lib")
#pragma comment(lib, "v8_libplatform.dll.lib")

using namespace std;

// Define a simple C++ function to be called from JavaScript
void MyCallback(const v8::FunctionCallbackInfo<v8::Value>& args) {
    // Check the number of arguments passed
    if (args.Length() < 1) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "Invalid number of arguments")
            .ToLocalChecked());
        return;
    }

    // Check if the first argument is a string
    if (!args[0]->IsString()) {
        args.GetIsolate()->ThrowException(
            v8::String::NewFromUtf8(args.GetIsolate(), "Argument must be a string")
            .ToLocalChecked());
        return;
    }

    // Convert the first argument to a string
    v8::Local<v8::String> str = args[0].As<v8::String>();
    v8::String::Utf8Value utf8(args.GetIsolate(), str);

    // Print the string to the console
    printf("Callback invoked with argument: %s\n", *utf8);
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

#pragma region Create C++ Callback

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


#pragma endregion Create C++ Callback


        {
            // Create a string containing the JavaScript source code.
            v8::Local<v8::String> source =
                v8::String::NewFromUtf8Literal(isolate, "MyNamespace.myCallback('hello'); 'Hello, World!';");
            // Compile the source code.
            v8::Local<v8::Script> script =
                v8::Script::Compile(context, source).ToLocalChecked();
            // Run the script to get the result.
            v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();
            // Convert the result to an UTF8 string.
            v8::String::Utf8Value utf8(isolate, result);
            // Print the result using printf.
            printf("%s\n", *utf8);
        }
    }
    // Dispose the isolate and tear down V8.
    isolate->Dispose();
    v8::V8::Dispose();
    v8::V8::DisposePlatform();
    delete create_params.array_buffer_allocator;
    return 0;
}
