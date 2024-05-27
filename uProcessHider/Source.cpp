#include <Windows.h>
#include <stdio.h>
#include <iostream>

__declspec(dllimport) DWORD TargetProcessPid;
__declspec(dllimport) WCHAR TargetProcessName[32];

void Banner()
{
	std::cout << R"(

  _    _ _____                             _    _ _     _      
 | |  | |  __ \                           | |  | (_)   | |     
 | |  | | |__) | __ ___   ___ ___  ___ ___| |__| |_  __| | ___ 
 | |  | |  ___/ '__/ _ \ / __/ _ \/ __/ __|  __  | |/ _` |/ _ \
 | |__| | |   | | | (_) | (_|  __/\__ \__ \ |  | | | (_| |  __/
  \____/|_|   |_|  \___/ \___\___||___/___/_|  |_|_|\__,_|\___|

                                            Coder: un4ckn0wl3z
                                            Website: https://unknownclub.net
                                                               
)" << std::endl;
}

int wmain(int argc, const wchar_t* argv[])
{
	Banner();

	if (argc < 3)
	{
		printf("Usage: %ws [<enable|disable>] [<pid|process_name>]\n", argv[0]);
		return 0;
	}

	if (_wcsicmp(argv[1], L"disable") == 0)
	{
		TargetProcessPid = 0;
		TargetProcessName[0] = 0;
		printf("[+] ::%ws:: revealed process %ws!\n", argv[0], argv[2]);
	}
	else if (_wcsicmp(argv[1], L"enable") == 0)
	{
		auto targetProcess = _wtoi(argv[2]);
		if (targetProcess == 0)
		{
			wcscpy_s(TargetProcessName, argv[2]);
		}
		else
		{
			TargetProcessPid = targetProcess;
		}
		printf("[+] ::%ws:: hidden process %ws!\n", argv[0], argv[2]);

	}
	else
	{
		printf("[-] ::%ws:: invalid argument!\n", argv[0]);
	}

	return 0;
}