/**
 *  @file seed.c
 *
 *  @brief Get PRNG seed from OS.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Link with jf_files library for reading random device files on Linux platform.
 *  -# Link with Bcrypt.lib for random data on Windows platform.
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <stdio.h>
#if defined(LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
    #include <sys/un.h>
    #include <errno.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
#elif defined(WINDOWS)
    #include <winsock2.h>
    #include <windows.h>
    #include <lm.h>
    #include <Tlhelp32.h>
    #include <bcrypt.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_prng.h"
#include "jf_file.h"
#include "jf_time.h"
#include "jf_process.h"
#include "jf_thread.h"

#include "prngcommon.h"
#include "clrmem.h"

/* --- private data/data structure section ------------------------------------------------------ */
#if defined(LINUX)
    /** Set this to a comma-separated list of 'random' device files to try out.
     */
    #define RANDOM_DEV "/dev/urandom","/dev/random","/dev/srandom"
#elif defined(WINDOWS)

#endif

/* --- private routine section ------------------------------------------------------------------ */

static u32 _getSeedFromSystem(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 tmpbuf[ENTROPY_NEEDED];
#if defined(LINUX)
    olchar_t * randomfiles[] = { RANDOM_DEV };
    u32 u32NumOfRandomFile = ARRAY_SIZE(randomfiles);
    jf_file_stat_t filestats[ARRAY_SIZE(randomfiles)];
    jf_file_t fd;
    u32 i, j;
    olint_t usec = 10 * 1000; /* spend 10ms on each file */
    jf_file_stat_t * st = NULL;
    olsize_t sread = 0, sleft = 0;
    struct timeval tv;

    memset(filestats, 0, sizeof(filestats));

    /** Use a random entropy pool device. Use /dev/urandom as /dev/random may
     *  block if it runs out of random entries.
     */
    for (i = 0; i < u32NumOfRandomFile && sread < ENTROPY_NEEDED; i ++)
    {
        u32Ret = jf_file_open(
            randomfiles[i], O_RDONLY | O_NONBLOCK | O_NOCTTY, &fd);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            st = &filestats[i];

            u32Ret = jf_file_getStat(randomfiles[i], st);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /** Avoid using same device */
                for (j = 0; j < i; j ++)
                {
                    if (filestats[j].jfs_u32INode == st->jfs_u32INode &&
                        filestats[j].jfs_u32Dev == st->jfs_u32Dev)
                        break;
                }
                if (j < i)
                {
                    jf_file_close(&fd);
                    continue;
                }
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                tv.tv_sec = 0;
                tv.tv_usec = usec;
                sleft = ENTROPY_NEEDED - sread;
                u32Ret = jf_file_readWithTimeout(fd, tmpbuf + sread, &sleft, &tv);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                sread += sleft;
            }

            jf_file_close(&fd);
        }
    }

    if (sread > 0)
    {
        jf_prng_seed(tmpbuf, sizeof(tmpbuf), (oldouble_t)sread);
        clearMemory(tmpbuf, sread);
    }

#elif defined(WINDOWS)
    HRESULT hr = S_OK;

    hr = BCryptGenRandom(NULL, tmpbuf, ENTROPY_NEEDED, BCRYPT_USE_SYSTEM_PREFERRED_RNG);

    if (FAILED(hr))
        u32Ret = JF_ERR_PRNG_NOT_SEEDED;
    else
        u32Ret = jf_prng_seed(tmpbuf, sizeof(tmpbuf), ENTROPY_NEEDED);

#endif

    return u32Ret;
}

static u32 _getSeedFromProcess(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    pid_t pid = jf_process_getCurrentId();
    pthread_t threadid = jf_thread_getCurrentId();
#if defined(LINUX)
    uid_t uid = getuid();
#endif

    jf_prng_seed((u8 *)&pid, sizeof(pid), 0.0);
    jf_prng_seed((u8 *)&threadid, sizeof(threadid), 0.0);

#if defined(LINUX)
    jf_prng_seed((u8 *)&uid, sizeof(uid), 0.0);
#endif

    return u32Ret;
}

static u32 _getSeedFromTime(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_time_val_t jtv;

    u32Ret = jf_time_getTimeOfDay(&jtv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_prng_seed((u8 *)&jtv.jtv_u64Second, sizeof(jtv.jtv_u64Second), 0.0);
        jf_prng_seed((u8 *)&jtv.jtv_u64MicroSecond, sizeof(jtv.jtv_u64MicroSecond), 0.0);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 getSeed(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*The seed from system has entropy.*/
    u32Ret = _getSeedFromSystem();

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*It's ok to failed to seed from process and time.*/
        _getSeedFromProcess();

        _getSeedFromTime();
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


