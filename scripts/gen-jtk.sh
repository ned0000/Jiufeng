#!/bin/bash
# Generate Jiufeng Tool Kit.

# Define the error code.
SUCCESS=0
E_ARGERROR=65
E_EXE_NEXISTING=66
E_BUILD_ERROR=67

# Define the global variable.
debug=no

# The binary files should be copied.
bin_files="jf_dongyuan jf_servctl jf_logserver jf_logctl jf_dispatcher jf_configmgr jf_configctl jf_genuuid jf_errcode"

# Funciton to print the usage.
help_gen_jtk()
{
    echo ""
    echo "`basename $0` : generate Jiufeng Took Kit"
    echo "Usage: `basename $0` dir debug"
    echo "  dir: the directory containing the JTK."
    echo "  debug: debug version."
    echo "Eg. `basename $0` /root debug"
    echo ""
}

# Function to print the banner.
print_banner()
{
    echo ""
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
    echo "+ ""$1"
    echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
}

# Test if it's the option to print usage.
if [ x"$1" == x-h ]; then
    help_gen_jtk
    exit $SUCCESS
fi

# Check the first option, the destination directory.
if [ ! -d "$1" ]; then
    echo "Invalid directory"
    help_gen_jtk
    exit $E_ARGERROR
fi

# Check the second option, the debug option.
if [ x"$2" = xdebug ]; then
    debug=yes
fi

# The destination directory.
destdir=$(cd $1; pwd)

# The sub-directory in the destination directory.
dest_rootdir=$destdir/jtk
dest_bindir=$dest_rootdir/bin
dest_incdir=$dest_rootdir/inc
dest_libdir=$dest_rootdir/lib
dest_configdir=$dest_rootdir/config
dest_docdir=$dest_rootdir/doc
dest_makdir=$dest_rootdir/mak
dest_templatedir=$dest_rootdir/template

# Remove the original destination directory.
rm -fr $dest_rootdir

# Create the destination directory and sub-directory.
mkdir $dest_rootdir
mkdir $dest_bindir
mkdir $dest_incdir
mkdir $dest_libdir
mkdir $dest_configdir
mkdir $dest_docdir
mkdir $dest_makdir
mkdir $dest_templatedir

# Build JTK.
cd ..
topdir=`pwd`

echo "Build Jiufeng Took Kit"
make -f linux.mak clean
if [ $debug = yes ]; then
    make -f linux.mak DEBUG_JIUFENG=yes
else
    make -f linux.mak
fi

# Check the build result.
if [ $? -ne 0 ]; then
    echo "Error in building Jiufeng Tool Kit"
    exit $E_BUILD_ERROR
fi

# Generate documentation of JTK.
print_banner "Generate documentation"
cd $topdir/
rm -fr doc/html
doxygen doc/Doxyfile

# Copy executable files.
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

# Copy header files.
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

# Copy library files.
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

# Copy config files.
print_banner "Copy config files"
cd $topdir/build/config
for file in $(ls)
do
    echo "Copy $file"
    cp $file $dest_configdir
done

# Copy documentation.
print_banner "Copy documentation"
cd $topdir/doc
cp -r html $dest_docdir

# Copy makefiles.
print_banner "Copy makefiles"
cd $topdir/mak
for file in $(ls)
do
    makefile=$file
    echo "Copy $makefile"
    cp $makefile $dest_makdir
done
# Append an extra configuration to config makefile according to the debug option.
if [ $debug == yes ]; then
    echo "DEBUG_JIUFENG = yes" > $dest_makdir/lnxcfg.mak
    cat $topdir/mak/lnxcfg.mak >> $dest_makdir/lnxcfg.mak
fi

# Copy template files.
print_banner "Copy template files"
cd $topdir/template
for file in $(ls)
do
    echo "Copy $file"
    cp $file $dest_templatedir
done

# Create symbolic link files.
cd $destdir
echo ""
echo "Create symbolic link for mak and template directory"

# Remove the original link files, otherwise the link files will be mistakenly created in the directory.
unlink mak 2> /dev/null
unlink template 2> /dev/null

# Create link files in the destination directory to the jtk directory.
ln -sf jtk/mak/ mak
ln -sf jtk/template/ template

echo ""

