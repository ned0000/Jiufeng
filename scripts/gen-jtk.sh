#!/bin/bash
# Generate Jiufeng Tool Kit.

SUCCESS=0
E_ARGERROR=65
E_EXE_NEXISTING=66
E_BUILD_ERROR=67

debug=no

bin_files="jf_dongyuan jf_servctl jf_logserver jf_logctl jf_dispatcher jf_configmgr jf_configctl jf_genuuid jf_errcode"

help_gen_jtk()
{
    echo ""
    echo "`basename $0` : generate Jiufeng Took Kit"
    echo "Usage: `basename $0` dir debug"
    echo "   dir: the directory containing the JTK."
    echo "   debug: debug version."
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

destdir=$(cd $1; pwd)

dest_rootdir=$destdir/jtk
dest_bindir=$dest_rootdir/bin
dest_incdir=$dest_rootdir/inc
dest_libdir=$dest_rootdir/lib
dest_configdir=$dest_rootdir/config
dest_docdir=$dest_rootdir/doc
dest_makdir=$dest_rootdir/mak
dest_templatedir=$dest_rootdir/template

rm -fr $dest_rootdir

mkdir $dest_rootdir
mkdir $dest_bindir
mkdir $dest_incdir
mkdir $dest_libdir
mkdir $dest_configdir
mkdir $dest_docdir
mkdir $dest_makdir
mkdir $dest_templatedir

cd ..
topdir=`pwd`

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

print_banner "Generate documentation"
cd $topdir/
rm -fr doc/html
doxygen doc/Doxyfile

print_banner "Copy executable files"
cd $topdir/build/bin
for file in $bin_files
do
    echo "Copy $file"
    if [ $debug != yes ]; then
        strip $file
    fi
    cp $file $dest_bindir
done

print_banner "Copy header files"
cd $topdir/jiutai
for headerfile in $(ls *.h)
do
    echo "Copy $headerfile"
    cp $headerfile $dest_incdir
    objfile=${headerfile/.h/.o}  
    if [ -e $objfile ]; then
        echo "Copy $objfile"
        cp $objfile $dest_incdir
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
    cp $libfile $dest_libdir
done

print_banner "Copy config files"
cd $topdir/build/config
for file in $(ls)
do
    echo "Copy $file"
    cp $file $dest_configdir
done

print_banner "Copy documentation"
cd $topdir/doc
cp -r html $dest_docdir

print_banner "Copy makefiles"
cd $topdir/mak
for file in $(ls)
do
    makefile=$file
    echo "Copy $makefile"
    cp $makefile $dest_makdir
done
if [ $debug == yes ]; then
    echo "DEBUG_JIUFENG = yes" > $dest_makdir/lnxcfg.mak
    cat $topdir/mak/lnxcfg.mak >> $dest_makdir/lnxcfg.mak
fi

print_banner "Copy template files"
cd $topdir/template
for file in $(ls)
do
    echo "Copy $file"
    cp $file $dest_templatedir
done

cd $destdir
echo ""
echo "Create symbolic link for mak and template directory"

ln -sf jtk/mak/ mak
ln -sf jtk/template/ template

echo ""

