yaTerm
======

**y**et **a**nother Serial **Term**inal

Description
===========

A simple program that can be used for serial communication. The main goals of this 
project are to replace other serial terminal programs that I'm not quite happy 
with and to learn GUI programming with QML and Qt. As a consequence, this
is a work-in-progress and pretty rough.

Prerequisites
=============

* Qt 5.3.1+
* Appropriate compiler such as GCC 4.8+

The easiest way to get all the prerequisites is to download the [Qt SDK](http://qt-project.org/downloads).

Developing
==========

Nothing too special about development; follows the standard Qt development process. I personally like to use [Qt Creator](http://qt-project.org/wiki/Category:Tools::QtCreator) for an IDE (also available as part of the [Qt SDK](http://qt-project.org/downloads)).

Building
========

* Clone the repository using `git clone https://github.com/wgraba/yaTerm.git`

Linux
-----

This follows a pretty standard Qt build; more information can be found in the [Qt Docs](http://qt-project.org/doc/).

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

TBD. I have done some preliminary testing and it looks good so far. Refer to the [Qt Docs](http://qt-project.org/doc/) for more information; it should build like a regular Qt project on Windows.
