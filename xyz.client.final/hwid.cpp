#include <string>
#include <Windows.h>
#include "encrypt\md5.h"
#include "encrypt\xor.h"
#include <conio.h>
#include <sstream>
#include <string.h>
#include <iostream>
#include "hwid.h"

using namespace std;
string HWID = _xor_("a");

string getProductId() {
    HKEY keyHandle;
    WCHAR rgValue[1024];
    WCHAR fnlRes[1024];
    DWORD size1;
    DWORD Type;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &keyHandle) == ERROR_SUCCESS)
    {
        size1 = 1023;
        RegQueryValueExW(keyHandle, L"Productid", NULL, &Type, (LPBYTE)rgValue, &size1);
        RegCloseKey(keyHandle);
        wstring rgValueCh = rgValue;
        string convertRgVal(rgValueCh.begin(), rgValueCh.end());
        return convertRgVal;
    }
}
string getCurrentBuild() {
    HKEY keyHandle;
    WCHAR rgValue[1024];
    WCHAR fnlRes[1024];
    DWORD size1;
    DWORD Type;

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &keyHandle) == ERROR_SUCCESS)
    {
        size1 = 1023;
        RegQueryValueExW(keyHandle, L"CurrentBuild", NULL, &Type, (LPBYTE)rgValue, &size1);
        RegCloseKey(keyHandle);
        wstring rgValueCh = rgValue;
        string convertRgVal(rgValueCh.begin(), rgValueCh.end());
        return convertRgVal;
    }
}
string getMachineId()
{
    // LPCTSTR szHD = "C:\\";  // ERROR
    string ss;
    ss = "Err_StringIsNull";
    UCHAR szFileSys[255],
        szVolNameBuff[255];
    DWORD dwSerial;
    DWORD dwMFL;
    DWORD dwSysFlags;
    int error = 0;

    bool success = GetVolumeInformation(LPCTSTR("C:\\"), (LPTSTR)szVolNameBuff,
        255, &dwSerial,
        &dwMFL, &dwSysFlags,
        (LPTSTR)szFileSys,
        255);
    if (!success) {
        ss = "Err_Not_Elevated";
    }
    std::stringstream errorStream;
    errorStream << dwSerial;
    return string(errorStream.str().c_str());
}
string getHWinfo64() {
    HW_PROFILE_INFO hwProfileInfo;
    if (GetCurrentHwProfile(&hwProfileInfo)) {

        string a, b, c, d, e, f, g, h;
        string hwid = hwProfileInfo.szHwProfileGuid;

        a = hwid.substr(0, 3); b = hwid.substr(2, 3);
        c = hwid.substr(4, 3); d = hwid.substr(6, 3);
        e = hwid.substr(8, 3); f = hwid.substr(10, 3);
        g = hwid.substr(12, 3); h = hwid.substr(14, 3);

        string mixedHwid = a + c + b + a + e + f + h + g + b + e + c + a + d + f + d;

        mixedHwid.erase(std::remove(mixedHwid.begin(), mixedHwid.end(), '-'), mixedHwid.end());
        mixedHwid.erase(std::remove(mixedHwid.begin(), mixedHwid.end(), '{'), mixedHwid.end());
        mixedHwid.erase(std::remove(mixedHwid.begin(), mixedHwid.end(), '}'), mixedHwid.end());

        string hashedHwid = md5(mixedHwid);

        string prod = getProductId();
        string j, k, l, m;

        j = prod.substr(0, 3); k = prod.substr(1, 3);
        l = prod.substr(2, 3); m = prod.substr(3, 3);

        string mixedProduct = j + k + m + j + k + l + l + m + k + j;

        mixedProduct.erase(std::remove(mixedProduct.begin(), mixedProduct.end(), '-'), mixedProduct.end());
        mixedProduct.erase(std::remove(mixedProduct.begin(), mixedProduct.end(), '{'), mixedProduct.end());
        mixedProduct.erase(std::remove(mixedProduct.begin(), mixedProduct.end(), '}'), mixedProduct.end());

        string hashedProduct = md5(mixedProduct);

        string cubu = getCurrentBuild();
        string hashedCubu = md5(cubu);

        string mcid = getMachineId();
        string x, y, z;

        x = mcid.substr(0, 3); y = mcid.substr(2, 3);
        z = mcid.substr(4, 3);

        string machineMixer = x + y + y + x + z + y + x;

        machineMixer.erase(std::remove(machineMixer.begin(), machineMixer.end(), '-'), machineMixer.end());
        machineMixer.erase(std::remove(machineMixer.begin(), machineMixer.end(), '{'), machineMixer.end());
        machineMixer.erase(std::remove(machineMixer.begin(), machineMixer.end(), '}'), machineMixer.end());

        string hashedMachineId = md5(machineMixer);

        HWID = md5(md5(hashedCubu + hashedMachineId + hashedHwid + hashedMachineId + hashedProduct + hashedCubu));

        return HWID;
    }
}