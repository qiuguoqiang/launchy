TEMPLATE		= app
linux:TARGET 	= launchy
win32:TARGET	= Launchy
macx:TARGET		= Launchy

CONFIG			+= debug_and_release
QT				+= network widgets gui-private
win32:QT		+= winextras
linux:QT		+= x11extras

INCLUDEPATH		+= . \
                ../common

UI_DIR			= .ui
MOC_DIR			= .moc

RESOURCES		+= launchy.qrc
FORMS			= options.ui

SOURCES			= main.cpp \
                AnimationLabel.cpp \
                catalog.cpp \
                catalog_builder.cpp \
                catalog_types.cpp \
                CharLineEdit.cpp \
                CharListWidget.cpp \
                CommandHistory.cpp \
                commandlineparser.cpp \
                Fader.cpp \
                FileSearch.cpp \
                globals.cpp \
                icon_delegate.cpp \
                icon_extractor.cpp \
                InputDataList.cpp \
                options.cpp \
                platform_base_hotkey.cpp \
                plugin_handler.cpp \
                plugin_interface.cpp \
                SettingsManager.cpp \
                ../common/DropListWidget.cpp \
                ../common/FileBrowserDelegate.cpp \
                ../common/FileBrowser.cpp

HEADERS			= main.h \
                AnimationLabel.h \
                catalog.h \
                catalog_builder.h \
                catalog_types.h \
                CharLineEdit.h \
                CharListWidget.h \
                CommandHistory.h \
                commandlineparser.h \
                Fader.h \
                FileSearch.h \
                globals.h \
                icon_delegate.h \
                icon_extractor.h \
                InputDataList.h \
                options.h \
                platform_base.h \
                plugin_handler.h \
                plugin_interface.h \
                SettingsManager.h \
                winfiles.h \
                ../common/FileBrowserDelegate.h \
                ../common/FileBrowser.h \
                ../common/DropListWidget.h
        
TRANSLATIONS	= \
                ../../translations/launchy_fr.ts \
                ../../translations/launchy_nl.ts \
                ../../translations/launchy_zh.ts \
                ../../translations/launchy_es.ts \
                ../../translations/launchy_de.ts \
                ../../translations/launchy_ja.ts \
                ../../translations/launchy_zh_TW.ts \
                ../../translations/launchy_rus.ts

DESTDIR		 	= ../../bin/
DLLDESTDIR		= ../../bin/

linux {
    ICON = Launchy.ico
    SOURCES += ../platforms/unix/platform_unix.cpp \
        ../platforms/unix/platform_unix_util.cpp \
        ../platforms/unix/platform_x11_hotkey.cpp
    HEADERS += ../platforms/unix/platform_unix.h \
        ../platforms/unix/platform_unix_util.h \
        ../platforms/unix/platform_x11_hotkey.h \
        platform_base_hotkey.h \
        platform_base_hottrigger.h
    LIBS += -lX11
    PREFIX = /usr
    DEFINES += SKINS_PATH=\\\"$$PREFIX/share/launchy/skins/\\\" \
        PLUGINS_PATH=\\\"$$PREFIX/lib/launchy/plugins/\\\" \
        PLATFORMS_PATH=\\\"$$PREFIX/lib/launchy/\\\"
    if(!debug_and_release|build_pass) { 
        CONFIG(debug, debug|release):DESTDIR = ../debug/
        CONFIG(release, debug|release):DESTDIR = ../release/
    }
    target.path = $$PREFIX/bin/
    skins.path = $$PREFIX/share/launchy/skins/
    skins.files = ../../skins/*
    icon.path = $$PREFIX/share/pixmaps
    icon.files = ../misc/Launchy_Icon/launchy_icon.png
    desktop.path = $$PREFIX/share/applications/
    desktop.files = ../linux/launchy.desktop
    INSTALLS += target \
        skins \
        icon \
        desktop
}
win32 { 
    ICON = Launchy.ico
    if(!debug_and_release|build_pass):CONFIG(debug, debug|release):CONFIG += console
    SOURCES += ../platforms/win/platform_win.cpp \
        ../platforms/win/platform_win_hotkey.cpp \
        ../platforms/win/platform_win_util.cpp \
        ../platforms/win/WinIconProvider.cpp \
        ../platforms/win/minidump.cpp
    HEADERS += ../platforms/win/WinIconProvider.h \
        platform_base_hotkey.h \
        platform_base_hottrigger.h \
        ../platforms/win/platform_win.h \
        ../platforms/win/platform_win_util.h \
        ../platforms/win/minidump.h
    CONFIG += embed_manifest_exe
    INCLUDEPATH += c:/boost/
    RC_FILE = ../win/launchy.rc
	LIBS += shell32.lib \
		user32.lib \
		gdi32.lib \
		ole32.lib \
		comctl32.lib \
		advapi32.lib \
		userenv.lib \
        netapi32.lib
    DEFINES = VC_EXTRALEAN \
        WIN32 \
        _UNICODE \
        UNICODE \
        WINVER=0x0600 \
        _WIN32_WINNT=0x0600 \
        _WIN32_WINDOWS=0x0600 \
        _WIN32_IE=0x0700
    if(!debug_and_release|build_pass) {
        CONFIG(debug, debug|release):DESTDIR = ../debug/
        CONFIG(release, debug|release):DESTDIR = ../release/
    }
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG
}
macx { 
    ICON = ../misc/Launchy_Icon/launchy_icon_mac.icns
    SOURCES += ../platforms/mac/platform_mac.cpp \
        ../platforms/mac/platform_mac_hotkey.cpp
    HEADERS += ../platforms/mac/platform_mac.h \
        ../platforms/mac/platform_mac_hotkey.h \
        platform_base_hotkey.h \
        platform_base_hottrigger.h
    if(!debug_and_release|build_pass) { 
        CONFIG(debug, debug|release):DESTDIR = ../debug/
        CONFIG(release, debug|release):DESTDIR = ../release/
    }
    INCLUDEPATH += /opt/local/include/
    LIBS += -framework \
        Carbon
    CONFIG(debug, debug|release):skins.path = ../debug/Launchy.app/Contents/Resources/skins/
    CONFIG(release, debug|release):skins.path = ../release/Launchy.app/Contents/Resources/skins/
    skins.files = 
    skins.extra = rsync -arvz ../../skins/   ../release/Launchy.app/Contents/Resources/skins/   --exclude=\".svn\"
    CONFIG(debug, debug|release):translations.path = ../debug/Launchy.app/Contents/MacOS/tr/
    CONFIG(release, debug|release):translations.path = ../release/Launchy.app/Contents/MacOS/tr/
    translations.files = ../../translations/*.qm
    translations.extra = lupdate \
        src.pro \
        ; \
        lrelease \
        src.pro
    dmg.path = ../release/
    dmg.files = 
    dmg.extra = cd \
        ../mac \
        ; \
        bash \
        deploy; \
        cd \
        ../src
    INSTALLS += skins \
        translations \
        dmg
}