EXTERNAL SOURCES
================

This directory contains external sources, which are used by OpenCPN.
For ease of use, they are in this repository, stripped of what is
not needed.

External sources should remain as unpatched as possible, outside the
projects own source.


pugixml-1.2
-----------

License: MIT (http://www.opensource.org/licenses/mit-license.html)

Homepage:

	http://pugixml.org/

Download:

	http://pugixml.googlecode.com/files/pugixml-1.2.tar.gz

svn:

	svn checkout http://pugixml.googlecode.com/svn/tags/latest pugixml

git:

	git clone https://github.com/zeux/pugixml.git

Notes:
- From the original package, the directories src and scripts are
  unpacked, but untouched.


tinyxml-2.6.2
-------------

License:

Homepage:

Download:

Notes:
- Altered CMakeLists.txt


libtcd-2.2.5
------------

License: Public Domain

Homepage:

	http://www.flaterco.com/xtide/libtcd.html

Download:

	ftp://ftp.flaterco.com/xtide/libtcd-2.2.5-r3.tar.bz2

Notes:
- Added a CMakeLists.txt file

