// utility_funcs.hÍ·ÎÄ¼þ
//

#pragma once
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <Dshow.h>

IplImage *BMP2Ipl(unsigned char *src, int FileSize);
int getDeviceIndex(std::string vid, std::string pid);
void MatAlphaBlend(cv::Mat &dst, cv::Mat &scr);
void WaitObjectAndMsg(HANDLE hEvent, DWORD dwMilliseconds);