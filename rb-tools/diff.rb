#!/usr/bin/ruby

# diff 2 normal files. 
# if the files are binary copy it

NONE = 0
DIFF = 1
ERR_OR_BIN = 2

`diff -u #{ARGV[0]} #{ARGV[1]} > file`
status = $?.exitstatus
File.unlink("file")

case status
  when NONE
    # nothing to do
    print "none\n"
  when DIFF
    # copy the entire file
    print "differ\n"
  when ERR_OR_BIN
    # hmm error, copy the entire file
    print "err\n"
end
