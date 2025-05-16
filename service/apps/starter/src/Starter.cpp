#include "../include/Starter.h"

int main(int argc, char* argv[]) {
    if (argc > 1 && std::string(argv[1]) == "--exit-ui") {
        showExitUI();
    } else if (argc > 1 && std::string(argv[1]) == "--warning-ui") {
        showWarningUI();
    } else {
        sendLaunchMessage();
    }

    return 0;
}

DWORD getSessionID() {
    return WTSGetActiveConsoleSessionId();
}

void showExitUI() {

    int return_code = 1;

    // Create a secure desktop
    HDESK hDesk = CreateDesktop(
        "SecureDesktop", NULL, NULL, 0, GENERIC_ALL, NULL);

    if (hDesk) {
        // Switch to the secure desktop
        SetThreadDesktop(hDesk);
        SwitchDesktop(hDesk);

        // Display a message box with "Yes" or "No" options
        int result = MessageBox(NULL, "Are you sure?", "Exit", MB_YESNO);

        // Check the result and print to the console
        if (result == IDYES) {
            std::cout << "Chosen: Yes" << std::endl;
            return_code = 0; // Set return code to 0 for "Yes"
        } else if (result == IDNO) {
            std::cout << "Chosen: No" << std::endl;
            return_code = 1; // Set return code to 1 for "No"
        }

        // Switch back to the original desktop
        HDESK hOldDesk = OpenDesktop("Default", 0, FALSE, GENERIC_ALL);
        if (hOldDesk) {
            SwitchDesktop(hOldDesk);
            CloseDesktop(hOldDesk);
        }

        // Close the secure desktop
        CloseDesktop(hDesk);

        // Exit the process with the return code
        ExitProcess(return_code);
    } else {
        MessageBox(NULL, "Failed to create secure desktop", "Error", MB_OK);
    }
}

void sendLaunchMessage() {

    HANDLE hPipe;
    DWORD dwWritten;
    DWORD sessionID = getSessionID();
    TCHAR pipeName[256];

    // Form the pipe name with session ID
    // _stprintf_s(pipeName, _T("%s_%d"), PIPE_NAME, sessionID);

    std::wcout << pipeName << std::endl;

    // Try to open a named pipe
    hPipe = CreateFile(
        PIPE_NAME,  // pipeName, 
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (hPipe != INVALID_HANDLE_VALUE) {
        // Send a message to the pipe
        WriteFile(
            hPipe,
            "launch_app:",
            strlen("launch_app:"),
            &dwWritten,
            NULL
        );

        // Close the pipe
        CloseHandle(hPipe);
    } else {
        std::cerr << "Failed to open named pipe." << std::endl;
    }
}

void showWarningUI() {
    MessageBox(
        NULL, 
        "Client Already exists.", 
        "Warning", 
        MB_OK | MB_ICONWARNING | MB_TOPMOST // Добавляем MB_TOPMOST
    );
}

