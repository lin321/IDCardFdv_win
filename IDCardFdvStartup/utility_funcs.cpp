// utility_funcs.cpp : 实现文件
//
#include "stdafx.h"
#include "utility_funcs.h"
#include "psapi.h"

using namespace std;

string ExtractFilePath(const string& szFile)
{
	if (szFile == "")
		return "";

	size_t idx = szFile.find_last_of("\\:");

	if (-1 == idx)
		return "";
	return string(szFile.begin(), szFile.begin() + idx + 1);
}

//TerminateProcess因权限问题可能导致无效，目前避免使用
void KillProcess(TCHAR target[])
{
	DWORD aps[1024], cbNeeded, cbNeeded2, i;
	TCHAR buffer[256];
	BOOL killed;
	HANDLE hProcess;
	HMODULE hModule;

	killed = FALSE;

	EnumProcesses(aps, sizeof(aps), &cbNeeded);
	for (i = 0; i < cbNeeded / sizeof(DWORD); ++i)
	{
		if (!(hProcess = OpenProcess(PROCESS_TERMINATE |
			PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ, FALSE, aps[i])))
			continue;
		if (!EnumProcessModules(hProcess, &hModule,
			sizeof(hModule), &cbNeeded2))
			continue;
		GetModuleBaseNameA(hProcess, hModule, buffer, 256);
		if (lstrcmpi(buffer, target) == 0)
		{
			if (TerminateProcess(hProcess, 0))
				killed = TRUE;
			CloseHandle(hProcess);
			break;
		}
		CloseHandle(hProcess);
	}
}

DWORD GetProcessPID(TCHAR target[])
{
	DWORD aps[1024], cbNeeded, cbNeeded2, i;
	TCHAR buffer[256];
	HANDLE hProcess;
	HMODULE hModule;

	EnumProcesses(aps, sizeof(aps), &cbNeeded);
	for (i = 0; i < cbNeeded / sizeof(DWORD); ++i)
	{
		if (!(hProcess = OpenProcess(PROCESS_TERMINATE |
			PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ, FALSE, aps[i])))
			continue;
		if (!EnumProcessModules(hProcess, &hModule,
			sizeof(hModule), &cbNeeded2))
			continue;
		GetModuleBaseNameA(hProcess, hModule, buffer, 256);
		if (lstrcmpi(buffer, target) == 0)
		{ 
			CloseHandle(hProcess);
			return aps[i];
		}
		CloseHandle(hProcess);
	}

	return 0;
}

bool getVerNum(string str, int &num, int &endidx)
{
	const char* cstr = str.c_str();
	string numstr = "";
	bool isnumstr = false;
	int idx = 0;
	while (idx < str.length()) {
		char c = cstr[idx];
		if (c > '0' && c < '9') {
			isnumstr = true;
			numstr += c;
		}
		else {
			if(isnumstr)
				break;
		}
		idx++;
	}

	endidx = idx;
	if (!numstr.empty()) {
		num = ::atoi(numstr.c_str());
		return true;
	}
	
	return false;
}

int CompareVersion(string v1, string v2)
{
	int numV1 = 0, numV2 = 0;
	int nextV1 = 0, nextV2 = 0;
	string v1_ = v1;
	string v2_ = v2;

	while (1) {
		bool r1 = getVerNum(v1_, numV1, nextV1);
		bool r2 = getVerNum(v2_, numV2, nextV2);
		if (!r1 && !r2)
			return 0;
		else if (!r1)
			return -1;
		else if (!r2)
			return 1;

		if (numV1 > numV2)
			return 1;
		else if (numV1 < numV2)
			return -1;

		if (nextV1 < v1_.length())
			v1_ = v1_.substr(nextV1);
		else
			v1_ = "";
		if (nextV2 < v2_.length())
			v2_ = v2_.substr(nextV2);
		else
			v2_ = "";
	}
}

BOOL CopyFolder(LPCTSTR pstrSrcFolder, LPCTSTR pstrDstFolder)//拷贝文件夹
{
	if ((NULL == pstrSrcFolder) || (NULL == pstrSrcFolder))
		return FALSE;

	size_t iSrcPathLen = _tcslen(pstrSrcFolder);
	size_t iDstPathLen = _tcslen(pstrDstFolder);
	if ((iSrcPathLen >= MAX_PATH) || (iDstPathLen >= MAX_PATH))
		return FALSE;

	/*确保源目录的路径以2个\0结尾*/
	TCHAR tczSrcFolder[MAX_PATH + 1];
	ZeroMemory(tczSrcFolder, (MAX_PATH + 1) * sizeof(TCHAR));
	strcpy_s(tczSrcFolder, pstrSrcFolder);
	tczSrcFolder[iSrcPathLen] = _T('\0');
	tczSrcFolder[iSrcPathLen + 1] = _T('\0');

	/*确保目的目录的路径以2个\0结尾*/
	TCHAR tczDstFolder[MAX_PATH + 1];
	ZeroMemory(tczDstFolder, (MAX_PATH + 1) * sizeof(TCHAR));
	strcpy_s(tczDstFolder, pstrDstFolder);
	tczDstFolder[iDstPathLen] = _T('\0');
	tczDstFolder[iDstPathLen + 1] = _T('\0');

	SHFILEOPSTRUCT FileOp;
	SecureZeroMemory((void*)&FileOp, sizeof(SHFILEOPSTRUCT));
	FileOp.fFlags |= FOF_SILENT;        //不显示进度
	FileOp.fFlags |= FOF_NOCONFIRMATION; //不进行确认
	FileOp.fFlags |= FOF_MULTIDESTFILES; //多文件（不包含源的根文件夹）
	FileOp.fFlags |= FOF_FILESONLY;
	FileOp.hNameMappings = NULL; //文件映射
	FileOp.hwnd = NULL; //消息发送的窗口句柄；
	FileOp.lpszProgressTitle = NULL; //文件操作进度窗口标题 
	FileOp.pFrom = tczSrcFolder; //源文件及路径 
	FileOp.pTo = tczDstFolder; //目标文件及路径 
	FileOp.wFunc = FO_COPY; //操作类型 

	return SHFileOperation(&FileOp) == 0;
}