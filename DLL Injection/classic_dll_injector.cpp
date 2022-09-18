
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>

DWORD GetProcId(const char *ProcName)
{

    DWORD ProcId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    

    if(!hSnap || hSnap == INVALID_HANDLE_VALUE)
    {
        std::cout << "[!] Error in CreateSnapshot!" << std::endl;
        return 0;
    }
    else
    {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(PROCESSENTRY32);
        if(Process32First(hSnap, &pe))
        {
            do {
                if(!_stricmp((char*)pe.szExeFile, ProcName))
                {
                    ProcId = pe.th32ProcessID;
                    break;
                }
            }
            while(Process32Next(hSnap, &pe));
        }
    }
    CloseHandle(hSnap);
    return ProcId;
}

int main(int argc, char* argv[])
{
    // const char* ProcessName;
    const char* DllPath;
    HANDLE hProc;
    if(argv[1] == NULL || argv[2] == NULL)
    {
        std::cout << "Usage: .\\classic_dll_injector.exe [ProcessName] [DllPath]" << std::endl;
    }
    else
    {
        DllPath = argv[2];
        DWORD PID = GetProcId(argv[1]);
        if(PID == 0)
        {
            std::cout << "[!] Invalid Process Name" << std::endl;
            return 0;
        }
        std::cout << "The Process ID : " << PID << std::endl;
        std::cout << "The Dll Path   : " << DllPath << std::endl;
        
        hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
        if(hProc != INVALID_HANDLE_VALUE || !hProc)
        {
            void* mem_exec = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if(!mem_exec)
            {
                std::cout << "[!] Error Allocation Target Memory Address" << std::endl;
                return 0;
            }
            WriteProcessMemory(hProc, mem_exec, DllPath, strlen(DllPath) + 1 , 0);
            HANDLE Thread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE) GetProcAddress( GetModuleHandle(TEXT("kernel32.dll") ), "LoadLibraryA"), mem_exec, 0, 0);
            // func LoadLibraryA ((lpParameter) mem_exec) () {};
            
            if(Thread)
            {
                CloseHandle(Thread);
            }
        }
        if(hProc)
        {
            CloseHandle(hProc);
        }
    }
    return 1;

}