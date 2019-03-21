// utility_funcs.h头文件
//

#pragma once
#include <fstream>


std::string ExtractFilePath(const std::string& szFile);
void KillProcess(TCHAR target[]);
DWORD GetProcessPID(TCHAR target[]);
int CompareVersion(std::string v1, std::string v2); // 1:v1版本高, 0:版本相同, -1:v2版本高
BOOL CopyFolder(LPCTSTR pstrSrcFolder, LPCTSTR pstrDstFolder);//拷贝文件夹