// utility_funcs.hÍ·ÎÄ¼þ
//

#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <Dshow.h>
#include <fstream>

#ifdef _WIN64
#define WORK_PTR unsigned __int64
#else
#define WORK_PTR unsigned long
#endif

IplImage *BMP2Ipl(unsigned char *src, int FileSize);
int getDeviceIndex(std::string vid, std::string pid);
void MatAlphaBlend(cv::Mat &dst, cv::Mat &scr);
void WaitObjectAndMsg(HANDLE hEvent, DWORD dwMilliseconds);
std::string ExtractFilePath(const std::string& szFile);

// for CHtmlView IE core
BOOL RaisePrivileges();
void SetIECoreVersion();