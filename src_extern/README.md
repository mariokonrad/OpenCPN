EXTERNAL SOURCES
================

This directory contains external sources, which are used by OpenCPN.
For ease of use, they are in this repository, stripped of what is
not needed.

External sources should remain as unpatched as possible, outside the
projects own source.


zlib-1.2.8
----------

License: zlib (http://zlib.net/zlib_license.html)

Homepage: http://zlib.net/

Download: http://zlib.net/zlib-1.2.8.tar.gz

git:

	git clone https://github.com/madler/zlib.git

Notes:
- Genuine distribution


bzip2-1.0.6
-----------

License: proprietary (see file: LICENSE)

Homepage: http://www.bzip.org/

Download: http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz

Notes:
- Tarball extracted and contents reduced to the necessary parts.
  All checks and test as well as most of the documentation was removed.
- Added a CMakeLists.txt file


pugixml-1.2
-----------

License: MIT (http://www.opensource.org/licenses/mit-license.html)

Homepage: http://pugixml.org/

Download: http://pugixml.googlecode.com/files/pugixml-1.2.tar.gz

svn:

	svn checkout http://pugixml.googlecode.com/svn/tags/latest pugixml

git:

	git clone https://github.com/zeux/pugixml.git

Notes:
- From the original package, the directories src and scripts are
  unpacked, but untouched.
- CMakeLists.txt extended to print some variables


tinyxml-2.6.2
-------------

License: zlib/libpng (http://opensource.org/licenses/Zlib)

Homepage: http://www.grinninglizard.com/tinyxml/

Download: http://sourceforge.net/projects/tinyxml/

Notes:
- Altered CMakeLists.txt
- CMakeLists.txt extended to print some variables


libtcd-2.2.5
------------

License: Public Domain

Homepage: http://www.flaterco.com/xtide/libtcd.html

Download: ftp://ftp.flaterco.com/xtide/libtcd-2.2.5-r3.tar.bz2

Notes:
- Added a CMakeLists.txt file
- Inserted additional '#ifdef HAVE\_UNISTD\_H' preprocessor directive, necessary because
  Windows does not know ftruncate (see tide\_db.c:delete\_tide\_record).


wxJSON-1.2.1
------------

License: wxWidgets license (http://www.wxwidgets.org/about/newlicen.htm)

Homepage:
- http://luccat.users.sourceforge.net/wxjson/
- http://wxcode.sourceforge.net/components/wxjson/

Download: http://sourceforge.net/projects/wxcode/files/Components/wxJSON/

Notes:
- Added a CMakeLists.txt file


