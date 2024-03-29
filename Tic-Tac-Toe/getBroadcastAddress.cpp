/*------------------------------------------------------------------------------
* getBroadcastAddress - Retrieves the network settings for the primary network
adapter and returns the associated broadcast address
(returns a pointer to a null-terminated, dotted-decimal string).
*------------------------------------------------------------------------------*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <iphlpapi.h>

#pragma comment (lib, "iphlpapi.lib")

char* getBroadcastAddress()
{
	// NOTE: This code only returns the broadcast address for the first adapter
	//       found that has a non-zero IP address associated

	// Declare and initialize variables
#define MAXSIZE 16
	static char Address[MAXSIZE] = "";
	unsigned long IPAddress = 0;
	unsigned long subnetMask = 0;
	unsigned long broadcastAddress = 0;

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		std::cout << "Error allocating memory needed by getBroadcastAddress" << std::endl;
		return NULL;
	}
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		// Release initial allocation of heap memory and request more appropriately sized memory
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);
		if (pAdapterInfo == NULL) {
			std::cout << "Error allocating memory needed by getBroadcastAddress" << std::endl;
			return NULL;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
		pAdapter = pAdapterInfo;
		// Loop through all of the adapters looking for the 1st one with non-zero IPAddress
		bool found = false;
		while (pAdapter && !found) {
			IPAddress = inet_addr(pAdapter->IpAddressList.IpAddress.String);
			if (IPAddress > 0) {
				found = true;
				subnetMask = inet_addr(pAdapter->IpAddressList.IpMask.String);
				unsigned long BcastMask = subnetMask ^ 0xffffffff;
				broadcastAddress = IPAddress | BcastMask;
				in_addr ip_addr;
				ip_addr.S_un.S_addr = broadcastAddress;
				strcpy_s(Address, MAXSIZE, inet_ntoa(ip_addr));
			}
			pAdapter = pAdapter->Next;
		}
		if (!found) {
			std::cout << "Error in getBroadcastAddress: No active netwrok adapter found" << std::endl;
			strcpy_s(Address, MAXSIZE, "255.255.255.255");
		}
	}
	else {
		std::cout << "Error in getBroadcastAddress: GetAdaptersInfo failed with error: " << dwRetVal << std::endl;
	}
	if (pAdapterInfo)
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);

	return Address;
}