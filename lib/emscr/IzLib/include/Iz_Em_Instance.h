#pragma once
#include <emscripten/emscripten.h>
#include <libcxx/iosfwd>
#include <libcxx/string>
#include <libcxx/map>
#include <stdio.h>



worker_handle converterWorker;

void PostMessage(std::string outputMessage);

void DebugMessage(std::string message);

void DebugErrorMessage( std::string message, int32_t result);

void onWorkerMessage(char *data, int size, void *arg);