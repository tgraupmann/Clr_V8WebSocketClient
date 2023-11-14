#pragma once

#include <v8.h>

std::string readWASM(const char* filePath);

std::string readFile(const char* filePath);

void SetCallbackConsoleLog(v8::Local<v8::Context> context, v8::Isolate* isolate);
