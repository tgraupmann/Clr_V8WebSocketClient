#include "Helpers.h"

#include <fstream>
#include <sstream>
#include <string>

#pragma comment(lib, "third_party_icu_icui18n.dll.lib")
#pragma comment(lib, "third_party_zlib.dll.lib")
#pragma comment(lib, "v8.dll.lib")
#pragma comment(lib, "v8_libbase.dll.lib")
#pragma comment(lib, "v8_libplatform.dll.lib")

using namespace std;

// Function to read the contents of a file and return them as a uint8array buffer
string readWASM(const char* filePath) {

	printf("Reading WASM file!\r\n");

	std::ifstream file(filePath, std::ios::in | std::ios::binary);
	if (!file.is_open()) {
		return "";
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

	/*
	printf("Script:\r\n%s ... %s\r\n",
		content.substr(0, 1000).c_str(),
		content.substr(content.size() - 1000).c_str());
	*/

	printf("WASM file loaded!\r\n");

	return content;
}

std::string readFile(const char* filePath) {
	printf("readFile: %s\r\n", filePath);
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

#pragma region Logging

void PrefixLog(const char* prefix, const v8::FunctionCallbackInfo<v8::Value>& args) {
	printf("%s:", prefix);;
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
void ConsoleLog(const v8::FunctionCallbackInfo<v8::Value>& args) {
	PrefixLog("log", args);
}

void ConsoleWarn(const v8::FunctionCallbackInfo<v8::Value>& args) {
	PrefixLog("warn", args);
}

void ConsoleError(const v8::FunctionCallbackInfo<v8::Value>& args) {
	PrefixLog("error", args);
}

void SetCallbackConsoleLog(v8::Local<v8::Context> context, v8::Isolate* isolate)
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

#pragma endregion Logging

void runJS(v8::Local<v8::Context> context, v8::Isolate* isolate, const char* js)
{
	// Create a string containing the JavaScript source code.
	v8::Local<v8::String> source =
		v8::String::NewFromUtf8(isolate, js).ToLocalChecked();

	// Compile the source code.
	v8::Local<v8::Script> script =
		v8::Script::Compile(context, source).ToLocalChecked();

	//v8::String::Utf8Value strScript(isolate, script);
	//printf("Script:\r\n%s\r\n", *strScript);

	// Run the script
	script->Run(context);
}

v8::Local<v8::Value> runJSResult(v8::Local<v8::Context> context, v8::Isolate* isolate, const char* js, bool printResult)
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
	v8::Local<v8::Value> result = script->Run(context).ToLocalChecked();

	if (printResult)
	{
		// Convert the result to an UTF8 string and print it.
		v8::String::Utf8Value utf8(isolate, result);
		printf("%s\n", *utf8);
	}

	return result;
}
