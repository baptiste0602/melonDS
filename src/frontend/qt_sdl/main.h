/*
    Copyright 2016-2022 melonDS team

    This file is part of melonDS.

    melonDS is free software: you can redistribute it and/or modify it under
    the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.

    melonDS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with melonDS. If not, see http://www.gnu.org/licenses/.
*/

#ifndef MAIN_H
#define MAIN_H

#include <QApplication>
#include <QThread>
#include <QWidget>
#include <QWindow>
#include <QMainWindow>
#include <QImage>
#include <QActionGroup>
#include <QTimer>
#include <QMutex>

#include <QOffscreenSurface>
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLShaderProgram>

#include "FrontendUtil.h"

class EmuThread : public QThread
{
    Q_OBJECT
    void run() override;

public:
    explicit EmuThread(QObject* parent = nullptr);

    void initOpenGL();
    void deinitOpenGL();

    void changeWindowTitle(char* title);

    // to be called from the UI thread
    void emuRun();
    void emuPause();
    void emuUnpause();
    void emuStop();
    void emuFrameStep();

    bool emuIsRunning();
    bool emuIsActive();

    int FrontBuffer = 0;
    QMutex FrontBufferLock;

    GLsync FrontBufferReverseSyncs[2] = {nullptr, nullptr};
    GLsync FrontBufferSyncs[2] = {nullptr, nullptr};

signals:
    void windowUpdate();
    void windowTitleChange(QString title);

    void windowEmuStart();
    void windowEmuStop();
    void windowEmuPause();
    void windowEmuReset();
    void windowEmuFrameStep();

    void windowLimitFPSChange();

    void screenLayoutChange();

    void windowFullscreenToggle();

    void swapScreensToggle();
    void cycleScreenLayout();
    void cycleScreenSizing();
    void cycleScreenRotation();

private:
    volatile int EmuStatus;
    int PrevEmuStatus;
    int EmuRunning;
    int EmuPause;

    QOffscreenSurface* oglSurface;
    QOpenGLContext* oglContext;
};


class ScreenHandler
{
    Q_GADGET

public:
    virtual ~ScreenHandler() {}
    QTimer* setupMouseTimer();
    void updateMouseTimer();
    QTimer* mouseTimer;
    QSize screenGetMinSize(int factor);

protected:
    void screenSetupLayout(int w, int h);

    void screenOnMousePress(QMouseEvent* event);
    void screenOnMouseRelease(QMouseEvent* event);
    void screenOnMouseMove(QMouseEvent* event);

    void screenHandleTablet(QTabletEvent* event);
    void screenHandleTouch(QTouchEvent* event);

    float screenMatrix[Frontend::MaxScreenTransforms][6];
    int screenKind[Frontend::MaxScreenTransforms];
    int numScreens;

    bool touching;

    void showCursor();
};


class ScreenPanelNative : public QWidget, public ScreenHandler
{
    Q_OBJECT

public:
    explicit ScreenPanelNative(QWidget* parent);
    ~ScreenPanelNative();

protected:
    void paintEvent(QPaintEvent* event) override;

    void resizeEvent(QResizeEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void tabletEvent(QTabletEvent* event) override;
    bool event(QEvent* event) override;
private slots:
    void onScreenLayoutChanged();

private:
    void setupScreenLayout();

    QImage screen[2];
    QTransform screenTrans[Frontend::MaxScreenTransforms];
};


class ScreenPanelGL : public QOpenGLWidget, public ScreenHandler, protected QOpenGLFunctions_3_2_Core
{
    Q_OBJECT

public:
    explicit ScreenPanelGL(QWidget* parent);
    ~ScreenPanelGL();

protected:
    void initializeGL() override;

    void paintGL() override;

    void resizeEvent(QResizeEvent* event) override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;

    void tabletEvent(QTabletEvent* event) override;
    bool event(QEvent* event) override;
private slots:
    void onScreenLayoutChanged();

private:
    void setupScreenLayout();

    QOpenGLShaderProgram* screenShader;
    GLuint screenVertexBuffer;
    GLuint screenVertexArray;
    GLuint screenTexture;
};

class MelonApplication : public QApplication
{
    Q_OBJECT

public:
    MelonApplication(int &argc, char** argv);
    bool event(QEvent* event) override;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    bool hasOGL;
    QOpenGLContext* getOGLContext();

    bool preloadROMs(QString filename, QString gbafilename);

    void onAppStateChanged(Qt::ApplicationState state);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

signals:
    void screenLayoutChange();

private slots:
    void onOpenFile();
    void onClickRecentFile();
    void onClearRecentFiles();
    void onBootFirmware();
    void onInsertCart();
    void onEjectCart();
    void onInsertGBACart();
    void onInsertGBAAddon();
    void onEjectGBACart();
    void onSaveState();
    void onLoadState();
    void onUndoStateLoad();
    void onImportSavefile();
    void onQuit();

    void onPause(bool checked);
    void onReset();
    void onStop();
    void onFrameStep();
    void onEnableCheats(bool checked);
    void onSetupCheats();
    void onCheatsDialogFinished(int res);
    void onROMInfo();
    void onRAMInfo();
    void onOpenTitleManager();

    void onOpenEmuSettings();
    void onEmuSettingsDialogFinished(int res);
    void onOpenPowerManagement();
    void onOpenInputConfig();
    void onInputConfigFinished(int res);
    void onOpenVideoSettings();
    void onOpenAudioSettings();
    void onOpenFirmwareSettings();
    void onOpenPathSettings();
    void onUpdateAudioSettings();
    void onAudioSettingsFinished(int res);
    void onOpenWifiSettings();
    void onWifiSettingsFinished(int res);
    void onFirmwareSettingsFinished(int res);
    void onPathSettingsFinished(int res);
    void onOpenInterfaceSettings();
    void onInterfaceSettingsFinished(int res);
    void onUpdateMouseTimer();
    void onChangeSavestateSRAMReloc(bool checked);
    void onChangeScreenSize();
    void onChangeScreenRotation(QAction* act);
    void onCycleScreenRotation();
    void onChangeScreenGap(QAction* act);
    void onChangeScreenLayout(QAction* act);
    void onCycleScreenLayout();
    void onChangeScreenSwap(bool checked);
    void onChangeScreenSizing(QAction* act);
    void onCycleScreenSizing();
    void onChangeScreenAspect(QAction* act);
    void onChangeIntegerScaling(bool checked);
    void onChangeScreenFiltering(bool checked);
    void onChangeShowOSD(bool checked);
    void onChangeLimitFramerate(bool checked);
    void onChangeAudioSync(bool checked);

    void onTitleUpdate(QString title);

    void onEmuStart();
    void onEmuStop();

    void onUpdateVideoSettings(bool glchange);

    void onFullscreenToggled();

private:
    QStringList currentROM;
    QStringList currentGBAROM;
    QList<QString> recentFileList;
    QMenu *recentMenu;
    void updateRecentFilesMenu();

    bool verifySetup();
    QString pickFileFromArchive(QString archiveFileName);
    QStringList pickROM(bool gba);
    void updateCartInserted(bool gba);

    void createScreenPanel();

    bool pausedManually = false;

    int oldW, oldH;
    bool oldMax;

public:
    QWidget* panel;
    ScreenPanelGL* panelGL;
    ScreenPanelNative* panelNative;

    QAction* actOpenROM;
    QAction* actBootFirmware;
    QAction* actCurrentCart;
    QAction* actInsertCart;
    QAction* actEjectCart;
    QAction* actCurrentGBACart;
    QAction* actInsertGBACart;
    QAction* actInsertGBAAddon[1];
    QAction* actEjectGBACart;
    QAction* actImportSavefile;
    QAction* actSaveState[9];
    QAction* actLoadState[9];
    QAction* actUndoStateLoad;
    QAction* actQuit;

    QAction* actPause;
    QAction* actReset;
    QAction* actStop;
    QAction* actFrameStep;
    QAction* actEnableCheats;
    QAction* actSetupCheats;
    QAction* actROMInfo;
    QAction* actRAMInfo;
    QAction* actTitleManager;

    QAction* actEmuSettings;
    QAction* actPowerManagement;
    QAction* actInputConfig;
    QAction* actVideoSettings;
    QAction* actAudioSettings;
    QAction* actWifiSettings;
    QAction* actFirmwareSettings;
    QAction* actPathSettings;
    QAction* actInterfaceSettings;
    QAction* actSavestateSRAMReloc;
    QAction* actScreenSize[4];
    QActionGroup* grpScreenRotation;
    QAction* actScreenRotation[4];
    QActionGroup* grpScreenGap;
    QAction* actScreenGap[6];
    QActionGroup* grpScreenLayout;
    QAction* actScreenLayout[4];
    QAction* actScreenSwap;
    QActionGroup* grpScreenSizing;
    QAction* actScreenSizing[6];
    QAction* actIntegerScaling;
    QActionGroup* grpScreenAspectTop;
    QAction** actScreenAspectTop;
    QActionGroup* grpScreenAspectBot;
    QAction** actScreenAspectBot;
    QAction* actScreenFiltering;
    QAction* actShowOSD;
    QAction* actLimitFramerate;
    QAction* actAudioSync;
};

#endif // MAIN_H
