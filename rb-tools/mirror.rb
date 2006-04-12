#!/usr/bin/ruby

#while $word1, $word2 = STDIN.gets
#        print $word1
#        print $word2
#end

S_MMASK = 4095
S_ISDIR = 16384 
S_ISLNK = 40960 

REG = 0
DIR = 1
LNK = 2

backupdir="/tmp/storage/200604"

STDIN.each do |line|
        mode, uid, gid, psize, fsize, path = line.chomp.split(/ /)

        # parse what we've got again
        dump = mode[0,1]
        mode = mode[1..-1]
        bits = mode.to_i & S_MMASK
        type = REG
        if mode.to_i & S_ISDIR == S_ISDIR then
                type = DIR
        end
        if mode.to_i & S_ISLNK == S_ISLNK then
                type = LNK
        end

        if (mode && uid && gid && psize && fsize && path) == NIL then
                STDERR.puts "Error\n"
        end

        begin
                stat = File.lstat(backupdir + path)
                suffix = sprintf("+%02d.%02d:%02d", stat.mtime.day, 
                        stat.mtime.hour,
                        stat.mtime.min)
        rescue SystemCallError
                suffix = NIL
        end

        case type
        when REG
                print "REG", path, suffix, "\n"
        when DIR 
                print "DIR", path, suffix, "\n"
        when LNK
                print "LNK", path, suffix, "\n"
        end

        


end
