#pragma once

#include <vector>
#include <string>

#ifdef _WIN64
#define MTLIBPTR unsigned __int64
#else
#define MTLIBPTR unsigned long
#endif

// 超时或其他网络错误
#define MTLIBNETWORK_NETWORK_ERROR	INT_MIN		

typedef void(__stdcall *CallVerifyCallback)(int err_no, std::string err_msg, double similarity, MTLIBPTR userdata);

int __stdcall MTLibCallVerify_Image(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	std::vector<unsigned char> &idcardPhoto, std::vector<unsigned char> verifyPhotos[], int verifyPhotoNum,
	CallVerifyCallback callverifyCB, MTLIBPTR userdata, int timeout);

int __stdcall MTLibCallVerify(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	std::string idcardPhoto, std::vector < std::string> &verifyPhotos, int verifyPhotoNum,
	CallVerifyCallback callverifyCB, MTLIBPTR userdata, int timeout);

typedef void(__stdcall *CallRegisterCallback)(int err_no, std::string err_msg, std::string RegisteredNo, MTLIBPTR userdata);
int __stdcall MTLibCallRegister(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string productsn, std::string macId,
	CallRegisterCallback callregisterCB, MTLIBPTR userdata, int timeout);

typedef void(__stdcall *TestUrlCallback)(int err_no, MTLIBPTR userdata);
int __stdcall MTLibTestUrl(std::string url, TestUrlCallback testurlCB, MTLIBPTR userdata, int timeout);
