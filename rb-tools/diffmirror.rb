#!/usr/bin/ruby

# mirror like sh-tools/mirror.sh, but instead of copying the
# whole file, create a diff store that.
# For binary files the whole file is stored again, just
# like mirror.sh

# defines 
S_MMASK = 4095
S_ISDIR = 16384 
S_ISLNK = 40960 
REG = 0
DIR = 1
LNK = 2

backupdir="/tmp/storage/200604"
Dir.mkdir "/tmp/storage" if ! test(?d, "/tmp/storage")
Dir.mkdir "/tmp/storage/200604" if ! test(?d, "/tmp/storage/200604")

File.umask(0000)
STDIN.each do |line|
  mode, uid, gid, psize, fsize, path = line.chomp.split(/ /)

  # reparse
  dump = mode[0,1]
  mode = mode[1..-1]
  bits = mode.to_i & S_MMASK
  bits = sprintf("%o", bits).to_i

  type = REG
  type = DIR if mode.to_i & S_ISDIR == S_ISDIR
  type = LNK if mode.to_i & S_ISLNK == S_ISLNK

  if (mode && uid && gid && psize && fsize && path) == NIL then
    STDERR.puts "Error\n"
    next
  end

  begin
    stat   = File.lstat(backupdir + path)
    suffix = stat.mtime.to_i # work in epoch
  rescue SystemCallError
    suffix = NIL
    stat   = NIL
  end

  if dump == "+" then
    case type
    when REG
      File.rename(backupdir + path, backupdir + path + suffix) if suffix != NIL
      copy_file(path, backupdir + path, preserve = true, dereference = false)
    when LNK
      File.rename(backupdir + path, backupdir + path + suffix) if suffix != NIL
      copy_file(path, backupdir + path, preserve = true, dereference = false)
    when DIR 
    if suffix != NIL then
      # type changes
      if stat.symlink? or stat.file? then
        File.rename(backupdir + path, backupdir + path + suffix) 
      end
    end
    if ! test(?d, backupdir + path) then
      print bits
      Dir.mkdir(backupdir + path, bits)
    end
    end

    begin
      File.lchown(uid.to_i, gid.to_i, backupdir + path)
    rescue SystemCallError
      print "lchown error", backupdir + path
    end
  else 
    # move it
    File.rename(backupdir + path, backupdir + path + suffix) if suffix != NIL
  end

end
