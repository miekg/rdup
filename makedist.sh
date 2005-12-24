#!/bin/sh

# Build a rdump distribution tar from the SVN repository.

set -e

cwd=`pwd`

usage () {
    cat >&2 <<EOF
Usage $0: [-h] [-s] [-d SVN_root]
Generate a distribution tar file for rdump.

    -h           This usage information.
    -s           Build a snapshot distribution file.  The current date is
                 automatically appended to the current rdump version number.
    -d SVN_root  Retrieve the rdump source from the specified repository.
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

#question "Do you wish to continue with these settings?" || error "User abort."


# Creating temp directory
info "Creating temporary working directory"
temp_dir=`mktemp -d rdump-dist-XXXXXX`
info "Directory '$temp_dir' created."
cd $temp_dir

info "Exporting source from SVN."
svn export "$SVNROOT" rdump || error_cleanup "SVN command failed"

cd rdump || error_cleanup "rdump not exported correctly from SVN"

info "Building configure script (autoconf)."
autoconf || error_cleanup "Autoconf failed."

rm -r autom4te* || error_cleanup "Failed to remove autoconf cache directory."

find . -name .c-mode-rc.el -exec rm {} \;
find . -name .cvsignore -exec rm {} \;
rm makedist.sh || error_cleanup "Failed to remove makedist.sh."

info "Determining rdump version."
version=`./configure --version | head -1 | awk '{ print $3 }'` || \
    error_cleanup "Cannot determine version number."

info "rdump version: $version"

if [ "$SNAPSHOT" = "yes" ]; then
    info "Building rdump snapshot."
    version="$version-`date +%Y%m%d`"
    info "Snapshot version number: $version"
fi

info "Renaming rdump directory to rdump-$version."
cd ..
mv rdump rdump-$version || error_cleanup "Failed to rename rdump directory."

tarfile="../rdump-$version.tar.gz"

if [ -f $tarfile ]; then
    (question "The file $tarfile already exists.  Overwrite?" \
        && rm -f $tarfile) || error_cleanup "User abort."
fi

info "Deleting the tpkg and test directory"
rm -rf rdump-$version/older/

info "Deleting the other fluff"
rm -rf rdump-$version/.svn
rm -f rdump-$version/core
rm -f rdump-$version/tar-exclude
rm -f rdump-$version/config.log 
rm -f rdump-$version/config.status
rm -f rdump-$version/tags rdump-$version/src/tags

info "Creating tar rdump-$version.tar.bz2"
tar cjf ../rdump-$version.tar.bz2 rdump-$version || error_cleanup "Failed to create tar file."

cleanup

case $OSTYPE in
        linux*)
                sha=`sha1sum rdump-$version.tar.bz2 |  awk '{ print $1 }'`
                ;;
        freebsd*)
                sha=`sha1  rdump-$version.tar.bz2 |  awk '{ print $5 }'`
                ;;
esac
echo $sha > rdump-$version.tar.bz2.sha1

info "rdump distribution created successfully."
info "SHA1sum: $sha"
