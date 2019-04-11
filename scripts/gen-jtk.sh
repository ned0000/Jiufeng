#!/bin/bash
# generate Jiufeng Tool Kit

SUCCESS=0
E_ARGERROR=65
E_EXE_NEXISTING=66
E_BUILD_ERROR=67

debug=no

cd ..
topdir=`pwd`

bin_files="jf_genuuid jf_dongyuan jf_servmgmt"
setting_files="servmgmt/servmgmt.setting"

help_gen_jtk()
{
    echo ""
    echo "`basename $0` : generate Jiufeng Took Kit"
    echo "Usage: `basename $0` dir debug"
    echo "   dir: the dir containing the JTK"
    echo "   debug: debug version"
    echo "Eg. `basename $0` /root debug"
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

destdir=$1
rootdir=$destdir/jtk
bindir=$rootdir/bin
incdir=$rootdir/inc
libdir=$rootdir/lib
docdir=$rootdir/doc
makdir=$rootdir/mak
templatedir=$rootdir/template

rm -fr $rootdir

mkdir $rootdir
mkdir $bindir
mkdir $incdir
mkdir $libdir
mkdir $docdir
mkdir $makdir
mkdir $templatedir

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

print_banner "Copy makefiles"
cd $topdir/mak
for file in $(ls)
do
    makefile=$file
    echo "Copy $makefile"
    cp $makefile $makdir
done
if [ $debug == yes ]; then
    echo "DEBUG_JIUFENG = yes" > $makdir/lnxcfg.mak
    cat $topdir/mak/lnxcfg.mak >> $makdir/lnxcfg.mak
fi

print_banner "Copy setting files"
cd $topdir
for file in $setting_files
do
    echo "Copy $file"
    cp $file $bindir
done

print_banner "Copy template files"
cd $topdir/template
for file in $(ls)
do
    echo "Copy $file"
    cp $file $templatedir
done

cd $destdir
echo ""
echo "Create symbolic link for mak and template directory"
ln -sf jtk/mak/ mak
ln -sf jtk/template/ template

echo ""

