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

STDIN.each do |line|
        mode, uid, gid, psize, fsize = line.chomp.split(/ /, 5)
        path = line.chomp

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

        newpath = @path

        if dump == "+" then
                File.chmod(bits, "testfile")
                case type
                when REG
                        STDOUT.print "REG ", mode," ", bits," ", path,"\n"
                when LNK
                        STDOUT.print "LNK ", mode, " ",bits," ", path,"\n"
                when DIR 
                        STDOUT.print "DIR ", mode," ", bits," ", path,"\n"
                end
        else 
                STDOUT.print "move","\n"
        end
end
File.chmod(448, "testfile");
