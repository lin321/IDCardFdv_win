#pragma once


typedef void(__stdcall *MTLibFaceImageCallBack)(HWND hWnd, BSTR imgBase64, ULONG_PTR userdata);
typedef void(__stdcall *MTLibFaceResultCallBack)(HWND hWnd, LONG result, ULONG_PTR userdata);

int __stdcall MTLibLoadCamera();
int __stdcall MTLibSetLiveChecker(ULONG_PTR checker);
int __stdcall MTLibOpenCamera(HWND hWnd,
					MTLibFaceImageCallBack fiCB, ULONG_PTR fi_userdata,
					MTLibFaceResultCallBack frCB, ULONG_PTR fr_userdata);
int __stdcall MTLibCloseCamera();
int __stdcall MTLibUnloadCamera();