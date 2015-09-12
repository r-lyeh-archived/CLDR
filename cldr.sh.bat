#/bin/bash 2>nul || goto :windows
# - rlyeh, public domain

# bash
echo Bash
git clone https://github.com/r-lyeh/apathy
git clone https://github.com/dropbox/json11
git clone https://github.com/unicode-cldr/cldr-core
git clone https://github.com/unicode-cldr/cldr-dates-modern
git clone https://github.com/unicode-cldr/cldr-localenames-modern
git clone https://github.com/unicode-cldr/cldr-misc-modern
git clone https://github.com/unicode-cldr/cldr-numbers-modern
git clone https://github.com/unicode-cldr/cldr-segments-modern
g++ cldr.cc -O3 -std=c++11
exit

:windows
@echo off
echo Windows
git clone https://github.com/r-lyeh/apathy
git clone https://github.com/dropbox/json11
git clone https://github.com/unicode-cldr/cldr-core
git clone https://github.com/unicode-cldr/cldr-dates-modern
git clone https://github.com/unicode-cldr/cldr-localenames-modern
git clone https://github.com/unicode-cldr/cldr-misc-modern
git clone https://github.com/unicode-cldr/cldr-numbers-modern
git clone https://github.com/unicode-cldr/cldr-segments-modern
cl cldr.cc /Ox /Oy /MT /DNDEBUG
exit /b
