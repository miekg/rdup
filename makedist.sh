#!/bin/sh

# Build a rdup distribution tar from the SVN repository.

set -e
cwd=`pwd`

usage () {
    cat >&2 <<EOF
Usage $0: [-h] [-s] [-d SVN_root]
Generate a distribution tar file for rdup.

    -h           This usage information.
    -s           Build a snapshot distribution file.  The current date is
                 automatically appended to the current rdup version number.
    -d SVN_root  Retrieve the rdup source from the specified repository.
EOF
    exit 1
}

info () {
    echo "$0: info: $1"
}

error () {
    echo "$0: error: $1" >&2
    exit 1
}

question () {
    printf "%s (y/n) " "$*"
    read answer
    case "$answer" in
        [Yy]|[Yy][Ee][Ss])
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

# Only use cleanup and error_cleanup after generating the temporary
# working directory.
cleanup () {
    info "Deleting temporary working directory."
    cd $cwd && rm -rf $temp_dir
}

error_cleanup () {
    echo "$0: error: $1" >&2
    cleanup
    exit 1
}

replace_text () {
    (cp "$1" "$1".orig && \
        sed -e "s/$2/$3/g" < "$1".orig > "$1" && \
        rm "$1".orig) || error_cleanup "Replacement for $1 failed."
}

replace_all () {
    info "Updating '$1' with the version number."
    replace_text "$1" "@version@" "$version"
    info "Updating '$1' with today's date."
    replace_text "$1" "@date@" "`date +'%b %e, %Y'`"
}

SNAPSHOT="no"

# Parse the command line arguments.
while [ "$1" ]; do
    case "$1" in
        "-h")
            usage
            ;;
        "-d")
            SVNROOT="$2"
            shift
            ;;
        "-s")
            SNAPSHOT="yes"
            ;;
        *)
            error "Unrecognized argument -- $1"
            ;;
    esac
    shift
done

# Check if SVNROOT is specified.
if [ -z "$SVNROOT" ]; then
    error "SVNROOT must be specified (using -d)"
fi

# Start the packaging process.
info "SVNROOT  is $SVNROOT"
info "SNAPSHOT is $SNAPSHOT"

# Creating temp directory
info "Creating temporary working directory"
temp_dir=`mktemp -d rdup-dist-XXXXXX`
info "Directory '$temp_dir' created."
cd $temp_dir

info "Exporting source from SVN."
svn export "$SVNROOT" rdup || error_cleanup "SVN command failed"

cd rdup || error_cleanup "rdup not exported correctly from SVN"

info "Building configure script (autoreconf)."
autoreconf || error_cleanup "Autoreconf failed."

rm -r autom4te* || error_cleanup "Failed to remove autoconf cache directory."
rm -rf patches  || error_cleanup "Failed to remove patches directory."
rm -rf feedback   || error_cleanup "Failed to remove feedback directory."
rm -rf rpm   || error_cleanup "Failed to remove rpm directory."

find . -name .c-mode-rc.el -exec rm {} \;
find . -name .cvsignore -exec rm {} \;
find . -name .svn -exec rm -rf {} \;
rm makedist.sh || error_cleanup "Failed to remove makedist.sh."

info "Determining rdup version."
version=`./configure --version | head -1 | awk '{ print $3 }'` || \
    error_cleanup "Cannot determine version number."

info "rdup version: $version"

if [ "$SNAPSHOT" = "yes" ]; then
    info "Building rdup snapshot."
    version="$version-`date +%Y%m%d`"
    info "Snapshot version number: $version"
fi

info "Renaming rdup directory to rdup-$version..."
cd ..
mv rdup rdup-$version || error_cleanup "Failed to rename rdup directory."

tarfile="../rdup-$version.tar.gz"

if [ -f $tarfile ]; then
    (question "The file $tarfile already exists.  Overwrite?" \
        && rm -f $tarfile) || error_cleanup "User abort."
fi

info "Deleting the tpkg and test directory"
rm -rf rdup-$version/test/
info "Deleting the php directory"
rm -rf rdup-$version/php/

info "Deleting the other fluff"
rm -rf rdup-$version/.svn
rm -f rdup-$version/core
rm -f rdup-$version/tar-exclude
rm -f rdup-$version/config.log
rm -f rdup-$version/config.status
rm -f rdup-$version/tags
rm -rf rdup-$version/doc/tex
rm -rf rdup-$version/sh-tools
rm -rf rdup-$version/*.o

info "Creating manual pages"
( cd doc
for i in rdup-*.rst ; do echo $i; rst2man $i > `basename $i .rst` ; done
)

info "Creating tar rdup-$version.tar.bz2"
tar cjf ../rdup-$version.tar.bz2 rdup-$version || error_cleanup "Failed to create tar file."

# make the debian package
info "Creating Debian package rdup_...$version.deb."
( cd rdup-$version
dpkg-buildpackage -us -uc -rfakeroot 2>/dev/null >/dev/null ) || \
error_cleanup "Failed to create Debian package."
mv *.deb ../

cleanup
sha=`sha1sum rdup-$version.tar.bz2 |  awk '{ print $1 }'`
echo $sha > rdup-$version.tar.bz2.sha1

info "rdup distribution created successfully."
info "SHA1: $sha"
