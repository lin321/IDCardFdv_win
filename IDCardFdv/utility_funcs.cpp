// utility_funcs.cpp : 实现文件
//
#include "stdafx.h"
#include "utility_funcs.h"
#include <algorithm>

using namespace std;

//文件头结构体
#pragma pack(push)
#pragma pack(2)  //两字节对齐，否则bmp_fileheader会占16Byte
//bmp文件头
struct bmp_fileheader
{
	unsigned short   bfType; //文件标识
	unsigned long	 bfSize; //文件大小
	unsigned long	 bfReserved1; //保留
	unsigned long    bfOffBits;//数据偏移
};

//bmp信息头
struct bmp_infoheader
{
	unsigned long    biSize;//信息头长度
	unsigned long    biWidth;//宽度
	unsigned long    biHeight;//高度
	unsigned short   biPlanes;//柱面数=1
	unsigned short   biBitCount;//像素位数
	unsigned long    biCompression;//压缩说明
	unsigned long    biSizeImage;//位图数据的大小
	unsigned long    biXPelsPerMeter;//水平分辨率
	unsigned long    biYPelsPerMeter;//垂直分辨率
	unsigned long    biClrUsed;//使用的颜色数
	unsigned long    biClrImportant;//重要的颜色数
};

#pragma pack(pop)

using namespace cv;
IplImage *BMP2Ipl(unsigned char *src, int FileSize)
{
	bmp_fileheader *Fileheader = (bmp_fileheader *)src;
	//检查错误
	assert(FileSize == Fileheader->bfSize);
	//判断是否是位图
	if (Fileheader->bfType != 0x4d42)
		return NULL;
	//开始处理信息头
	bmp_infoheader *bmpheader = (bmp_infoheader *)(src + sizeof(bmp_fileheader));
	unsigned int width = bmpheader->biWidth;//宽度
	unsigned int height = bmpheader->biHeight;//高度
	unsigned int hSize = bmpheader->biSize;//信息头长度

	if (bmpheader->biBitCount < 24)
		return NULL;//支持真色彩32位

	if (bmpheader->biCompression != 0)
		return NULL;//不支持压缩算法

					//数据大小
	//unsigned int dataByteSize = bmpheader->biSizeImage;//实际位图数据大小  (读卡器读出数据可能出现biSizeImage是0的情况)
	unsigned int dataByteSize = FileSize - Fileheader->bfOffBits; //实际位图数据大小
	unsigned int rowByteSize = dataByteSize / height;//对齐的每行数据

	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, bmpheader->biBitCount / 8);
	//拷贝实际数据无颜色表
	//img->origin = IPL_ORIGIN_BL;
	//memcpy(img->imageData, src + Fileheader->bfOffBits, FileSize - Fileheader->bfOffBits);

	//*
	img->origin = IPL_ORIGIN_TL;
	unsigned char *gc = src + FileSize - rowByteSize;
	char *p = img->imageData;
	for (unsigned int i = 0; i<height; ++i)
	{
		memcpy(p, gc, rowByteSize);
		p += rowByteSize;
		gc -= rowByteSize;
	}/**/

	
	return img;

}


int getDeviceIndex(std::string vid, std::string pid)
{
	std::string str_vid = "VID_" + vid;
	std::string str_pid = "PID_" + pid;
	transform(str_vid.begin(), str_vid.end(), str_vid.begin(), ::tolower);
	transform(str_pid.begin(), str_pid.end(), str_pid.begin(), ::tolower);
	int vidfind = -1;
	int pidfind = -1;
	int retIdx = -1;

	ICreateDevEnum *pDevEnum = NULL;
	IEnumMoniker *pEnum = NULL;
	int deviceCounter = 0;
	CoInitialize(NULL);
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
		CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
		reinterpret_cast<void**>(&pDevEnum));


	if (SUCCEEDED(hr))
	{
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator(
			CLSID_VideoInputDeviceCategory,
			&pEnum, 0);

		if (hr == S_OK) {

			//if (!silent)printf("SETUP: Looking For Capture Devices\n");
			IMoniker *pMoniker = NULL;

			while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {

				IPropertyBag *pPropBag;
				hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
					(void**)(&pPropBag));

				if (FAILED(hr)) {
					pMoniker->Release();
					continue;  // Skip this one, maybe the next one will work.
				}


				// Find the DevicePath, description or friendly name.
				VARIANT varName;
				VariantInit(&varName);
				//hr = pPropBag->Read(L"Description", &varName, 0);

				//if (FAILED(hr)) hr = pPropBag->Read(L"FriendlyName", &varName, 0);

				hr = pPropBag->Read(L"DevicePath", &varName, 0);

				if (SUCCEEDED(hr)) {

					//hr = pPropBag->Read(L"FriendlyName", &varName, 0);

					_bstr_t bstr_t(varName.bstrVal);
					std::string strVal(bstr_t);
					
					vidfind = (int)strVal.find(str_vid);
					pidfind = (int)strVal.find(str_pid);

					printf("SETUP: %i) %s \n", deviceCounter, strVal.c_str());
				}

				pPropBag->Release();
				pPropBag = NULL;

				pMoniker->Release();
				pMoniker = NULL;

				if (vidfind >= 0 && pidfind >= 0) {
					retIdx = deviceCounter;
					break;
				}

				deviceCounter++;
			}

			pDevEnum->Release();
			pDevEnum = NULL;

			pEnum->Release();
			pEnum = NULL;
		}

		//printf("SETUP: %i Device(s) found\n\n", deviceCounter);
	}

	//return deviceCounter;
	return retIdx;
}

void MatAlphaBlend(cv::Mat &dst, cv::Mat &scr)
{
	if (dst.channels() != 3 || scr.channels() != 4)
		return;
	
	std::vector<cv::Mat>scr_channels;
	std::vector<cv::Mat>dstt_channels;
	split(scr, scr_channels);
	split(dst, dstt_channels);
	CV_Assert(scr_channels.size() == 4 && dstt_channels.size() == 3);

	for (int i = 0; i < 3; i++)
	{
		dstt_channels[i] = dstt_channels[i].mul(255.0 - scr_channels[3], 1.0 / 255.0);
		dstt_channels[i] += scr_channels[i].mul(scr_channels[3], 1.0 / 255.0);
	}
	merge(dstt_channels, dst);
}

// function:阻塞并等待,但处理消息
void WaitObjectAndMsg(HANDLE hEvent, DWORD dwMilliseconds) {
	BOOL bWait = TRUE;
	DWORD dwResult = 0;

	while (bWait)
	{
		DWORD dwResult = ::MsgWaitForMultipleObjects(1, &hEvent, FALSE, dwMilliseconds, QS_ALLINPUT);

		if (WAIT_OBJECT_0 == dwResult) {
			break;
		}
		else {
			MSG msg;
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			DispatchMessage(&msg);
		}
	}
}

string ExtractFilePath(const string& szFile)
{
	if (szFile == "")
		return "";

	size_t idx = szFile.find_last_of("\\:");

	if (-1 == idx)
		return "";
	return string(szFile.begin(), szFile.begin() + idx + 1);
}

BOOL RaisePrivileges()
{
	HANDLE TokenHandle;
	TOKEN_PRIVILEGES t_privileges = { 0 };
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle))
	{
		return FALSE;
	}
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &t_privileges.Privileges[0].Luid))
	{
		return TRUE;
	}
	t_privileges.PrivilegeCount = 1;
	t_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(TokenHandle, FALSE, &t_privileges, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		CloseHandle(TokenHandle);
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

void SetIECoreVersion()
{
	const char* path = "SOFTWARE\\Microsoft\\Internet Explorer\\MAIN\\FeatureControl\\FEATURE_BROWSER_EMULATION";
	const char* valueName = "IDCardFdv.exe";
	long version = 11000;
	char err[1024];
	HKEY hKey;
	DWORD dwDisposition;
	long ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, REG_LEGAL_OPTION, &hKey);
	if (ret != ERROR_SUCCESS)
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, ret, NULL, err, sizeof(err), NULL);
		ret = RegCreateKeyEx(HKEY_LOCAL_MACHINE, path, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition);
		if (ret != ERROR_SUCCESS)
		{
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, ret, NULL, err, sizeof(err), NULL);
			return;
		}
	}
	ret = RegSetValueEx(hKey, valueName, NULL, REG_DWORD, (BYTE*)&version, sizeof(version));
	if (ret != ERROR_SUCCESS)
		return;
}

