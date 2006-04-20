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
# isn't there a more easy way to do this?
  all = line.chomp.scan(/(.*?) (.*?) (.*?) (.*?) (.*?) (.*)/)
  mode  = all[0][0]
  uid   = all[0][1]
  gid   = all[0][2]
  psize = all[0][3]
  fsize = all[0][4]
  path  = all[0][5]

  # parse what we've got again
  dump = mode[0,1]
  mode = mode[1..-1]
  bits = mode.to_i & S_MMASK

  type = REG
  type = DIR if mode.to_i & S_ISDIR == S_ISDIR
  type = LNK if mode.to_i & S_ISLNK == S_ISLNK

  if dump == "+" then
          case type
          when REG
                  STDOUT.print "REG ", mode," ", bits,"[", path,"]\n"
          when LNK
                  STDOUT.print "LNK ", mode, " ",bits,"[", path,"]\n"
          when DIR 
                  STDOUT.print "DIR ", mode," ", bits,"[", path,"]\n"
          end
  else 
          STDOUT.print "move","\n"
  end
end
