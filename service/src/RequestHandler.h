#pragma once

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <windows.h>

#include <string>

using namespace std;

string SendPostRequest(const std::string& url, const std::string& data);
string SendGetRequest(const std::string& url);
string SendPatchRequest(const string& url, const string& data);
void HandleRequest(const std::string& request, HANDLE hPipe);
void SendRegisterRequest(const std::string& data, HANDLE hPipe);
void SendLoginRequest(const std::string& data, HANDLE hPipe);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

// #define SERVER_IP "192.168.3.134"
#define SERVER_IP "localhost"
#define SERVER_PORT "8081"

#endif  // REQUEST_HANDLER_H