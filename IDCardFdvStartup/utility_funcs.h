// utility_funcs.hͷ�ļ�
//

#pragma once
#include <fstream>


std::string ExtractFilePath(const std::string& szFile);
void KillProcess(TCHAR target[]);
DWORD GetProcessPID(TCHAR target[]);
int CompareVersion(std::string v1, std::string v2); // 1:v1�汾��, 0:�汾��ͬ, -1:v2�汾��
BOOL CopyFolder(LPCTSTR pstrSrcFolder, LPCTSTR pstrDstFolder);//�����ļ���