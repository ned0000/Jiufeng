/**
 *  @file jf_host.c
 *
 *  @brief get host information implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#if defined(WINDOWS)

#elif defined(LINUX)
    #include <errno.h>
    #include <sys/utsname.h>
    #include <sys/socket.h>    
    #include <sys/types.h>
    #include <sys/select.h>
    #include <sys/un.h>
    #include <sys/time.h>
    #include <sys/ioctl.h>
    #include <unistd.h>
    #include <net/if.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_host.h"
#include "jf_string.h"
#include "ifmgmt.h"

/* --- private data/data structure section --------------------------------- */
#if defined(WINDOWS)
    typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
#endif
#define SM_SERVERR2 89
#define BUFSIZE 80

/* --- private funciton routines ------------------------------------------- */

static u32 _getHostVersion(jf_host_info_t * pjhi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;
    struct utsname name;

    nRet = uname(&name);
    if (nRet == 0)
    {
        ol_sprintf(pjhi->jhi_strHostName, "%s", name.nodename);
        ol_sprintf(
            pjhi->jhi_strOSName, "%s %s %s",
            name.sysname, name.release, name.machine);
    }
    else
    {
        ol_strcpy(pjhi->jhi_strHostName, jf_string_getStringUnknown());
        ol_strcpy(pjhi->jhi_strOSName, jf_string_getStringUnknown());
    }
#elif defined(WINDOWS)
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    PGNSI pGNSI;
    BOOL bOsVersionInfoEx;
    u8 u8HostName[MAX_COMPUTERNAME_LENGTH + 1];
    u32 u32HostNameLen;

    /*get host name*/
    memset(u8HostName, 0, MAX_COMPUTERNAME_LENGTH + 1);
    u32HostNameLen = MAX_COMPUTERNAME_LENGTH + 1;
    if (GetComputerName(u8HostName, &u32HostNameLen))
    {
        ol_strncpy(
            pjhi->jhi_strHostName, u8HostName, JF_HOST_MAX_HOST_NAME_LEN - 1);
    }
    else
    {
        ol_strcpy(pjhi->jhi_strHostName, jf_string_getStringUnknown());
    }

    /*get OS name*/
    ZeroMemory(&si, sizeof(SYSTEM_INFO));
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    
    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (! (bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)))
    {
        osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
        if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
            return FALSE;
    }
    
    // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
    pGNSI = (PGNSI) GetProcAddress(
        GetModuleHandle(TEXT("kernel32.dll")), 
        "GetNativeSystemInfo");
    if(NULL != pGNSI)
        pGNSI(&si);
    else
        GetSystemInfo(&si);
    
    switch (osvi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
        // Test for the Windows NT product family.
        // Test for the specific product.
        if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
        {
            if (osvi.wProductType == VER_NT_WORKSTATION)
                ol_strcat(pjhi->jhi_strOSName, "Microsoft Windows Vista ");
            else if (osvi.wProductType == VER_NT_SERVER)
                ol_strcat(pjhi->jhi_strOSName, "Windows Server 2008 ");
            else
                ol_strcat(pjhi->jhi_strOSName, "Windows Server \"Longhorn\" ");
        }
    
        if ((osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) ||
            (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0))
        {
            if (GetSystemMetrics(SM_SERVERR2))
                ol_strcat(
                    pjhi->jhi_strOSName, "Microsoft Windows Server 2003 \"R2\" ");
            else if (osvi.wProductType == VER_NT_WORKSTATION &&
                     si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            {
                ol_strcat(
                    pjhi->jhi_strOSName,
                    "Microsoft Windows XP Professional x64 Edition ");
            }
            else
                ol_strcat(pjhi->jhi_strOSName, "Microsoft Windows Server 2003, ");
        }
    
        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
            ol_strcat(pjhi->jhi_strOSName, "Microsoft Windows XP ");
    
        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
            ol_strcat(pjhi->jhi_strOSName, "Microsoft Windows 2000 ");
    
        if (osvi.dwMajorVersion <= 4)
            ol_strcat(pjhi->jhi_strOSName, "Microsoft Windows NT ");
    
        // Test for specific product on Windows NT 4.0 SP6 and later.
        if (bOsVersionInfoEx)
        {
            // Test for the workstation type.
            if (osvi.wProductType == VER_NT_WORKSTATION &&
                si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64)
            {
                if (osvi.dwMajorVersion == 4)
                    ol_strcat(pjhi->jhi_strOSName,  "Workstation 4.0 ");
                else if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
                    ol_strcat(pjhi->jhi_strOSName,  "Home Edition ");
                else ol_strcat(pjhi->jhi_strOSName,  "Professional ");
            }
              
            // Test for the server type.
            else if (osvi.wProductType == VER_NT_SERVER || 
                     osvi.wProductType == VER_NT_DOMAIN_CONTROLLER)
            {
                if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
                {
                    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
                    {
                        if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                            ol_strcat(
                                pjhi->jhi_strOSName,
                                "Datacenter Edition for Itanium-based Systems ");
                        else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                            ol_strcat(
                                pjhi->jhi_strOSName,
                                "Enterprise Edition for Itanium-based Systems ");
                    }
    
                    else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                    {
                        if (osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            ol_strcat(pjhi->jhi_strOSName,  "Datacenter x64 Edition ");
                        else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            ol_strcat(pjhi->jhi_strOSName,  "Enterprise x64 Edition ");
                        else 
                            ol_strcat(pjhi->jhi_strOSName,  "Standard x64 Edition ");
                    }
                    else
                    {
                        if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                            ol_strcat(pjhi->jhi_strOSName, "Datacenter Edition ");
                        else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                            ol_strcat(pjhi->jhi_strOSName, "Enterprise Edition ");
                        else if (osvi.wSuiteMask == VER_SUITE_BLADE)
                            ol_strcat(pjhi->jhi_strOSName, "Web Edition ");
                        else 
                            ol_strcat(pjhi->jhi_strOSName, "Standard Edition ");
                    }
                }
                else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
                {
                    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                        ol_strcat(pjhi->jhi_strOSName, "Datacenter Server ");
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        ol_strcat(pjhi->jhi_strOSName, "Advanced Server ");
                    else
                        ol_strcat(pjhi->jhi_strOSName, "Server ");
                }
                else  // Windows NT 4.0 
                {
                    if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        ol_strcat(pjhi->jhi_strOSName, "Server 4.0, Enterprise Edition ");
                    else
                        ol_strcat(pjhi->jhi_strOSName, "Server 4.0 ");
                }
            }
        }
        else  
        {
            // Test for specific product on Windows NT 4.0 SP5 and earlier
            HKEY hKey;
            TCHAR szProductType[BUFSIZE];
            DWORD dwBufLen=BUFSIZE*sizeof(TCHAR);
            LONG lRet;
    
            lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
                0, KEY_QUERY_VALUE, &hKey );
            if (lRet != ERROR_SUCCESS)
                return FALSE;
    
            lRet = RegQueryValueEx(hKey, TEXT("ProductType"), NULL, NULL,
                (LPBYTE) szProductType, &dwBufLen);
            RegCloseKey(hKey);
    
            if ((lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE*sizeof(TCHAR)))
                return FALSE;
    
            if (lstrcmpi(TEXT("WINNT"), szProductType) == 0)
                ol_strcat(pjhi->jhi_strOSName, "Workstation ");
            if (lstrcmpi(TEXT("LANMANNT"), szProductType) == 0)
                ol_strcat(pjhi->jhi_strOSName, "Server ");
            if (lstrcmpi(TEXT("SERVERNT"), szProductType) == 0)
                ol_strcat(pjhi->jhi_strOSName, "Advanced Server ");
            ol_sprintf(
                pjhi->jhi_strOSName,  "%s%d.%d ", pjhi->jhi_strOSName,
                osvi.dwMajorVersion, osvi.dwMinorVersion );
        }
    
        // Display service pack (if any) and build number.
        if (osvi.dwMajorVersion == 4 &&
            lstrcmpi(osvi.szCSDVersion, TEXT("Service Pack 6")) == 0)
        { 
            HKEY hKey;
            LONG lRet;
    
            // Test for SP6 versus SP6a.
            lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009"),
                0, KEY_QUERY_VALUE, &hKey );
            if (lRet == ERROR_SUCCESS)
                ol_sprintf(
                    pjhi->jhi_strOSName, "%s Service Pack 6a (Build %d)",
                    pjhi->jhi_strOSName, osvi.dwBuildNumber & 0xFFFF);
            else // Windows NT 4.0 prior to SP6a
            {
                ol_sprintf(pjhi->jhi_strOSName, "%s%s (Build %d)",
                        pjhi->jhi_strOSName,
                        osvi.szCSDVersion,
                        osvi.dwBuildNumber & 0xFFFF);
            }
    
            RegCloseKey(hKey);
        }
        else // not Windows NT 4.0 
        {
            ol_sprintf(pjhi->jhi_strOSName, "%s%s (Build %d)",
                    pjhi->jhi_strOSName,
                    osvi.szCSDVersion,
                    osvi.dwBuildNumber & 0xFFFF);
        }
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        // Test for the Windows Me/98/95.
        if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
        {
            ol_strcat(pjhi->jhi_strOSName, "Microsoft Windows 95 ");
            if (osvi.szCSDVersion[1]=='C' || osvi.szCSDVersion[1]=='B')
                ol_strcat(pjhi->jhi_strOSName,  "OSR2 " );
        } 
    
        if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
        {
            ol_strcat(pjhi->jhi_strOSName, "Microsoft Windows 98 ");
            if (osvi.szCSDVersion[1]=='A' || osvi.szCSDVersion[1]=='B')
                ol_strcat(pjhi->jhi_strOSName,  "SE " );
        } 
    
        if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
        {
            ol_strcat(pjhi->jhi_strOSName, "Microsoft Windows Millennium Edition ");
        } 
        break;
    case VER_PLATFORM_WIN32s:
        ol_strcat(pjhi->jhi_strOSName, "Microsoft Win32s");
        break;
    }

    if (pjhi->jhi_strOSName[0] == '\0')
        ol_strcpy(pjhi->jhi_strOSName, jf_string_getStringUnknown());
#endif
    return u32Ret;
}

static u32 _getHostNetworkInfo(jf_host_info_t * pjhi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(LINUX)
    jf_ifmgmt_if_t allIf[JF_HOST_MAX_NET_INTERFACES];
    u32 u32Count = JF_HOST_MAX_NET_INTERFACES;
    u32 u32Index;

    u32Ret = jf_ifmgmt_getAllIf(allIf, &u32Count);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pjhi->jhi_u16NetCount = (u16)u32Count;
        for (u32Index = 0; u32Index < u32Count; u32Index ++)
        {
            ol_strncpy(
                pjhi->jhi_jhniNet[u32Index].jhni_strName,
                allIf[u32Index].jii_strName, JF_HOST_MAX_NET_NAME_LEN);
            jf_ipaddr_getStringIpAddr(
                pjhi->jhi_jhniNet[u32Index].jhni_strIpAddr,
                &allIf[u32Index].jii_jiAddr);
        }
    }

#elif defined(WINDOWS)
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    PIP_ADAPTER_INFO pOriginalPtr;
    ULONG ulSizeAdapterInfo = 0;
    DWORD dwStatus;
    u16 u16Count = 0;

    /*Find out how big our buffer needs to be to hold the data*/
    dwStatus = GetAdaptersInfo(pAdapterInfo, &ulSizeAdapterInfo);
    if (dwStatus == ERROR_BUFFER_OVERFLOW)
    {
        /*Allocate a buffer of the appropriate size*/
        pAdapterInfo = (PIP_ADAPTER_INFO)malloc(ulSizeAdapterInfo);
        if (pAdapterInfo != NULL)
        {
            dwStatus = GetAdaptersInfo(pAdapterInfo, &ulSizeAdapterInfo);
            if (dwStatus == ERROR_SUCCESS)
            {
                pOriginalPtr = pAdapterInfo;

                /*Step through the adapter list*/
                while (pAdapterInfo != NULL)
                {
                    IP_ADDR_STRING * pAddressList = &(pAdapterInfo->IpAddressList);

                    ol_strncpy(
                        pjhi->jhi_jhniNet[u16Count].jhni_strName,
                        pAdapterInfo->Description, JF_HOST_MAX_NET_NAME_LEN - 1);

                    /*This is an address list, we only get the first one
                      To get the next address, iterate pAddressList by
                      pAddressList = pAddressList->Next*/
                    ol_strncpy(
                        pjhi->jhi_jhniNet[u16Count].jhni_strIpAddr,
                        pAddressList->IpAddress.String, JF_HOST_MAX_IP_ADDR_LEN - 1);

                    /*And so on with other members of the Adapter info structure*/
                    pAdapterInfo = pAdapterInfo->Next;
                    u16Count ++;
                }
                
                pjhi->jhi_u16NetCount = u16Count;        
            }

            free(pAdapterInfo);
        }
    }

#endif
    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
void jf_host_getName(olchar_t * pstrName, u32 u32Len)
{
#if defined(LINUX)
    gethostname(pstrName, u32Len);
#elif defined(WINDOWS)

#endif
}

u32 jf_host_getInfo(jf_host_info_t * pjhi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    
    ol_memset(pjhi, 0, sizeof(jf_host_info_t));

    u32Ret = _getHostVersion(pjhi);
    if(u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _getHostNetworkInfo(pjhi);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


