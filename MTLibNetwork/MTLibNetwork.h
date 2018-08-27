#pragma once

#include <vector>
#include <string>

typedef void(__stdcall *CallVerifyCallback)(int err_no, std::string err_msg, double similarity, unsigned long userdata);
int __stdcall MTLibCallVerify(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string macId, std::string registeredNo,
	std::string idcardId, std::string idcardIssuedate,
	std::vector<unsigned char> idcardPhoto, std::vector<unsigned char> verifyPhotos[], int verifyPhotoNum,
	CallVerifyCallback callverifyCB, unsigned long userdata);

typedef void(__stdcall *CallRegisterCallback)(int err_no, std::string err_msg, std::string RegisteredNo, unsigned long userdata);
int __stdcall MTLibCallRegister(std::string url, std::string appId, std::string apiKey, std::string secretKey, std::string uuid,
	std::string productsn, std::string macId,
	CallRegisterCallback callregisterCB, unsigned long userdata);	

typedef void(__stdcall *TestUrlCallback)(int err_no, unsigned long userdata);
int __stdcall MTLibTestUrl(std::string url, TestUrlCallback testurlCB, unsigned long userdata);
