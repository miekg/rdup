#!/bin/bash

# updates a hardlinked backup
# licensed under the GPL version 3
# Copyright Miek Gieben, 2007 - 2009
# rewritten for rdup-up + rdup-tr
#{exec_prefix} XXX todo?

echo2() {
    echo "** $PROGNAME: $@" >&2
}

usage() {
        cat << HELP
$PROGNAME [+N] DIR [DIR ...] DEST

This is a wrapper around rdup, rdup-tr and rdup-up

DIR  - directories to back up
+N   - Look N days back for previous backups, defaults to 8
DEST - where to store the backup. This can be:
	ssh://user@host/directory (note: no colon after the hostname
	ssh://host/directory
	file:///directory (note: 3 slashes)
	/directory
	directory

OPTIONS:
 -k KEYFILE encrypt all files: rdup-tr -Pmcrypt,-fKEYFILE,-c
 -g	    encrypt all files: rdup-tr -Pgpg,??
 -z         compress all files: rdup-tr -Pgzip,-c,-f
 -E FILE    use FILE as an exclude list
 -f         force a full dump
 -v         echo the files processed to stderr
 -x         pass -x to rdup
 -h         this help
 -V         print version
HELP
}

PROGNAME=$0
NOW=`date +%Y%m/%d`
DAYS=8
ssh=""
trans=""
l=" -l"
c=" -c"
enc=false
etc=~/.rdup
force=false

while getopts "E:k:vfgzxhV" o; do
        case $o in
		E)
                if [[ -z "$OPTARG" ]]; then
                        echo2 "-E needs an argument"
                        exit 1
                fi
                E=" -E $OPTARG "
                ;;
                k)
                if [[ -z "$OPTARG" ]]; then
                        echo2 "-k needs an argument"
                        exit 1
                fi
                if [[ ! -r "$OPTARG" ]]; then
                        echo2 "Cannot read keyfile \`$OPTARG': failed"
                        exit 1
                fi
                trans="$trans -Pmcrypt,-f$OPTARG,-c"
		if $enc; then
			echo2 "Encryption already set"
			exit 1
		fi
		enc=true
		c=""   # rdup-tr expects a list of filenames, so reset -c
                ;;
                z) trans="$trans -Pgzip,-f,-c"
		if $enc; then
			echo2 "Select compression first, then encryption"
			exit 1
		fi
		c=""
                ;;
		g) trans="$trans -Pgpg,--default-recipient-self"
		if $enc; then
			echo2 "Encryption already set"
			exit 1
		fi
		enc=true
		c=""
		;;
                f) force=true;;
                a) ;;
                v) OPT=" $OPT -v ";;
                x) x=" -x ";;
                h) usage && exit;;
                V) version && exit;;
                \?) echo2 "Invalid option: $OPTARG"; exit 1;;
        esac
done
shift $((OPTIND - 1))

if [[ ${1:0:1} == "+" ]]; then
        DAYS=${1:1}
        if [[ $DAYS -lt 1 || $DAYS -gt 99 ]]; then
                echo2 "+N needs to be a number [1..99]"
                exit 1
        fi
        shift
else
        DAYS=8
fi

[[ $# -eq 0 ]] && usage && exit

i=1; last=$#; DIRS=
while [[ $i -lt $last ]]; do
	DIRS="$DIRS $1"
	shift
	((i=$i+1))
done
# rdup [options] source destination
#dest="ssh://elektron.atoom.net/directory"
#dest="ssh://elektron.atoom.net/directory/var/"
#dest="file:///var/backup"
#dest="/var/backup"
#dest="ssh://miekg@elektron.atoom.net/directory"

dest=$1
if [[ ${dest:0:6} == "ssh://" ]]; then
	rest=${dest/ssh:\/\//}
	u=${rest%%@*}
	rest=${rest/$u@/}
	h=`echo $rest | cut -s -f1 -d/`
	BACKUPDIR=${rest/$h/}

	c="-c"
	l=""	# enable race checking in rdup
	if [[ -z $u ]]; then
		ssh=" ssh -x $h"
	else
		ssh=" ssh -x $u@$h"
	fi
fi
if [[ ${dest:0:7} == "file://" ]]; then
	rest=${dest/file:\/\//}
	BACKUPDIR=$rest
fi
[[ ${dest:0:1} == "/" ]] && BACKUPDIR=$dest

# no hits above, assume relative filename
[[ -z $BACKUPDIR ]] && BACKUPDIR=$PWD/$dest

# change all / to _ to make a valid filename
STAMP=$etc/timestamp.${HOSTNAME}.${dest//\//_}
LIST=$etc/list.${HOSTNAME}.${dest//\//_}

[[ ! -d $etc ]] && mkdir $etc

# create our command line
if [[ -z $ssh ]]; then
        pipe="rdup-tr$trans | rdup-up$OPT $BACKUPDIR/$NOW"
else
        pipe="rdup-tr$trans | $ssh rdup-up$OPT $BACKUPDIR/$NOW"
fi
cmd="rdup$E$x$l -N $STAMP $LIST $DIRS | $pipe"

if ! $force; then
        if [[ -z $ssh ]]; then
		/usr/lib/rdup/rdup-ln.sh -l $DAYS $BACKUPDIR
                purpose=$?
        else
                $ssh "/usr/lib/rdup/rdup-ln.sh -l $DAYS $BACKUPDIR"
                purpose=$?
        fi
else
        purpose=1
fi
case $purpose in
        0) echo2 "INCREMENTAL DUMP" ;;
        1)
        echo2 "FULL DUMP"
        rm -f $LIST
        rm -f $STAMP ;;
        *)
        echo2 "Illegal return code from rdup-ln.sh"
        exit 1 ;;
esac
# execute the backup command
echo2 "Executing: ${cmd}"
eval ${cmd}
