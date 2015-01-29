#ifndef JABDAH_H
#define JABDAH_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QApplication>
#include <QSystemTrayIcon>

#ifdef _WIN32
    #include <QtWinExtras>
#endif

#include "about.h"
#include "preferencesdialog.h"
#include "settings.h"

namespace Ui {
class Jabdah;
}

class Playlist; //Forward declaration

class Jabdah : public QMainWindow
{
    Q_OBJECT
    

public:
    explicit Jabdah(QWidget *parent = 0);

    bool WindowStatus();
    void ScanDir(QTreeWidgetItem *item);
    void load_Song(Playlist *currentPlaylist, bool Autoplay);
    void set_Player();
    void changeSongInfoText(QString text);
    void changeSongTimeText(QString text);
    ~Jabdah();
    
private slots:
    void on_actionFile_triggered();

    void on_actionQuit_triggered();

    void on_playButton_clicked();

    virtual void closeEvent(QCloseEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);

    void on_stopButton_clicked();

    void on_positionBar_sliderPressed();

    void on_positionBar_sliderReleased();

    void on_positionBar_sliderMoved(int position);

    void on_actionAbout_triggered();

    void on_checkBox_stateChanged(int arg1);

    void on_volumeSlider_sliderMoved(int position);

    void on_nextButton_clicked();

    void on_previousButton_clicked();

    void on_actionPreferences_triggered();

    void on_actionAbout_QT_triggered();

    void positionBarValueChanged(int newPos);

    void change_Buttons();

    void update_Slider();

    void update_Volumeknob();

    void update_Time();

    void update_currentPlayList();

    void on_folderList_itemClicked(QTreeWidgetItem *item, int column);

    void on_fileList_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_addNowPlayingButton_clicked();

    void on_actionAdd_Tab_triggered();

    void on_tabWidget_tabCloseRequested(int index);

    void on_muteButton_clicked();

    void deleteAbout();

    void deletePreferences();

private:

    Playlist *currentList;
    Playlist *defaultPlaylist;
    About *about;
    PreferencesDialog *preferences;
    Settings settings;
    Ui::Jabdah *ui;

    QSystemTrayIcon *ticon;

    //WINDOWS RELATED STUFF
    #ifdef _WIN32
    QWinTaskbarButton *taskbarButton;
    QWinTaskbarProgress *taskbarProgress;
    void createTaskbar();
    #endif
    //END OF WINDOWS RELATED STUFF

    QIcon muteIcon;
    QIcon soundIcon;
    QIcon playIcon;
    QIcon pauseIcon;

    bool isInFocus;
    bool MainWindowClosed;
    bool ApplicationClosed;
    bool Expanded;

    float fvolume;
    float *pfvolume;
    float lastvolume;

    int playlistNumber;
};

#endif // JABDAH_H
