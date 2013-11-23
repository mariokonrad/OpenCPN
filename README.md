Overview
========

a.  Opencpn was built with the following objectives in mind.

    i.   Intended use as primary navigation interface for vessels
         with full-time helm-visible navigational suites.
         Other tools may be better for offline route planning, tide
         and current prediction, online logging, etc.
    ii.  Quick startup and shutdown.
    iii. Those and only those toolbar buttons really needed for
         daily operation.
    iv.  Portability, thus wxWidgets core components.  Currently
         tested and in production use on W98, XP, OS X, and Linux.
    v.   Conventional ( i.e. popular and modern ) chart format
         support.  In the real world, this means BSB format raster
         charts, and S57ENC format vector charts.



        And, of course, opencpn is all GPL'ed (or equivalent)
                        Open Source code.


    Personal Note:
    Opencpn is in primary daily use as the navigation package aboard
    M/V Dyad, a 48 ft trawler yacht cruising from Newfoundland to the
    Bahamas yearly.


Notes
=====

The purpose of this repository is to achieve a refactored version
of the OpenCPN navigation software.


Library Notes & Requirements
============================

a.  OpenCPN S57 ENC support works best with access to the OpenGL Utility
library GLU.  GLU is used to tesselate large polygon areas into small
triangles for faster display rendering.  If you cannot use, or do not have
the OpenGL Utility library, you may choose to build OpenCPN with internal
tesselator support.  The internal tesselator is sub-optimal compared to GLU,
but it does work, if somewhat slower. OpenGLU is better.
See the Build Notes section for applicable tesselator configuration options.


b.  OpenCPN requires WxWidgets Version 2.8.8 or greater.  It has been tested
with the following wxWidgets hosts:

         GTK2                 (__WXGTK__)
         MSWindows            (__WXMSW__)
         MAC OSx              (__WXOSX__)



Platform Specific Build Notes
=============================



Build OpenCPN
=============

Opencpn uses the cmake system, so...

        cd {wherever the opencpn base directory is}
        mkdir build
        cd build
        cmake ../

        make

        su, <password>

        make install


File and Directory Permissions under Linux
==========================================

It is sufficient for all other directories in /usr/share/opencpn
to have permissions 0755, i.e. exec/searchable and readable by all.


Support File Locations
======================

a.  Opencpn requires numerous auxiliary data files.  These files
are installed by the installer into the following locations by default:

      Linux   - /usr/local/share/opencpn/
      Windows - \Program Files\opencpn\
      Mac     - /Users/YourUserName/openCPNfiles/

The following directories exist within the above:
         .../bitmaps                     - self evident
         .../tcdata                      - tide and current location data
         .../s57data                     - data files for S57ENC support
         .../wvsdata                     - World Vector Shoreline data

b.  Opencpn config files are expected in the following locations:

      Linux   - ~/.opencpn/opencpn.conf
      Windows - \Program Files\opencpn\opencpn.ini

The installer will place nice default files for your use.  The first
execution of opencpn will update as needed.  If for some reason the
config file is not found, opencpn will offer to create a useable
starting configuration.


Serial Port GPS/AIS Data Input and Autopilot Output
===================================================


a.  LINUX
Opencpn runs at user privilege.  This means that in order to
read GPS input data and/or write autopilot output data, the serial
devices to be used must exhibit read and write permission for the
user in question.  For linux, these devices are created at startup.
Typically, the devices as created are owned by root, with additional
specific group (e.g. "uucp") r/w access,  i.e. permissions are 0660.

This configuration WILL NOT WORK for OpenCPN unless the user happens
to belong to the group under which the devices were created,
typically "uucp".  Not likely...

For the more general case, you must ensure that device permissions
will enable opencpn to read and write serial devices without root
privileges.  There are several ways to do this.

On a Linux with udev, check the files in /etc/udev/rules.d to
ensure that /dev/tty* devices are all created with the same group
and with 0666 permissions.  More generally, you may need to run mknod
or MAKEDEV as root to create a properly permissioned serial device before
executing opencpn.  For example:

      linux# mknod -m 666 /dev/ttyS0 c 4 64

If you use USB serial port adapters and your system has the Linux
hotplug facility installed, Todo............

Test your GPS input.  At user privilege,

      linux$ stty -F /dev/ttyXXX ispeed 4800
      linux$ cat </dev/ttyXXX

replace ttyXXX with the filename of the port.  This will probably be
either /dev/ttyUSB0 or /dev/ttyS0.  When you run this command, you
should see text lines beginning with $ come to stdout (possibly after
a short initial burst of binary garbage).  If you don't see this, you
may have OS-level problems with your serial support, but more likely
have the wrong device or permissions.  Look again.


