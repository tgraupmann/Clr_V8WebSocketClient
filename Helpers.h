#pragma once

#include <v8.h>

std::string readWASM(const char* filePath);

std::string readFile(const char* filePath);

void SetCallbackConsoleLog(v8::Local<v8::Context> context, v8::Isolate* isolate);

void updateDOMCallback(const v8::FunctionCallbackInfo<v8::Value>& args);

void ExposeUpdateDOMCallback(v8::Local<v8::Context> context, v8::Isolate* isolate, v8::Local<v8::Object> global);

void runJS(v8::Local<v8::Context> context, v8::Isolate* isolate, const char* js);

v8::Local<v8::Value> runJSResult(v8::Local<v8::Context> context, v8::Isolate* isolate, const char* js, bool printResult);
