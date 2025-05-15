#pragma once

#ifndef STARTER_H
#define STARTER_H

#include <windows.h>
#include <wtsapi32.h>
#include <tchar.h>
#include <iostream>
#include <string>

#include "../../../include/Config.h"

DWORD getSessionID();
void showExitUI();
void sendLaunchMessage();
void showWarningUI();

#endif // STARTER_H