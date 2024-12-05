#pragma once

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <string>
#include <windows.h>

std::string SendPostRequest(const std::string& url, const std::string& data);
void HandleRequest(const std::string& request, HANDLE hPipe);
void SendRegisterRequest(const std::string& data, HANDLE hPipe);
void SendLoginRequest(const std::string& data, HANDLE hPipe);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

#define SERVER_IP "localhost"
#define SERVER_PORT 8080

#endif // REQUEST_HANDLER_H