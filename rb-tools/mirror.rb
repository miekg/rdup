#!/usr/bin/ruby

require "fileutils"

S_MMASK = 4095
S_ISDIR = 16384 
S_ISLNK = 40960 

REG = 0
DIR = 1
LNK = 2

backupdir="/tmp/storage/200604"
Dir.mkdir "/tmp/storage" if ! test(?d, "/tmp/storage")
Dir.mkdir "/tmp/storage/200604" if ! test(?d, "/tmp/storage/200604")

STDIN.each do |line|
  all = line.chomp.scan(/(.*?) (.*?) (.*?) (.*?) (.*?) (.*)/)
  mode  = all[0][0]
  uid   = all[0][1]
  gid   = all[0][2]
  psize = all[0][3]
  fsize = all[0][4]
  path  = all[0][5]

  dump = mode[0,1]
  mode = mode[1..-1]
  bits = mode.to_i & S_MMASK

  type = REG
  type = DIR if mode.to_i & S_ISDIR == S_ISDIR
  type = LNK if mode.to_i & S_ISLNK == S_ISLNK 

  begin
    stat = File.lstat(backupdir + path)
    suffix = sprintf("+%02d.%02d:%02d", stat.mtime.day, stat.mtime.hour, stat.mtime.min)
  rescue SystemCallError
    suffix = NIL
    stat = NIL
  end

  STDOUT.print type, mode, bits, "[",path,"]\n"
  next

  if dump == "+" then
    case type
      when REG
        File.rename(backupdir + path, backupdir + path + suffix) if suffix != NIL
        FileUtils.copy_file(path, backupdir + path, preserve = true, dereference = false)
        File.chmod(bits, backupdir + path)
      when LNK
        File.rename(backupdir + path, backupdir + path + suffix) if suffix != NIL
        FileUtils.copy_file(path, backupdir + path, preserve = true, dereference = false)
        #File.chmod(bits, backupdir + path) # we
        #preserve the bits, might not be needed
      when DIR 
        if suffix != NIL then
          if stat.symlink? or stat.file? then
            File.rename(backupdir + path, backupdir + path + suffix) 
          end
        end
        if ! test(?d, backupdir + path) then
          Dir.mkdir(backupdir + path)
          File.chmod(bits, backupdir + path)
        end
      end
      begin
        File.lchown(uid.to_i, gid.to_i, backupdir + path)
      rescue SystemCallError
        print "error"
      end
    else 
      # move it
      File.rename(backupdir + path, backupdir + path + suffix) if suffix != NIL
    end
end
