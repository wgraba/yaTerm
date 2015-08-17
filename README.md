yaTerm
======

**y**et **a**nother Serial **Term**inal

![yaTerm in action](https://lh3.googleusercontent.com/vea0VUPOtPPdU1lNEF3SkDm4iWO7GIo5pOxhQ1huYlE=w714-h531-no)

Description
===========

A simple program that can be used for serial communication. The main goals of this 
project is to replace other serial terminal programs that I'm not quite happy 
with and to learn GUI programming with QML and Qt. As a consequence, this
is a work-in-progress and pretty rough.

Features
========

* Separate input and output windows
* Input history (use 'up' and 'down' keys with input window focused)
* Simple command-line interface (type "/help" into input window)
* Settable auto-scrolling of output window
* Visual cues to help distinguish input from output, commands from command response, errors, etc.

Installing
==========

This is dependent on the OS.

Windows
-------

* [Build](#building) from source or download pre-built binaries at 
[my Bintray project](https://bintray.com/wgraba/windows/yaTerm/view).
* Currently, there is no installer; unzip content to desired directory. Install Visual Studio C++
  redistributable (get from Microsoft web site or from unzipped contents).
* Run yaTerm.exe and enjoy!

Linux
-----

Currently, the only way is to [build](#building) from source. I'm hoping to create packages for popular distros
soon.

Mac OSX
-------

TBD. The best starting point right now is to [build](#building) from source.

Developing
==========

Nothing too special about development; follows the standard Qt development process. I personally 
like to use [Qt Creator](http://qt-project.org/wiki/Category:Tools::QtCreator) for an IDE (also 
available as part of the [Qt SDK](http://qt-project.org/downloads)).

Prerequesites
-------------

* Qt 5.4+ (Standard Qt + Qt Quick Controls + QtSerialPort)
* Appropriate compiler such as GCC 4.8+

The easiest way to get all the prerequisites is to download the [Qt SDK](http://qt-project.org/downloads).


Building
========

* Clone the repository using `git clone https://github.com/wgraba/yaTerm.git`

The rest of the building process is dependent on the architecture.

Linux
-----

This follows a pretty standard Qt build; more information can be found in the [Qt Docs](http://doc.qt.io/).

* Run qmake (debug; lose the `CONFIG+=` stuff for release). This is shown running in the root project path, but this can be run from any desired path as long the path to `yaTerm.pro` is given (useful for separating build and source files)

```
qmake yaTerm.pro -r -spec linux-g++ CONFIG+=debug CONFIG+=declarative_debug CONFIG+=qml_debug
```

* Run make

```
make
```

Windows
-------

TBD. 

I have done some preliminary testing and it looks good so far. Refer to the [Qt Docs](http://qt-project.org/doc/) for more information; it should build like a regular Qt project on Windows; just have to change the `-spec` option.

OSX
---

TBD.
