#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTimer>
#include <QFileInfoList>
#include <QDir>
#include <QDirIterator>
#include <QImage>
#include <QInputDialog>
#include <QSystemTrayIcon>

#ifdef _WIN32
    #include <QtWinExtras>
#endif

#include "playlist.h"
#include "jabdah.h"
#include "ui_jabdah.h"
#include "settings.h"
#include "definitions.h"
#include "bass.h"
#include "fileref.h"
#include "tag.h"



using namespace TagLib;

Jabdah::Jabdah(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Jabdah),
    isInFocus(true), MainWindowClosed(false), ApplicationClosed(false), Expanded(1), fvolume(0), pfvolume(&fvolume), lastvolume(0), playlistNumber(2),
    muteIcon(QIcon(":/icons/mute.svg")), soundIcon(QIcon(":/icons/sound.svg")), playIcon(QIcon(":/icons/play.svg")), pauseIcon(QIcon(":/icons/pause.svg"))
{
    ui->setupUi(this);

    //Create systemtray icon
    ticon = new QSystemTrayIcon(this);
    ticon->setIcon(QIcon(":/icons/jabdahLogo.svg"));
    ticon->show();

    //This will override some values qt creator sets
    ui->folderList->setHeaderHidden(true);
    ui->fileList->setHeaderHidden(false);

    //Connects positionbars valueChanged signal to a slot that enables mouse clicking in the slider.
    connect(ui->positionBar, SIGNAL(valueChanged(int)),
            this, SLOT(positionBarValueChanged(int)));

    //Timer that surveys when update functions are executed. This is done in order to reduce processor usage.
    QTimer *ptimer = new QTimer(this);
    ptimer->start(5);
    connect(ptimer, SIGNAL(timeout()), this, SLOT(update_Slider()));
    connect(ptimer, SIGNAL(timeout()), this, SLOT(update_Volumeknob()));
    connect(ptimer, SIGNAL(timeout()), this, SLOT(change_Buttons()));
    connect(ptimer, SIGNAL(timeout()), this, SLOT(update_Time()));
    connect(ptimer, SIGNAL(timeout()), this, SLOT(update_currentPlayList()));

    //Create about and preferences dialogs.
    preferences = new PreferencesDialog(this);
    about = new About(this);

    //Creates the default playlist
    defaultPlaylist = new Playlist(this);
    ui->tabWidget->addTab(defaultPlaylist, tr("Default"));
    currentList = defaultPlaylist; //Sets the defaultPlaylist as a current list
    ui->playlistBox->addItem("Default", QVariant::fromValue(defaultPlaylist));

    //Sets song browser's and default playlist's close buttons off so they can't be closed
    ui->tabWidget->tabBar()->tabButton(1, QTabBar::RightSide)->resize(0,0);
    ui->tabWidget->tabBar()->tabButton(0, QTabBar::RightSide)->resize(0,0);

    //Sets up the player according to the loaded settings
    lastvolume = settings.getVolume() / 100.0;

    if(settings.getRepeatState()) ui->checkBox->setChecked(true);

    //Scans given directories if any was given.
    if(settings.isDirBrowserEnabled()){
        QStringList dirlist;
        QTreeWidgetItem* currDir;
        QTreeWidgetItem * subDir;
        dirlist.append(settings.getLocations());
        while(dirlist.count() > 0){
            QDir dir(dirlist.first());
            QTreeWidgetItem * topDir = new QTreeWidgetItem(ui->folderList);
            topDir->setText(0, dir.dirName());
            topDir->setText(1, dir.absolutePath());
            QDirIterator dirIt(dirlist.first(), QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while(dirIt.hasNext()){
                dirIt.next();
                if(dirIt.fileInfo().isDir()){
                    if(dirIt.fileInfo().dir().dirName() == topDir->text(0)) currDir = topDir;
                    else if (dirIt.fileInfo().dir().dirName() == subDir->text(0)) currDir = subDir;
                    else if (dirIt.fileInfo().dir().dirName() == currDir->parent()->text(0)) currDir = currDir->parent();
                    subDir = new QTreeWidgetItem(currDir);
                    subDir->setText(0, dirIt.fileName());
                    subDir->setText(1, dirIt.filePath());
                }
            }
            dirlist.removeFirst();
        }
    }
    else{
        //If there's no directories then the song browser is disabled
        ui->tabWidget->setCurrentIndex(1);
        ui->tabWidget->removeTab(0);
    }

    #ifdef _WIN32
    createTaskbar();
    #endif
}

Jabdah::~Jabdah()
{
    delete ui;
    delete ticon;
    delete about;
    delete preferences;
}

void Jabdah::on_actionFile_triggered()
{
    QStringList filelist = QFileDialog::getOpenFileNames(this, tr("Open File"), QString(), tr("All Supported Files (*.wav *.wave *.aac *.ac3 *.aif *.aiff *.alac *.ape *.flac *.it *.m4a *.mo3 *.mod *.mp3 *.mp2 *.mp1 *.mpc *.mp+ *.mpp *.mid *.mtm *.ogg *.opus *.s3m *.spx *.tta *.umx *.wv *.xm);;"
                                                                                     "Advanced Audio Coding Files (*.aac);;"
                                                                                     "Apple Lossless Files (*.alac);;"
                                                                                     "Audio Interchange File Format Files (*.aif *.aiff);;"
                                                                                     "Dolby AC3 Files (*.ac3);;"
                                                                                     "FastTracker 2 (Extended Module) Files (*.xm);;"
                                                                                     "Free Lossless Audio Compression Files (*.flac);;"
                                                                                     "Impulse Tracker Files (*.it);;"
                                                                                     "MO3 Files (*.mo3);;"
                                                                                     "Monkey's Audio Files (*.ape);;"
                                                                                     "MPEG-1 Audio Layer I Files (*.mp1);;"
                                                                                     "MPEG-1 Audio Layer II Files (*.mp2);;"
                                                                                     "MPEG-1 Audio Layer 3 Files (*.mp3);;"
                                                                                     "MPEG 4 Audio Files (*.m4a);;"
                                                                                     "MOD Files (*.mod);;"
                                                                                     "MultiTracker Files (*.mtm);;"
                                                                                     "Musepack Files (*.mpc *.mp+ *.mpp);;"
                                                                                     "Musical Instrument Digital Interface (MIDI) Files (*.mid);;"
                                                                                     "Ogg Files (*.ogg);;"
                                                                                     "Opus Files (*.opus);;"
                                                                                     "Scream Tracker S3M Files (*.s3m);;"
                                                                                     "Speex Files (*.spx);;"
                                                                                     "true Audio Files (*.tta);;"
                                                                                     "Unreal Tournament Music Files (*.umx);;"
                                                                                     "Waveform Audio File Format Files (*.wav *.wave);;"
                                                                                     "WavPack Files (*.wv);;"
                                                                                     "All Files (*.*)"));
    if(!filelist.isEmpty()){
        int cannotOpen = 0;
        //Set defaultPlaylist as a current playlist and set its row number to 0
        defaultPlaylist->playlistRow = 0;
        //Clear the playlist, add all opened items to it and mark first file as selected
        defaultPlaylist->clear();
        for(int x = 0; x < filelist.count(); x++){
            QString suffix = QFileInfo(filelist.at(x)).suffix();
            if(!(suffix == "mo3" || suffix == "it" || suffix == "xm" || suffix == "s3m" || suffix == "mtm" || suffix == "mod" || suffix == "umx")){
                QByteArray temparr = filelist.at(x).toLocal8Bit();
                char *cfile = temparr.data();
                FileRef song(cfile);
                if(!song.isNull()){
                    String artist = song.tag()->artist();
                    QString Qartist = artist.to8Bit().c_str();

                    String title = song.tag()->title();
                    QString Qtitle = title.to8Bit().c_str();

                    String album = song.tag()->album();
                    QString Qalbum = album.to8Bit().c_str();

                    int year = song.tag()->year();
                    QString Qyear = QString::number(year);

                    int length = song.audioProperties()->length();
                    QString Qlength;
                    int hours = length / 3600;
                    length %= 3600;
                    int minutes = length / 60;
                    length %= 60;
                    int seconds = length;
                    if(hours > 0) Qlength += (QString::number(hours) + ":");
                    Qlength += (QString::number(minutes) + ":");
                    if(seconds < 10) Qlength += ("0" + QString::number(seconds));
                    else Qlength += QString::number(seconds);

                    int track = song.tag()->track();
                    QString Qtrack = QString::number(track);

                    if(Qtrack == "0") Qtrack = "";
                    if(Qyear == "0") Qyear = "";
                    if(Qtitle == "") Qtitle = QFileInfo(filelist.at(x)).fileName();

                    QTreeWidgetItem * song = new QTreeWidgetItem();
                    song->setText(1, Qtrack);
                    song->setText(2, Qtitle);
                    song->setText(3, Qartist);
                    song->setText(4, Qalbum);
                    song->setText(5, Qyear);
                    song->setText(6, Qlength);
                    song->setText(7, filelist.at(x));
                    defaultPlaylist->addItem(song);
                }
                else cannotOpen++;
            }
            else{
                QTreeWidgetItem *song = new QTreeWidgetItem();
                song->setText(1, QFileInfo(filelist.at(x)).fileName());
                song->setText(7, filelist.at(x));
                defaultPlaylist->addItem(song);
            }
        }
        if(cannotOpen > 0) QMessageBox::warning(NULL, "Warning", "Couldn't load " + QString::number(cannotOpen) + " file(s)!");
        if(cannotOpen < filelist.count()){
            defaultPlaylist->setSelected(defaultPlaylist->getItem(defaultPlaylist->playlistRow), true);
            defaultPlaylist->getItem(defaultPlaylist->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));
            load_Song(defaultPlaylist, true);
        }
    }
}

void Jabdah::on_actionQuit_triggered()
{
    ApplicationClosed = true;
    settings.setVolume(fvolume * 100);
    settings.saveSettings();
    this->close();
}

void Jabdah::on_playButton_clicked()
{
    //If play button is clicked then choose the right stream and do the required actions to that stream.
    if(stream::audiofile != 0){
        if(BASS_ChannelIsActive(stream::audiofile) == BASS_ACTIVE_STOPPED){
            BASS_ChannelPlay(stream::audiofile, true);
            this->setWindowTitle(currentList->getItem(currentList->playlistRow)->text(3) + " - " + currentList->getItem(currentList->playlistRow)->text(2) + " Playing - Jabdah");
            this->setWindowIcon(QIcon(":/icons/jabdah_playing"));
            currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));
        }
        else if(BASS_ChannelIsActive(stream::audiofile) == BASS_ACTIVE_PLAYING){
            BASS_ChannelPause(stream::audiofile);
            this->setWindowTitle(currentList->getItem(currentList->playlistRow)->text(3) + " - " + currentList->getItem(currentList->playlistRow)->text(2) + " Paused - Jabdah");
            this->setWindowIcon(QIcon(":/icons/jabdah_paused"));
            currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/pause.svg"));
        }
        else if(BASS_ChannelIsActive(stream::audiofile) == BASS_ACTIVE_PAUSED){
            BASS_ChannelPlay(stream::audiofile, 0);
            this->setWindowTitle(currentList->getItem(currentList->playlistRow)->text(3) + " - " + currentList->getItem(currentList->playlistRow)->text(2) + " Playing - Jabdah");
            this->setWindowIcon(QIcon(":/icons/jabdah_playing"));
            currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));
        }
    }
    else if(stream::modfile != 0){
        if(BASS_ChannelIsActive(stream::modfile) == BASS_ACTIVE_STOPPED){
            BASS_ChannelPlay(stream::modfile, true);
            this->setWindowTitle(currentList->getItem(currentList->playlistRow)->text(2) + " - Playing - Jabdah");
            this->setWindowIcon(QIcon(":/icons/jabdah_playing"));
            currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));
        }
        else if(BASS_ChannelIsActive(stream::modfile) == BASS_ACTIVE_PLAYING){
            BASS_ChannelPause(stream::modfile);
            this->setWindowTitle(currentList->getItem(currentList->playlistRow)->text(2) + " - Paused - Jabdah");
            this->setWindowIcon(QIcon(":/icons/jabdah_paused"));
            currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/pause.svg"));
        }
        else if(BASS_ChannelIsActive(stream::modfile) == BASS_ACTIVE_PAUSED){
            BASS_ChannelPlay(stream::modfile, 0);
            this->setWindowTitle(currentList->getItem(currentList->playlistRow)->text(2) + " - Playing - Jabdah");
            this->setWindowIcon(QIcon(":/icons/jabdah_playing"));
            currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));
        }
    }
    else{
        QMessageBox error;
        error.setWindowTitle("Error playing file.");
        error.setText("Error playing file:");
        error.setInformativeText("There is no file!");
        error.exec();
    }

}

void Jabdah::change_Buttons(){
    if((stream::audiofile != 0 || stream::modfile != 0)){

        if(!ui->playButton->isEnabled())
            ui->playButton->setEnabled(true);
            #ifdef _WIN32
            taskbarProgress->show();
            #endif

        if(!ui->stopButton->isEnabled())
            ui->stopButton->setEnabled(true);

        if(BASS_ChannelIsActive(stream::audiofile) == BASS_ACTIVE_PLAYING || BASS_ChannelIsActive(stream::modfile) == BASS_ACTIVE_PLAYING){
            ui->playButton->setIcon(pauseIcon);
        }
        else ui->playButton->setIcon(playIcon);

    }
    else{
        if(ui->playButton->isEnabled())
            ui->playButton->setDisabled(true);
            #ifdef _WIN32
            taskbarProgress->hide();
            #endif

        if(ui->stopButton->isEnabled())
            ui->stopButton->setDisabled(true);
    }

    if(currentList->getItemCount() > 0){
        ui->nextButton->setEnabled(true);
        ui->previousButton->setEnabled(true);
    }
    else{
        ui->nextButton->setDisabled(true);
        ui->previousButton->setDisabled(true);
    }

    if(settings.isMuted()) ui->muteButton->setIcon(muteIcon);
    else ui->muteButton->setIcon(soundIcon);
}

void Jabdah::update_Slider(){
    if(stream::audiofile != 0){
        QWORD len=BASS_ChannelGetPosition(stream::audiofile, BASS_POS_BYTE);
        double dtime=BASS_ChannelBytes2Seconds(stream::audiofile, len);
        int time = dtime * 10;
        ui->positionBar->setValue(time);
    }
    else if(stream::modfile != 0){
        QWORD len=BASS_ChannelGetPosition(stream::modfile, BASS_POS_BYTE);
        double dtime=BASS_ChannelBytes2Seconds(stream::modfile, len);
        int time = dtime * 10;
        ui->positionBar->setValue(time);
    }
}

void Jabdah::update_Volumeknob(){
    if(!settings.isMuted()){
        if(stream::audiofile != 0) BASS_ChannelGetAttribute(stream::audiofile, BASS_ATTRIB_VOL, pfvolume);
        else if(stream::modfile != 0) BASS_ChannelGetAttribute(stream::modfile, BASS_ATTRIB_VOL, pfvolume);
        else fvolume = lastvolume;
        int volume = fvolume * 100;
        ui->volumeSlider->setValue(volume);
    }
}

bool Jabdah::WindowStatus(){
    if(Jabdah::MainWindowClosed || Jabdah::ApplicationClosed) return true;
    else return false;
}

void Jabdah::closeEvent(QCloseEvent *event){

    settings.setVolume(fvolume * 100);
    settings.saveSettings();

    event->accept();
}

void Jabdah::on_stopButton_clicked()
{
    if(stream::audiofile != 0){
        BASS_ChannelStop(stream::audiofile);
        BASS_ChannelSetPosition(stream::audiofile, 0, BASS_POS_BYTE);
        this->setWindowTitle(currentList->getItem(currentList->playlistRow)->text(3) + " - " + currentList->getItem(currentList->playlistRow)->text(2) + " Stopped - Jabdah");
        //Deselect everything
        for(int x = 0; x < currentList->getItemCount(); x++){
            currentList->getItem(x)->setIcon(0, QIcon());
        }
        this->setWindowIcon(QIcon(":/icons/jabdah_stopped"));
    }
    else if(stream::modfile != 0){
        BASS_ChannelStop(stream::modfile);
        BASS_ChannelSetPosition(stream::modfile, 0, BASS_POS_BYTE);
        this->setWindowTitle(currentList->getItem(currentList->playlistRow)->text(3) + " - " + currentList->getItem(currentList->playlistRow)->text(2) + " Stopped - Jabdah");
        //Deselect everything
        for(int x = 0; x < currentList->getItemCount(); x++){
            currentList->getItem(x)->setIcon(0, QIcon());
        }
        this->setWindowIcon(QIcon(":/icons/jabdah_stopped"));
    }
    this->change_Buttons();
}

void Jabdah::update_Time(){
    if(stream::audiofile != 0){
        //Some QString declarations
        QString shours;
        QString sminutes;
        QString sseconds;
        QString stseconds;
        QString stminutes;
        QString sthours;

        //Let's fetch our data, first the total time and then the current time of sound we're playing
        QWORD tlen=BASS_ChannelGetLength(stream::audiofile, BASS_POS_BYTE);
        QWORD len=BASS_ChannelGetPosition(stream::audiofile, BASS_POS_BYTE);

        /*---------TOTAL TIME TRANSFORMATION-------------*/
        double dttime=BASS_ChannelBytes2Seconds(stream::audiofile, tlen);
        int ttime = dttime;
        int thours = ttime / 3600;
        ttime %= 3600;
        int tminutes = ttime / 60;
        ttime %= 60;
        int tseconds = ttime;

        if(thours < 10) sthours = "0" + QString::number(thours);
        else sthours = QString::number(thours);
        if(tminutes < 10) stminutes = "0" + QString::number(tminutes);
        else stminutes = QString::number(tminutes);
        if(tseconds < 10) stseconds = "0" + QString::number(tseconds);
        else stseconds = QString::number(tseconds);

        /*-----------CURRENT TIME TRANSFORMATION------------*/
        double dtime=BASS_ChannelBytes2Seconds(stream::audiofile, len);
        int time = dtime;
        int hours = time / 3600;
        time %= 3600;
        int minutes = time / 60;
        time %= 60;
        int seconds = time;

        if(hours < 10) shours = "0" + QString::number(hours);
        else shours = QString::number(hours);
        if(minutes < 10) sminutes = "0" + QString::number(minutes);
        else sminutes = QString::number(minutes);
        if(seconds < 10) sseconds = "0" + QString::number(seconds);
        else sseconds = QString::number(seconds);

        //Put all the times to one huge QString and then put that QString into songTimeLabel
        QString stime = shours + ":" + sminutes + ":" + sseconds + "/" + sthours + ":" + stminutes + ":" + stseconds;
        ui->songTimeLabel->setText(stime);
    }
    else if(stream::modfile != 0){
        //Some QString declarations
        QString shours;
        QString sminutes;
        QString sseconds;
        QString stseconds;
        QString stminutes;
        QString sthours;

        //Let's fetch our data, first the total time and then the current time of sound we're playing
        QWORD tlen=BASS_ChannelGetLength(stream::modfile, BASS_POS_BYTE);
        QWORD len=BASS_ChannelGetPosition(stream::modfile, BASS_POS_BYTE);

        /*---------TOTAL TIME TRANSFORMATION-------------*/
        double dttime=BASS_ChannelBytes2Seconds(stream::modfile, tlen);
        int ttime = dttime;
        int thours = ttime / 3600;
        ttime %= 3600;
        int tminutes = ttime / 60;
        ttime %= 60;
        int tseconds = ttime;

        if(thours < 10) sthours = "0" + QString::number(thours);
        else sthours = QString::number(thours);
        if(tminutes < 10) stminutes = "0" + QString::number(tminutes);
        else stminutes = QString::number(tminutes);
        if(tseconds < 10) stseconds = "0" + QString::number(tseconds);
        else stseconds = QString::number(tseconds);

        /*-----------CURRENT TIME TRANSFORMATION------------*/
        double dtime=BASS_ChannelBytes2Seconds(stream::modfile, len);
        int time = dtime;
        int hours = time / 3600;
        time %= 3600;
        int minutes = time / 60;
        time %= 60;
        int seconds = time;

        if(hours < 10) shours = "0" + QString::number(hours);
        else shours = QString::number(hours);
        if(minutes < 10) sminutes = "0" + QString::number(minutes);
        else sminutes = QString::number(minutes);
        if(seconds < 10) sseconds = "0" + QString::number(seconds);
        else sseconds = QString::number(seconds);

        //Put all the times to one huge QString and then put that QString into songTimeLabel
        QString stime = shours + ":" + sminutes + ":" + sseconds + "/" + sthours + ":" + stminutes + ":" + stseconds;
        ui->songTimeLabel->setText(stime);
    }
}

void Jabdah::on_positionBar_sliderPressed()
{
    if(stream::audiofile != 0 && settings.isMusicPausedWhenTimelineIsDragged()) BASS_ChannelPause(stream::audiofile);
    else if(stream::modfile != 0 && settings.isMusicPausedWhenTimelineIsDragged()) BASS_ChannelPause(stream::modfile);
}

void Jabdah::on_positionBar_sliderReleased()
{
    if(stream::audiofile != 0 && settings.isMusicPausedWhenTimelineIsDragged()) BASS_ChannelPlay(stream::audiofile, 0);
    else if(stream::modfile != 0 && settings.isMusicPausedWhenTimelineIsDragged()) BASS_ChannelPlay(stream::modfile, 0);
}

void Jabdah::on_positionBar_sliderMoved(int position)
{
    if(stream::audiofile != 0){
        double pos = position / 10;
        QWORD time = BASS_ChannelSeconds2Bytes(stream::audiofile, pos);
        BASS_ChannelSetPosition(stream::audiofile, time, BASS_POS_BYTE);
    }
    else if(stream::modfile != 0){
        double pos = position / 10;
        QWORD time = BASS_ChannelSeconds2Bytes(stream::modfile, pos);
        BASS_ChannelSetPosition(stream::modfile, time, BASS_POS_BYTE);
    }
}

void Jabdah::on_actionAbout_triggered()
{
    about->show();
    about->raise();
    about->activateWindow();
}

//Changes BASS channel's repeatability
void Jabdah::on_checkBox_stateChanged(int arg1)
{
    if(stream::audiofile != 0){
        if(arg1){
            BASS_ChannelFlags(stream::audiofile, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
        }
        else if(!arg1){
            BASS_ChannelFlags(stream::audiofile, 0, BASS_SAMPLE_LOOP);
        }
    }
    else if(stream::modfile != 0){
        if(arg1){
            BASS_ChannelFlags(stream::modfile, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
        }
        else if(!arg1){
            BASS_ChannelFlags(stream::modfile, 0, BASS_SAMPLE_LOOP);
        }
    }
    settings.setRepeatState(arg1);

}

//Sets volume when volumeSlider is moved
void Jabdah::on_volumeSlider_sliderMoved(int position)
{
    float pos = position;
    pos /= 100;
    if(!settings.isMuted()){
        if(stream::audiofile != 0) BASS_ChannelSetAttribute(stream::audiofile, BASS_ATTRIB_VOL, pos);
        else if(stream::modfile != 0) BASS_ChannelSetAttribute(stream::modfile, BASS_ATTRIB_VOL, pos);
    }
    lastvolume = pos;
}

//Handles playlist where songs are playing
void Jabdah::update_currentPlayList(){

    if(stream::audiofile != 0 && BASS_ChannelIsActive(stream::audiofile) == BASS_ACTIVE_STOPPED && BASS_ChannelGetPosition(stream::audiofile, BASS_POS_BYTE) == BASS_ChannelGetLength(stream::audiofile, BASS_POS_BYTE)){

        //If list row number is higher than the actual number of items, then do not play unless user wants the playlist to be repeated
        if(++currentList->playlistRow > currentList->getItemCount()-1){
            if(!(currentList->isRepeated())){
                currentList->playlistRow--;
                return;
            }
            currentList->playlistRow = 0;
        }


        //Deselect everything
        for(int x = 0; x < currentList->getItemCount(); x++){
            currentList->setSelected(currentList->getItem(x), false);
            currentList->getItem(x)->setIcon(0, QIcon());
        }

        //Move the select box to highlight the appropriate list item
        currentList->setSelected(currentList->getItem(currentList->playlistRow), true);
        currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));

        //Load the song
        load_Song(currentList, true);
    }
    else if(stream::modfile != 0 && BASS_ChannelIsActive(stream::modfile) == BASS_ACTIVE_STOPPED && BASS_ChannelGetPosition(stream::modfile, BASS_POS_BYTE) == BASS_ChannelGetLength(stream::modfile, BASS_POS_BYTE)){
        //Increase the list row number
        currentList->playlistRow++;

        //If list row number is higher than the actual number of items, then do not play unless user wants the playlist to be repeated
        if(currentList->playlistRow++ > currentList->getItemCount()-1){
            if(!(currentList->isRepeated())){
                currentList->playlistRow--;
                return;
            }
            else currentList->playlistRow = 0;

        }

        //Deselect everything
        for(int x = 0; x < currentList->getItemCount(); x++){
            currentList->setSelected(currentList->getItem(x), false);
            currentList->getItem(x)->setIcon(0, QIcon());
        }

        //Move the select box to highlight the appropriate list item
        currentList->setSelected(currentList->getItem(currentList->playlistRow), true);
        currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));

        //Load the song
        load_Song(currentList, true);
    }

}

void Jabdah::on_nextButton_clicked()
{
    //Increase the list row number
    currentList->playlistRow++;

    //If list row number is higher than the actual number of items, then go to start
    if(currentList->playlistRow > currentList->getItemCount()-1) currentList->playlistRow = 0;

    //Deselect everything
    for(int x = 0; x < currentList->getItemCount(); x++){
        currentList->setSelected(currentList->getItem(x), false);
        currentList->getItem(x)->setIcon(0, QIcon());
    }

    //Move the select box to highlight the appropriate list item
    currentList->setSelected(currentList->getItem(currentList->playlistRow), true);
    currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));

    //Load the song
    load_Song(currentList, true);
}

void Jabdah::load_Song(Playlist *currentPlaylist, bool Autoplay){
    //Store the currently played playlist so we can manipulate it
    currentList = currentPlaylist;
    int playlistRow = currentPlaylist->playlistRow;

    //Load the requested item to BASS
    QString file = currentPlaylist->getText(playlistRow, 7);
    if(stream::audiofile != 0){
        BASS_StreamFree(stream::audiofile);
        stream::audiofile = 0;
    }
    else if(stream::modfile != 0){
            BASS_MusicFree(stream::modfile);
            stream::modfile = 0;
    }

    //Convert filename to c char array so we can use it with BASS
    QByteArray temparr = file.toLocal8Bit();
    char *cfile = temparr.data();

    //Check whether we should use audio stream or mod stream
    QString suffix = QFileInfo(file).suffix();
    if(suffix == "mo3" || suffix == "it" || suffix == "xm" || suffix == "s3m" || suffix == "mtm" || suffix == "mod" || suffix == "umx") stream::modfile = BASS_MusicLoad(0, cfile, 0, 0, BASS_MUSIC_PRESCAN, 0);
    else stream::audiofile = BASS_StreamCreateFile(false, cfile, 0, 0, 0);

    if(stream::audiofile != 0){
        //If loop is set, then loop the song, otherwise don't
        if(settings.getRepeatState()) BASS_ChannelFlags(stream::audiofile, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
        else BASS_ChannelFlags(stream::audiofile, 0, BASS_SAMPLE_LOOP);

        //Read the total length of a song and store it into the slider
        QWORD len=BASS_ChannelGetLength(stream::audiofile, BASS_POS_BYTE);
        double dtime=BASS_ChannelBytes2Seconds(stream::audiofile, len);
        int time = dtime * 10;
        ui->positionBar->setMaximum(time);

        //Get the info from the channel
        BASS_CHANNELINFO info;
        BASS_ChannelGetInfo(stream::audiofile, &info);

        //Read ID3 tags
        FileRef song(cfile);
        int bitrate = song.file()->audioProperties()->bitrate();
        int samplerate = song.file()->audioProperties()->sampleRate();
        ui->fileInfoLabel->setText(QString::number(bitrate) + " kbps | " + QString::number(samplerate) + " Hz");
        ui->songInfoLabel->setText(currentPlaylist->getText(playlistRow, 3) + " - " + currentPlaylist->getText(playlistRow, 2));

        //Change Window title to playing
        this->setWindowTitle(currentPlaylist->getText(playlistRow, 3) + " - " + currentPlaylist->getText(playlistRow, 2) + " - Playing - Jabdah");
        this->setWindowIcon(QIcon(":/icons/jabdah_playing"));

        //Set volume and panning and looping
        BASS_ChannelSetAttribute(stream::audiofile, BASS_ATTRIB_VOL, fvolume);
        BASS_ChannelSetAttribute(stream::audiofile, BASS_ATTRIB_PAN, settings.getBalance());
        if(settings.getRepeatState()) BASS_ChannelFlags(stream::audiofile, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
        //If Autoplay -flag is set, then start playing the song
        if(Autoplay) BASS_ChannelPlay(stream::audiofile, true);

        #ifdef _WIN32
        taskbarProgress->setMaximum(time);
        #endif
    }
    else if(stream::modfile != 0){
        //If loop is set, then loop the song, otherwise don't
        if(settings.getRepeatState()) BASS_ChannelFlags(stream::modfile, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
        else BASS_ChannelFlags(stream::modfile, 0, BASS_SAMPLE_LOOP);

        //Read the total length of a song and store it into the slide
        QWORD len =BASS_ChannelGetLength(stream::modfile, BASS_POS_BYTE);
        double dtime=BASS_ChannelBytes2Seconds(stream::modfile, len);
        int time = dtime * 10;
        ui->positionBar->setMaximum(time);

        //Get the info from the channel
        BASS_CHANNELINFO info;
        BASS_ChannelGetInfo(stream::modfile, &info);

        QString songname(info.filename);
        ui->songInfoLabel->setText(songname);

        //Change Window title to playing
        this->setWindowTitle(songname + " - Playing - Jabdah");
        this->setWindowIcon(QIcon(":/icons/jabdah_playing"));

        //Set volume and panning and looping
        BASS_ChannelSetAttribute(stream::modfile, BASS_ATTRIB_VOL, fvolume);
        BASS_ChannelSetAttribute(stream::modfile, BASS_ATTRIB_PAN, settings.getBalance());
        if(settings.getRepeatState()) BASS_ChannelFlags(stream::modfile, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
        //If Autoplay -flag is set, then start playing the song
        if(Autoplay) BASS_ChannelPlay(stream::modfile, true);

        #ifdef _WIN32
        taskbarProgress->setMaximum(time);
        #endif
    }
    else{
        int errorCode = BASS_ErrorGetCode();
        QMessageBox::warning(this, "Error opening file!", "Error code: " + QString::number(errorCode));
        ui->songInfoLabel->setText("");
    }
}

void Jabdah::on_previousButton_clicked()
{
    //Decrease the list row number
    currentList->playlistRow--;

    //If list row number is less than zero, then go to the end of the list
    if(currentList->playlistRow < 0) currentList->playlistRow = (currentList->getItemCount()) - 1;

    //Deselect everything
    for(int x = 0; x < currentList->getItemCount(); x++){
        currentList->setSelected(currentList->getItem(x), false);
        currentList->getItem(x)->setIcon(0, QIcon());
    }

    //Move the select box to highlight the appropriate list item
    currentList->setSelected(currentList->getItem(currentList->playlistRow), true);
    currentList->getItem(currentList->playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));

    //Load the song
    load_Song(currentList, true);
}

void Jabdah::on_actionPreferences_triggered()
{
    preferences->scanThemes();
    preferences->loadSettings();
    preferences->show();
    preferences->raise();
    preferences->activateWindow();
}

void Jabdah::set_Player(){
    //Sets up the player according to the loaded settings
    lastvolume = settings.getVolume() / 100.0;

    if(settings.getRepeatState()) ui->checkBox->setChecked(true);

}

void Jabdah::on_actionAbout_QT_triggered()
{
    qApp->aboutQt();
}


void Jabdah::positionBarValueChanged(int newPos){

    if (this->isActiveWindow()){
        // Make slider to follow the mouse directly and not by pageStep steps
        Qt::MouseButtons btns = QApplication::mouseButtons();
        QPoint localMousePos = ui->positionBar->mapFromGlobal(QCursor::pos());
        bool clickOnSlider = (btns & Qt::LeftButton) &&
                            (localMousePos.x() >= 0 && localMousePos.y() >= 0 &&
                            localMousePos.x() < ui->positionBar->size().width() &&
                            localMousePos.y() < ui->positionBar->size().height());
        if (clickOnSlider)
        {
            // Attention! The following works only for Horizontal, Left-to-right sliders
            float posRatio = localMousePos.x() / (float )ui->positionBar->size().width();
            int sliderRange = ui->positionBar->maximum() - ui->positionBar->minimum();
            int sliderPosUnderMouse = ui->positionBar->minimum() + sliderRange * posRatio;
            if (sliderPosUnderMouse != newPos)
            {
                if(stream::audiofile != 0 && settings.isMusicPausedWhenTimelineIsDragged()) BASS_ChannelPause(stream::audiofile);
                else if(stream::modfile != 0 && settings.isMusicPausedWhenTimelineIsDragged()) BASS_ChannelPause(stream::modfile);

                ui->positionBar->setValue(sliderPosUnderMouse);

                if(stream::audiofile != 0){
                    double pos = sliderPosUnderMouse / 10;
                    QWORD time = BASS_ChannelSeconds2Bytes(stream::audiofile, pos);
                    BASS_ChannelSetPosition(stream::audiofile, time, BASS_POS_BYTE);
                }
                else if(stream::modfile != 0){
                    double pos = sliderPosUnderMouse / 10;
                    QWORD time = BASS_ChannelSeconds2Bytes(stream::modfile, pos);
                    BASS_ChannelSetPosition(stream::modfile, time, BASS_POS_BYTE);
                }

                if(stream::audiofile != 0 && settings.isMusicPausedWhenTimelineIsDragged()) BASS_ChannelPlay(stream::audiofile, false);
                if(stream::modfile != 0 && settings.isMusicPausedWhenTimelineIsDragged()) BASS_ChannelPlay(stream::modfile, false);

                return;
            }
        }
    }

    #ifdef _WIN32
    taskbarProgress->setValue(newPos);
    #endif
}

void Jabdah::on_folderList_itemClicked(QTreeWidgetItem *item, int column)
{
    ui->fileList->clear();
    QDirIterator dirIt(item->text(1));
    while(dirIt.hasNext()){
        dirIt.next();
        QByteArray temparr = dirIt.filePath().toLocal8Bit();
        char *cfile = temparr.data();
        FileRef song(cfile);
        if(!song.isNull()){
            String artist = song.tag()->artist();
            QString Qartist = artist.to8Bit().c_str();

            String title = song.tag()->title();
            QString Qtitle = title.to8Bit().c_str();

            String album = song.tag()->album();
            QString Qalbum = album.to8Bit().c_str();

            int year = song.tag()->year();
            QString Qyear = QString::number(year);

            int length = song.audioProperties()->length();
            QString Qlength;
            int hours = length / 3600;
            length %= 3600;
            int minutes = length / 60;
            length %= 60;
            int seconds = length;
            if(hours > 0) Qlength += (QString::number(hours) + ":");
            Qlength += (QString::number(minutes) + ":");
            if(seconds < 10) Qlength += ("0" + QString::number(seconds));
            else Qlength += QString::number(seconds);

            int track = song.tag()->track();
            QString Qtrack = QString::number(track);

            if(Qtrack == "0") Qtrack = "";
            if(Qyear == "0") Qyear = "";
            if(Qtitle == "") Qtitle = dirIt.fileName();

            QTreeWidgetItem * song = new QTreeWidgetItem();
            song->setText(0, Qtrack);
            song->setText(1, Qtitle);
            song->setText(2, Qartist);
            song->setText(3, Qalbum);
            song->setText(4, Qyear);
            song->setText(5, Qlength);
            song->setText(6, dirIt.filePath());
            ui->fileList->addTopLevelItem(song);
        }
    }
}

void Jabdah::on_fileList_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QTreeWidgetItem *song = new QTreeWidgetItem();
    song->setText(1, item->text(0));
    song->setText(2, item->text(1));
    song->setText(3, item->text(2));
    song->setText(4, item->text(3));
    song->setText(5, item->text(4));
    song->setText(6, item->text(5));
    song->setText(7, item->text(6));
    currentList->addItem(song);
    currentList->playlistRow = currentList->getItemCount()-1;
    currentList->setSelected(currentList->getItem(currentList->playlistRow), true);
    load_Song(currentList, true);
}

void Jabdah::on_addNowPlayingButton_clicked()
{
    bool emptylist = false;
    Playlist *selectedList;
    if(ui->playlistBox->currentIndex() == 0){
        bool ok = false;
        QString playlistName = QInputDialog::getText(this, tr("New playlist"), tr("Playlist name:"), QLineEdit::Normal, "Playlist " + QString::number(playlistNumber), &ok);
        if(ok && !playlistName.isEmpty()){
            selectedList = new Playlist(this);
            ui->tabWidget->addTab(selectedList, playlistName);
            ui->playlistBox->addItem(playlistName, QVariant::fromValue(selectedList));
            playlistNumber++;
        }
        else if(ok && playlistName.isEmpty()){
            QMessageBox::critical(this, tr("Naming error"), tr("At least one character is required. Playlist not created."));
            return;
        }
        else return;
    }
    else{
        selectedList = ui->playlistBox->currentData().value<Playlist*>();
    }

    QTreeWidgetItem *song;

    if(selectedList->getItemCount() == 0) emptylist = true;
    //Adds selected files to the selected playlist.
    for(int x = 0; x < ui->fileList->topLevelItemCount(); x++){
        if(ui->fileList->topLevelItem(x)->isSelected()){
            song = new QTreeWidgetItem();
            song->setText(1, ui->fileList->topLevelItem(x)->text(0));
            song->setText(2, ui->fileList->topLevelItem(x)->text(1));
            song->setText(3, ui->fileList->topLevelItem(x)->text(2));
            song->setText(4, ui->fileList->topLevelItem(x)->text(3));
            song->setText(5, ui->fileList->topLevelItem(x)->text(4));
            song->setText(6, ui->fileList->topLevelItem(x)->text(5));
            song->setText(7, ui->fileList->topLevelItem(x)->text(6));
            selectedList->addItem(song);
        }
    }
    if(emptylist && stream::audiofile == 0 && stream::modfile == 0){
        selectedList->playlistRow = 0;
        selectedList->setSelected(selectedList->getItem(selectedList->playlistRow), true);
        load_Song(selectedList, false);
    }
}

void Jabdah::changeSongInfoText(QString text){
    ui->songInfoLabel->setText(text);
}

void Jabdah::changeSongTimeText(QString text){
    ui->songTimeLabel->setText(text);
}

void Jabdah::on_actionAdd_Tab_triggered()
{
    bool ok = false;
    QString playlistName = QInputDialog::getText(this, tr("New playlist"), tr("Playlist name:"), QLineEdit::Normal, "Playlist " + QString::number(playlistNumber), &ok);
    if(ok && !playlistName.isEmpty()){
        Playlist *nPlaylist = new Playlist(this);
        ui->tabWidget->addTab(nPlaylist, playlistName);
        ui->playlistBox->addItem(playlistName, QVariant::fromValue(nPlaylist));
        playlistNumber++;
    }
    else if(ok && playlistName.isEmpty()){
        QMessageBox::critical(this, tr("Naming error"), tr("At least one character is required. Playlist not created."));
        return;
    }
    else return;

}

void Jabdah::on_tabWidget_tabCloseRequested(int index)
{
    QWidget *widgetToRemove = ui->tabWidget->widget(index);
    Playlist *listToRemove = (Playlist*)widgetToRemove;
    if(listToRemove == currentList){
        if(defaultPlaylist->getItemCount() > 0){
            load_Song(defaultPlaylist, false);
        }
        else{
            if(stream::audiofile != 0){
                BASS_ChannelStop(stream::audiofile);
                BASS_StreamFree(stream::audiofile);
                stream::audiofile = 0;
            }
            else if(stream::modfile != 0){
                BASS_ChannelStop(stream::modfile);
                BASS_MusicFree(stream::modfile);
                stream::modfile = 0;
            }
            ui->songTimeLabel->setText("00:00:00/00:00:00");
            ui->songInfoLabel->setText("");
        }
        currentList = defaultPlaylist;
    }
    ui->playlistBox->removeItem(ui->playlistBox->findData(QVariant::fromValue(ui->tabWidget->widget(index))));
    delete ui->tabWidget->widget(index);


}

void Jabdah::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Space){
        if(stream::audiofile != 0 || stream::modfile != 0){
            on_playButton_clicked();
        }
    }
    else if(event->key() == Qt::Key_Right){

        if(stream::audiofile != 0){
            QWORD oldBytePos = BASS_ChannelGetPosition(stream::audiofile, BASS_POS_BYTE);
            double oldPos = BASS_ChannelBytes2Seconds(stream::audiofile, oldBytePos);
            double newPos = oldPos + 5.0;
            QWORD time = BASS_ChannelSeconds2Bytes(stream::audiofile, newPos);
            BASS_ChannelSetPosition(stream::audiofile, time, BASS_POS_BYTE);
        }
        else if(stream::modfile != 0){
            QWORD oldBytePos = BASS_ChannelGetPosition(stream::modfile, BASS_POS_BYTE);
            double oldPos = BASS_ChannelBytes2Seconds(stream::modfile, oldBytePos);
            double newPos = oldPos + 5.0;
            QWORD time = BASS_ChannelSeconds2Bytes(stream::modfile, newPos);
            BASS_ChannelSetPosition(stream::modfile, time, BASS_POS_BYTE);
        }

    }
    else if(event->key() == Qt::Key_Left){

        if(stream::audiofile != 0){
            QWORD oldBytePos = BASS_ChannelGetPosition(stream::audiofile, BASS_POS_BYTE);
            double oldPos = BASS_ChannelBytes2Seconds(stream::audiofile, oldBytePos);
            double newPos = oldPos - 5.0;
            QWORD time = BASS_ChannelSeconds2Bytes(stream::audiofile, newPos);
            BASS_ChannelSetPosition(stream::audiofile, time, BASS_POS_BYTE);
        }
        else if(stream::modfile != 0){
            QWORD oldBytePos = BASS_ChannelGetPosition(stream::modfile, BASS_POS_BYTE);
            double oldPos = BASS_ChannelBytes2Seconds(stream::modfile, oldBytePos);
            double newPos = oldPos - 5.0;
            QWORD time = BASS_ChannelSeconds2Bytes(stream::modfile, newPos);
            BASS_ChannelSetPosition(stream::modfile, time, BASS_POS_BYTE);
        }

    }
}

void Jabdah::on_muteButton_clicked()
{
    if(settings.isMuted()){
        if(stream::audiofile != 0) BASS_ChannelSetAttribute(stream::audiofile, BASS_ATTRIB_VOL, lastvolume);
        else if(stream::modfile != 0) BASS_ChannelSetAttribute(stream::modfile, BASS_ATTRIB_VOL, lastvolume);
        settings.setMute(false);
    }
    else{
        if(stream::audiofile != 0) BASS_ChannelSetAttribute(stream::audiofile, BASS_ATTRIB_VOL, 0);
        else if(stream::modfile != 0) BASS_ChannelSetAttribute(stream::modfile, BASS_ATTRIB_VOL, 0);
        settings.setMute(true);
    }
}

void Jabdah::deleteAbout(){
    delete about;
}

void Jabdah::deletePreferences(){
    delete preferences;
}

#ifdef _WIN32
void Jabdah::createTaskbar(){
    QMessageBox(this).show();
    taskbarButton = new QWinTaskbarButton(this);
    taskbarButton->setWindow(windowHandle());
    taskbarProgress = taskbarButton->progress();
}
#endif
