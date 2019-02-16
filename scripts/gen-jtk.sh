#!/bin/bash
# generate Jiufeng Tool Kit

SUCCESS=0
E_ARGERROR=65
E_EXE_NEXISTING=66
E_BUILD_ERROR=67

debug=no

cd ..
topdir=`pwd`

head_files="aether.h olflag.h crc32c.h hexstr.h ollimit.h stringparse.h archive.h array.h hostinfo.h priotree.h syncmutex.h attask.h dynlib.h prng.h syncrwlock.h avltree.h ifmgmt.h process.h syncsem.h bases.h encode.h radixtree.h bignum.h encrypt.h resource.h user.h bitarray.h errcode.h respool.h uuid.h bitop.h logger.h version.h byteorder.h menu.h waitqueue.h conffile.h cghash.h cgmac.h cksum.h files.h network.h compress.h workqueue.h getopt.h xmalloc.h comminit.h hash.h sharedmemory.h olbasic.h xtime.h event.h clieng.h matrix.h zeus.h hermes.h webclient.h httpparser.h nelly.h persistency.h datavec.h"

lib_files="logger aether stringparse files cghash cgmac encrypt prng encode archive ifmgmt network uuid matrix compress servmgmt clieng iscsit iscsii hermes webclient httpparser audio persistency"
libxml2_file=libxml2.so

bin_files="genuuid oldongyuan olservmgmt"

help_gen_jtk()
{
    echo ""
    echo "`basename $0` : generate Jiufeng Took Kit"
    echo "Usage: `basename $0` dir debug"
    echo "   dir: the dir containing the JTK"
    echo "   debug: debug version"
    echo ""
}

print_banner()
{
    echo ""
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo "+ ""$1"
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
}

if [ x"$1" == x-h ]; then
    help_gen_jtk
    exit $SUCCESS
fi

if [ ! -d "$1" ]; then
    echo "Invalid directory"
    help_gen_jtk
    exit $E_ARGERROR
fi

if [ x"$2" = xdebug ]; then
    debug=yes
fi

rootdir=$1/jtk
bindir=$rootdir/bin
incdir=$rootdir/inc
libdir=$rootdir/lib
docdir=$rootdir/doc
makdir=$rootdir/mak

rm -fr $rootdir

mkdir $rootdir
mkdir $bindir
mkdir $incdir
mkdir $libdir
mkdir $docdir
mkdir $makdir

echo "Build Jiufeng Took Kit"
make -f linux.mak clean
if [ $debug = yes ]; then
    make -f linux.mak DEBUG_JIUFENG=yes
else
    make -f linux.mak
fi

if [ $? -ne 0 ]; then
    echo "Error in building Jiufeng Tool Kit"
    exit $E_BUILD_ERROR
fi

cd $topdir

print_banner "Copy header files"
cd $topdir/jiutai
for headerfile in $(ls *.h)
do
    echo "Copy $headerfile"
    cp $headerfile $incdir
    objfile=${headerfile/.h/.o}  
    if [ -e $objfile ]; then
        echo "Copy $objfile"
        cp $objfile $incdir
    fi
done

print_banner "Copy library files"
cd $topdir/build/lib
for file in $(ls lib*)
do
    libfile=$file
    echo "Copy $libfile"
    if [ $debug != yes ]; then
        strip $libfile
    fi
    cp $libfile $libdir
done
if [ $debug != yes ]; then
    strip $libxml2_file
fi

print_banner "Copy executable files"
cd $topdir/build/bin
for file in $bin_files
do
    echo "Copy $file"
    if [ $debug != yes ]; then
        strip $file
    fi
    cp $file $bindir
done

print_banner "Copy make files"
cp $topdir/mak/lnxdef.mak $makdir
if [ $debug = yes ]; then
    echo "CFLAGS += -g -DDEBUG" >> $makdir/lnxdef.mak
fi

print_banner "Copy setting files"
cp $topdir/servmgmt/servmgmt.setting $bindir


