#!/usr/bin/env ruby
# Deletes/Prunes old backups
# Use at your own risk
#
# Author: Matthias-Christian Ott (ott@mirix.org)
# License: GPLv3

require 'optparse'
require 'date'
require 'fileutils'

options = { :limit => 30 }
OptionParser.new do |opts|
  opts.on '-l', '--limit',
    "set archive limit (default: #{options[:limit]})" do |v|
    options[:limit] = v.to_i
  end
end.parse!

if ARGV.size < 1
  warn "no backup directory given"
  exit 1
elsif ARGV.size > 1
  warn "more than one backup directory given"
  exit 1
end

today = DateTime.strptime Time.now.strftime('%Y%m/%d'), '%Y%m/%d'
limit = today - options[:limit]

Dir.chdir ARGV[0]
Dir.glob '*/*' do |entry|
  begin
    date = DateTime.strptime entry, '%Y%m/%d'
  rescue
    next
  end
  if date < limit
    FileUtils.rm_r entry
  end
  topdir = date.strftime '%Y%m'
  entries = Dir.entries(topdir) - ['.', '..']
  p entries
  if entries.empty?
    FileUtils.rmdir topdir
  end
end
