yaTerm
======

**y**et **a**nother Serial **Term**inal

![yaTerm in action](https://lh3.googleusercontent.com/0WPrZf9qU1sOE8bHxiCkF_NCN23_zyr0mDKsQ6rZrFlSnR9zhLaWogUsSp6y1N-ZGgUa0MgZjr9CrVJ3CK8iI09SiIyJrmMVn02pJDTqiXLPg2knPbtOBXERefXSXPPudJ49QorsZU5ZXfAtKiAcdQdWkCK_Tmfm95S9SQm5-iHdLxB_oyqW_0tjcZe8x7c5cgrr4j3PRLIBuEL7EtWGcqe2RBy_DdjNAqpi-b1i4E8VxOGiDF_iRvcSasuqUVH_hY2vl_qhMT602LP9iSxh3DSKgzoSkZwGikI0sW9Jv8v-finksiMh5FXOJ_Rsnj6EZ-KCPM3DzTAc_xepNDdIT5O74LGbIaIOKOkPcUHz_4u6zaz2Xc_VR1NfTm-dNrKyBKlHCurvF4ssMQpSQnHqfrZkVNL7IqHE8hHqKg7kUIf-h72j-AemBHJszCK0D1b85YViVD_JeZi_8yofW8O_3-mrNERmK-b-4OZ_eEtcjKK1eKbAXascY07WV4fWqVoLwq8efQnTaV2srJC9mmb3FSs2azi-Tg7hriECDLuTX_yBw6_LYLQGCC2Q1i39PyKRH1pKuuDE7WwXUl_NUcuAfJ7l1g1e2JkbT61VfeoGfLHrLGlP=w769-h555-no)

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
* Set custom start-of-message and end-of-message text

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

* Qt 5.7+ (Standard Qt + Qt Quick Controls + QtSerialPort)
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
