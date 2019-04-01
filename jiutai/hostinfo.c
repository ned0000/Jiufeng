/**
 *  @file hostinfo.c
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
#include "olbasic.h"
#include "errcode.h"
#include "hostinfo.h"
#include "stringparse.h"
#include "ifmgmt.h"

/* --- private data/data structure section --------------------------------- */
#if defined(WINDOWS)
    typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
#endif
#define SM_SERVERR2 89
#define BUFSIZE 80

/* --- private funciton routines ------------------------------------------- */

static u32 _getHostVersion(host_info_t * phi)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olint_t nRet;
    struct utsname name;

    nRet = uname(&name);
    if (nRet == 0)
    {
        ol_sprintf(phi->hi_strHostName, "%s", name.nodename);
        ol_sprintf(phi->hi_strOSName, "%s %s %s", name.sysname, name.release, name.machine);
    }
    else
    {
        ol_strcpy(phi->hi_strHostName, STRING_UNKNOWN);
        ol_strcpy(phi->hi_strOSName, STRING_UNKNOWN);
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
        ol_strncpy(phi->hi_strHostName, u8HostName, MAX_HOST_NAME_LEN - 1);
    }
    else
    {
        ol_strcpy(phi->hi_strHostName, STRING_UNKNOWN);
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
                ol_strcat(phi->hi_strOSName, "Microsoft Windows Vista ");
            else if (osvi.wProductType == VER_NT_SERVER)
                ol_strcat(phi->hi_strOSName, "Windows Server 2008 ");
            else
                ol_strcat(phi->hi_strOSName, "Windows Server \"Longhorn\" ");
        }
    
        if ((osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) ||
            (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0))
        {
            if (GetSystemMetrics(SM_SERVERR2))
                ol_strcat(phi->hi_strOSName, "Microsoft Windows Server 2003 \"R2\" ");
            else if (osvi.wProductType == VER_NT_WORKSTATION &&
                     si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
            {
                ol_strcat(phi->hi_strOSName, "Microsoft Windows XP Professional x64 Edition ");
            }
            else
                ol_strcat(phi->hi_strOSName, "Microsoft Windows Server 2003, ");
        }
    
        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
            ol_strcat(phi->hi_strOSName, "Microsoft Windows XP ");
    
        if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
            ol_strcat(phi->hi_strOSName, "Microsoft Windows 2000 ");
    
        if (osvi.dwMajorVersion <= 4)
            ol_strcat(phi->hi_strOSName, "Microsoft Windows NT ");
    
        // Test for specific product on Windows NT 4.0 SP6 and later.
        if (bOsVersionInfoEx)
        {
            // Test for the workstation type.
            if (osvi.wProductType == VER_NT_WORKSTATION &&
                si.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_AMD64)
            {
                if (osvi.dwMajorVersion == 4)
                    ol_strcat(phi->hi_strOSName,  "Workstation 4.0 ");
                else if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
                    ol_strcat(phi->hi_strOSName,  "Home Edition ");
                else ol_strcat(phi->hi_strOSName,  "Professional ");
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
                            ol_strcat(phi->hi_strOSName,  "Datacenter Edition for Itanium-based Systems ");
                        else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                            ol_strcat(phi->hi_strOSName,  "Enterprise Edition for Itanium-based Systems ");
                    }
    
                    else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                    {
                        if (osvi.wSuiteMask & VER_SUITE_DATACENTER )
                            ol_strcat(phi->hi_strOSName,  "Datacenter x64 Edition ");
                        else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                            ol_strcat(phi->hi_strOSName,  "Enterprise x64 Edition ");
                        else 
                            ol_strcat(phi->hi_strOSName,  "Standard x64 Edition ");
                    }
                    else
                    {
                        if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                            ol_strcat(phi->hi_strOSName, "Datacenter Edition ");
                        else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                            ol_strcat(phi->hi_strOSName, "Enterprise Edition ");
                        else if (osvi.wSuiteMask == VER_SUITE_BLADE)
                            ol_strcat(phi->hi_strOSName, "Web Edition ");
                        else 
                            ol_strcat(phi->hi_strOSName, "Standard Edition ");
                    }
                }
                else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
                {
                    if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
                        ol_strcat(phi->hi_strOSName, "Datacenter Server ");
                    else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        ol_strcat(phi->hi_strOSName, "Advanced Server ");
                    else
                        ol_strcat(phi->hi_strOSName, "Server ");
                }
                else  // Windows NT 4.0 
                {
                    if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
                        ol_strcat(phi->hi_strOSName, "Server 4.0, Enterprise Edition ");
                    else
                        ol_strcat(phi->hi_strOSName, "Server 4.0 ");
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
                ol_strcat(phi->hi_strOSName, "Workstation ");
            if (lstrcmpi(TEXT("LANMANNT"), szProductType) == 0)
                ol_strcat(phi->hi_strOSName, "Server ");
            if (lstrcmpi(TEXT("SERVERNT"), szProductType) == 0)
                ol_strcat(phi->hi_strOSName, "Advanced Server ");
            ol_sprintf(phi->hi_strOSName,  "%s%d.%d ", phi->hi_strOSName, osvi.dwMajorVersion, osvi.dwMinorVersion );
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
                ol_sprintf(phi->hi_strOSName, "%s Service Pack 6a (Build %d)", phi->hi_strOSName,  
                        osvi.dwBuildNumber & 0xFFFF);
            else // Windows NT 4.0 prior to SP6a
            {
                ol_sprintf(phi->hi_strOSName, "%s%s (Build %d)",
                        phi->hi_strOSName,
                        osvi.szCSDVersion,
                        osvi.dwBuildNumber & 0xFFFF);
            }
    
            RegCloseKey(hKey);
        }
        else // not Windows NT 4.0 
        {
            ol_sprintf(phi->hi_strOSName, "%s%s (Build %d)",
                    phi->hi_strOSName,
                    osvi.szCSDVersion,
                    osvi.dwBuildNumber & 0xFFFF);
        }
        break;
    case VER_PLATFORM_WIN32_WINDOWS:
        // Test for the Windows Me/98/95.
        if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
        {
            ol_strcat(phi->hi_strOSName, "Microsoft Windows 95 ");
            if (osvi.szCSDVersion[1]=='C' || osvi.szCSDVersion[1]=='B')
                ol_strcat(phi->hi_strOSName,  "OSR2 " );
        } 
    
        if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
        {
            ol_strcat(phi->hi_strOSName, "Microsoft Windows 98 ");
            if (osvi.szCSDVersion[1]=='A' || osvi.szCSDVersion[1]=='B')
                ol_strcat(phi->hi_strOSName,  "SE " );
        } 
    
        if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
        {
            ol_strcat(phi->hi_strOSName, "Microsoft Windows Millennium Edition ");
        } 
        break;
    case VER_PLATFORM_WIN32s:
        ol_strcat(phi->hi_strOSName, "Microsoft Win32s");
        break;
    }

    if (phi->hi_strOSName[0] == '\0')
        ol_strcpy(phi->hi_strOSName, STRING_UNKNOWN);
#endif
    return u32Ret;
}

static u32 _getHostNetworkInfo(host_info_t * phi)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)
    olchar_t strBuffer[16 * sizeof(struct ifreq)];
    struct ifconf ifConf;
    struct ifreq ifReq;
    olint_t sock = -1;
    olint_t ret;
    struct sockaddr_in localaddr;
    u16 u16Count = 0;
    olint_t i;
    jf_ipaddr_t ipaddr;

    /* Create an unbound datagram socket to do the SIOCGIFADDR ioctl on. */
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == -1)
        u32Ret = OLERR_FAIL_CREATE_SOCKET;

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* Get the interface configuration information... */
        ifConf.ifc_len = sizeof strBuffer;
        ifConf.ifc_ifcu.ifcu_buf = (caddr_t)strBuffer;
        ret = ioctl(sock, SIOCGIFCONF, &ifConf);
        if (ret == -1)
            u32Ret = OLERR_FAIL_IOCTL_SOCKET;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* Cycle through the list of interfaces looking for IP addresses. */
        for (i = 0; (i < ifConf.ifc_len); )
        {
            struct ifreq * pifReq = (struct ifreq *)((caddr_t)ifConf.ifc_req + i);

            i += sizeof(*pifReq);
            /* See if this is the sort of interface we want to deal with. */
            ol_strcpy(ifReq.ifr_name, pifReq->ifr_name);
            ret = ioctl(sock, SIOCGIFFLAGS, &ifReq);
            if (ret == -1)
            {
                u32Ret = OLERR_FAIL_IOCTL_SOCKET;
                break;
            }

            /* Skip loopback, point-to-point and down interfaces, */
            /* except don't skip down interfaces */
            /* if we're trying to get a list of configurable interfaces. */
            if ((ifReq.ifr_flags & IFF_LOOPBACK) || (! (ifReq.ifr_flags & IFF_UP)))
            {
                continue;
            }   
            if (pifReq->ifr_addr.sa_family == AF_INET)
            {
                /* Get a pointer to the address... */
                memcpy(&localaddr, &pifReq->ifr_addr, sizeof(pifReq->ifr_addr));
                if (localaddr.sin_addr.s_addr != htonl(INADDR_LOOPBACK))
                {
                    ipaddr.ji_u8AddrType = JF_IPADDR_TYPE_V4;
                    ipaddr.ji_uAddr.ju_nAddr = localaddr.sin_addr.s_addr;
                    jf_ipaddr_getStringIpAddr(phi->hi_niNet[u16Count].ni_strIpAddr, &ipaddr);

                    ol_strcpy(phi->hi_niNet[u16Count].ni_strName, ifReq.ifr_name);

                    ++u16Count;
                }
            }
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        phi->hi_u16NetCount = u16Count;
    }

    if (sock != -1)
        close(sock);
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

                    ol_strncpy(phi->hi_niNet[u16Count].ni_strName,
                        pAdapterInfo->Description, MAX_NET_NAME_LEN - 1);

                    /*This is an address list, we only get the first one
                      To get the next address, iterate pAddressList by
                      pAddressList = pAddressList->Next*/
                    ol_strncpy(phi->hi_niNet[u16Count].ni_strIpAddr,
                        pAddressList->IpAddress.String, MAX_IP_ADDR_LEN - 1);

                    /*And so on with other members of the Adapter info structure*/
                    pAdapterInfo = pAdapterInfo->Next;
                    u16Count ++;
                }
                
                phi->hi_u16NetCount = u16Count;        
            }

            free(pAdapterInfo);
        }
    }

#endif
    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
void getHostName(olchar_t * pstrName, u32 u32Len)
{
#if defined(LINUX)
    gethostname(pstrName, u32Len);
#elif defined(WINDOWS)

#endif
}

u32 getHostInfo(host_info_t * phi)
{
    u32 u32Ret = OLERR_NO_ERROR;
    
    memset(phi, 0, sizeof(host_info_t));

    u32Ret = _getHostVersion(phi);
    if(u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = _getHostNetworkInfo(phi);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


