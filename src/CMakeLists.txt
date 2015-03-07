include(${QT_USE_FILE})

set(rpmdownloader_SRCS
  main.cpp
  mainwindow.cpp
  rpmdownloaderwidget.cpp
  profilesettingsdialog.cpp
  repositoryprofile.cpp
  rpm.cpp
  rpmnamedialog.cpp
  rpmdownloadersettings.cpp
  downloadprogressdialog.cpp
  rpmdownloadersettingsdialog.cpp
  plainrepositorycontentdownloader.cpp
  yumrepositorycontentdownloader.cpp
  abstractcontentdownloader.cpp
  yumrepomddomparser.cpp
  checksumcheck.cpp
  yumcachebuilder.cpp
  yumuncompressmetafiles.cpp
  yumcachebuilderthread.cpp
  yumdepsolver.cpp
  rddatabasehandler.cpp
  rddatabaseinserter.cpp
  packagemetadata.cpp
  repositorysqlitecontentlister.cpp
  package.cpp
  rdpackagelisterthread.cpp
  yumxmlmetaparser.cpp
  searchfieldlineedit.cpp
  packagearchdetailswidget.cpp
  packagedetailsdialog.cpp
  yumfileconstants.cpp
  asynccurlhandle.cpp
)

SET(rpmdownloader_MOC_HDRS
  mainwindow.h
  rpmdownloaderwidget.h
  profilesettingsdialog.h
  repositoryprofile.h
  rpmnamedialog.h  
  downloadprogressdialog.h
  rpmdownloadersettingsdialog.h
  plainrepositorycontentdownloader.h
  yumrepositorycontentdownloader.h
  abstractcontentdownloader.h
  checksumcheck.h
  yumcachebuilder.h
  yumuncompressmetafiles.h
  yumcachebuilderthread.h
  rddatabasehandler.h
  repositorysqlitecontentlister.h
  searchfieldlineedit.h
  packagearchdetailswidget.h
  asynccurlhandle.h
)

set(rpmdownloader_UIS
  profilesettingsdialog.ui
  rpmnamedialog.ui
  downloadprogressdialog.ui
  rpmdownloadersettingsdialog.ui
  packagearchdetailswidget.ui
)

SET(rpmdownloader_RCS
  rpmdownloader.qrc
)

SET(rpmdownloader_TRANS
  rpmdownloader_de.ts
)

QT4_ADD_RESOURCES(rpmdownloader_RC_SRCS ${rpmdownloader_RCS})
QT4_WRAP_UI(rpmdownloader_UI_HDRS ${rpmdownloader_UIS})
QT4_WRAP_CPP(rpmdownloader_MOC_SRCS ${rpmdownloader_MOC_HDRS})
QT4_ADD_TRANSLATION(rpmdownloader_QM ${TRANS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_executable(rpmdownloader ${rpmdownloader_SRCS} ${rpmdownloader_MOC_SRCS} ${rpmdownloader_RC_SRCS} ${rpmdownloader_UI_HDRS} ${rpmdownloader_QM})

target_link_libraries(rpmdownloader 
  ${QT_LIBRARIES}
  ${CURL_LIBRARIES}
)
