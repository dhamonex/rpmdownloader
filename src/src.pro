SOURCES += main.cpp \
 mainwindow.cpp \
 rpmdownloaderwidget.cpp \
 profilesettingsdialog.cpp \
 repositoryprofile.cpp \
 rpm.cpp \
 rpmnamedialog.cpp \
 rpmdownloadersettings.cpp \
 downloadprogressdialog.cpp \
 rpmdownloadersettingsdialog.cpp \
 plainrepositorycontentdownloader.cpp \
 yumrepositorycontentdownloader.cpp \
 abstractcontentdownloader.cpp \
 yumrepomddomparser.cpp \
 checksumcheck.cpp \
 yumcachebuilder.cpp \
 yumuncompressmetafiles.cpp \
 yumcachebuilderthread.cpp \
 yumdepsolver.cpp \
 rddatabasehandler.cpp \
 rddatabaseinserter.cpp \
 packagemetadata.cpp \
 repositorysqlitecontentlister.cpp \
 package.cpp \
 rdpackagelisterthread.cpp \
 yumxmlmetaparser.cpp \
 searchfieldlineedit.cpp \
 rdhttp.cpp \
 packagearchdetailswidget.cpp \
 packagedetailsdialog.cpp \
 yumfileconstants.cpp
 
TEMPLATE = app
CONFIG += warn_on \
	  thread \
          qt \
 debug
TARGET = ../bin/rpmdownloader

QT += network \
 xml \
 sql

HEADERS += mainwindow.h \
 rpmdownloaderwidget.h \
 profilesettingsdialog.h \
 repositoryprofile.h \
 rdnamespace.h \
 rpm.h \
 rpmnamedialog.h \
 rpmdownloadersettings.h \
 downloadprogressdialog.h \
 rpmdownloadersettingsdialog.h \
 plainrepositorycontentdownloader.h \
 yumrepositorycontentdownloader.h \
 abstractcontentdownloader.h \
 yumrepomddomparser.h \
 checksumcheck.h \
 yumcachebuilder.h \
 yumuncompressmetafiles.h \
 yumcachebuilderthread.h \
 yumfileconstants.h \
 yumdepsolver.h \
 rddatabasehandler.h \
 rddatabaseinserter.h \
 packagemetadata.h \
 repositorysqlitecontentlister.h \
 package.h \
 rdpackagelisterthread.h \
 yumxmlmetaparser.h \
 searchfieldlineedit.h \
 rdhttp.h \
 packagearchdetailswidget.h \
 packagedetailsdialog.h

FORMS += profilesettingsdialog.ui \
 rpmnamedialog.ui \
 downloadprogressdialog.ui \
 rpmdownloadersettingsdialog.ui \
 packagearchdetailswidget.ui

RESOURCES += rpmdownloader.qrc

TRANSLATIONS += rpmdownloader_de.ts

CONFIG -= release

; QMAKE_CXXFLAGS_DEBUG += -pg
; QMAKE_LFLAGS += -pg

