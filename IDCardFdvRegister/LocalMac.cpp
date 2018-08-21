
#include "stdafx.h"
#include <atlbase.h>
#include <atlconv.h>
#include "iphlpapi.h"
#include "LocalMac.h"

#pragma comment ( lib, "Iphlpapi.lib")
BOOL IsLocalAdapter(char *pAdapterName);

void GetLocalMAC(char *buf, size_t buf_size)
{
	IP_ADAPTER_INFO *IpAdaptersInfo = NULL;
	IP_ADAPTER_INFO *IpAdaptersInfoHead = NULL;

	IpAdaptersInfo = (IP_ADAPTER_INFO *)GlobalAlloc(GPTR, sizeof(IP_ADAPTER_INFO));

	if (IpAdaptersInfo == NULL)
	{
		return;
	}

	DWORD dwDataSize = sizeof(IP_ADAPTER_INFO);
	DWORD dwRetVal = GetAdaptersInfo(IpAdaptersInfo, &dwDataSize);

	if (ERROR_SUCCESS != dwRetVal)
	{
		GlobalFree(IpAdaptersInfo);
		IpAdaptersInfo = NULL;

		if (ERROR_BUFFER_OVERFLOW == dwRetVal)
		{
			IpAdaptersInfo = (IP_ADAPTER_INFO *)GlobalAlloc(GPTR, dwDataSize);
			if (IpAdaptersInfo == NULL)
			{
				return;
			}
			if (ERROR_SUCCESS != GetAdaptersInfo(IpAdaptersInfo, &dwDataSize))
			{
				GlobalFree(IpAdaptersInfo);
				return;
			}

		}
		else
		{
			return;
		}

	}

	//Save the head pointer of IP_ADAPTER_INFO structures list.
	IpAdaptersInfoHead = IpAdaptersInfo;

	do {
		if (IsLocalAdapter(IpAdaptersInfo->AdapterName))
		{
			sprintf_s(buf, buf_size, "%02X:%02X:%02X:%02X:%02X:%02X",
				IpAdaptersInfo->Address[0],
				IpAdaptersInfo->Address[1],
				IpAdaptersInfo->Address[2],
				IpAdaptersInfo->Address[3],
				IpAdaptersInfo->Address[4],
				IpAdaptersInfo->Address[5]);
			//
			//printf(buf); printf("\n");
			break;
		}

		IpAdaptersInfo = IpAdaptersInfo->Next;

	} while (IpAdaptersInfo);

	if (IpAdaptersInfoHead != NULL)
	{
		GlobalFree(IpAdaptersInfoHead);
	}
}


BOOL IsLocalAdapter(char *pAdapterName)
{
	BOOL ret_value = FALSE;

#define NET_CARD_KEY _T("System\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}")
	char szDataBuf[MAX_PATH + 1];
	DWORD dwDataLen = MAX_PATH;
	DWORD dwType = REG_SZ;
	HKEY hNetKey = NULL;
	HKEY hLocalNet = NULL;

	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, NET_CARD_KEY, 0, KEY_READ, &hNetKey))
		return FALSE;
	wsprintf(szDataBuf, "%s\\Connection", pAdapterName);
	if (ERROR_SUCCESS != RegOpenKeyEx(hNetKey, szDataBuf, 0, KEY_READ, &hLocalNet))
	{
		RegCloseKey(hNetKey);
		return FALSE;
	}
	//	if (ERROR_SUCCESS != RegQueryValueEx(hLocalNet, "MediaSubType", 0, &dwType, (BYTE *)szDataBuf, &dwDataLen))
	//	{
	//		goto ret;
	//	}
	//	if (*((DWORD *)szDataBuf) != 0x01)
	//		goto ret;
	dwDataLen = MAX_PATH;
	if (ERROR_SUCCESS != RegQueryValueEx(hLocalNet, "PnpInstanceID", 0, &dwType, (BYTE *)szDataBuf, &dwDataLen))
	{
		goto ret;
	}
	if (strncmp(szDataBuf, "PCI", strlen("PCI")))
		goto ret;

	ret_value = TRUE;

ret:
	RegCloseKey(hLocalNet);
	RegCloseKey(hNetKey);

	return ret_value;
}