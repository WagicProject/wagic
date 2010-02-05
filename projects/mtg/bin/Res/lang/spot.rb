#!/usr/bin/ruby -w

require 'iconv'
c = Iconv.new('UTF-8', 'WINDOWS-1252')
line = 0
while l = gets
  line += 1
  if l.match(/[\x80-\xFF]/)
    puts line.to_s + " : " + c.iconv(l.gsub(/([\x80-\xFF])/, '[31m\\1[0m'))
  end
end
