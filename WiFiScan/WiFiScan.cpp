// Simple Wi-Fi scan console application using Native Wi-Fi APIs
// 
// Payam Torab, July 2023

#include "pch.h"
#include <iostream>

#include <Windows.h>
#include <wlanapi.h>

#pragma comment(lib, "wlanapi.lib")

int main() {
    DWORD dwResult;
    HANDLE hClient = NULL;
    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_BSS_LIST pBssList = NULL;

    // initialize the Wi-Fi API
    dwResult = WlanOpenHandle(2, NULL, &dwResult, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        std::cout << "Failed to open WLAN handle: " << dwResult << std::endl;
        return 1;
    }

    // enumerate Wi-Fi interfaces
    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        std::cout << "Failed to enumerate interfaces: " << dwResult << std::endl;
        WlanCloseHandle(hClient, NULL);
        return 1;
    }

    // initiate a scan on the first interface 
    dwResult = WlanScan(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid, NULL, NULL, NULL);
    if (dwResult != ERROR_SUCCESS) {
        std::cout << "Failed to initiate scan, error code: " << dwResult << std::endl;
        WlanFreeMemory(pIfList);
        WlanCloseHandle(hClient, NULL);
        return 1;
    }

    // simple wait instead of registering for a scan complete notification
    // Wireless network drivers that meet Windows logo requirements are
    // required to complete a WlanScan function request in 4 seconds.
    // https://learn.microsoft.com/en-us/windows/win32/api/wlanapi/nf-wlanapi-wlanscan
    Sleep(8000);

    // retrieve the scanned networks
    dwResult = WlanGetNetworkBssList(hClient, &pIfList->InterfaceInfo[0].InterfaceGuid,
        NULL, dot11_BSS_type_any, false, NULL, &pBssList);

    if (dwResult == ERROR_SUCCESS) {
        for (DWORD i = 0; i < pBssList->dwNumberOfItems; i++) {
            const WLAN_BSS_ENTRY bssEntry = pBssList->wlanBssEntries[i];
            std::cout << "SSID: " << bssEntry.dot11Ssid.ucSSID << std::endl;
            std::cout << "BSSID: "; CHAR string[2 + 1];
            for (DWORD j = 0; j < sizeof(bssEntry.dot11Bssid); j++) {
                _snprintf_s(string, sizeof(string), "%02X", bssEntry.dot11Bssid[j]);
                std::cout << string << (j < (sizeof(bssEntry.dot11Bssid) - 1) ? ":" : "\n");
            }
            std::cout << "Scan radio identifier: " << bssEntry.uPhyId << std::endl;
            std::cout << "RSSI: " << bssEntry.lRssi << " dBm" << std::endl;
            //std::cout << "Link quality: " << bssEntry.uLinkQuality << std::endl;
            std::cout << "PHY type: " << bssEntry.dot11BssPhyType << std::endl;
            std::cout << "Operating in designated regulatory domain: " <<
                (bssEntry.bInRegDomain ? "True" : "False") << std::endl;
            std::cout << "Beacon interval: " << bssEntry.usBeaconPeriod * 1.024 << " msec" << std::endl;
            //std::cout << "Beacon or Probe Response timestamp: " << bssEntry.ullTimestamp << std::endl;
            //std::cout << "Host timestamp: " << bssEntry.ullHostTimestamp << std::endl;
            std::cout << "Capability information bitmap: " << bssEntry.usCapabilityInformation << std::endl;
            std::cout << "Channel: " << bssEntry.ulChCenterFrequency / 1000.0 << " MHz" << std::endl;
            std::cout << "Rate set in Mbps: ";
            DWORD numberOfRates = bssEntry.wlanRateSet.uRateSetLength / sizeof(USHORT);
            for (DWORD j = 0; j < numberOfRates; j++) {
                std::cout << (bssEntry.wlanRateSet.usRateSet[j] & 0x7FFF) * 0.5 <<
                    ((j < numberOfRates - 1) ? ", " : "\n");
            }
            std::cout << "IE buffer offset: " << bssEntry.ulIeOffset << std::endl;
            std::cout << "IE buffer size: " << bssEntry.ulIeSize << std::endl;

            // IE retrieval fails; IE blob is absent based on memory dumps around &bssEntry
            // const PBYTE ieBuffer = (const PBYTE)((const PBYTE)(&bssEntry) + bssEntry.ulIeOffset);
            // for (DWORD j = 0; j < bssEntry.ulIeSize; j++) {
            //    printf("%02X ", ieBuffer[j]);
            // }
            // std::cout << std::endl;
       
            std::cout << std::endl;

        }
        WlanFreeMemory(pBssList);
    }

    // clean up
    WlanFreeMemory(pIfList);
    WlanCloseHandle(hClient, NULL);

    return 0;
}