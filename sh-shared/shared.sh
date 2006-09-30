
# source this file to get some commonly 
# used functions in the rdup-utils.

# show the version of the tool
version() {
        echo "$PROGNAME: @PACKAGE_VERSION@ (rdup-utils)"
}

# echo to standard error
_echo2() {
        echo "** $PROGNAME: $1" >&2
}

