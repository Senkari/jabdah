#include <QTreeWidgetItem>
#include <QFileDialog>
#include <QMessageBox>

#include "jabdah.h"
#include "playlist.h"
#include "ui_playlist.h"
#include "bass.h"
#include "definitions.h"
#include "fileref.h"
#include "tag.h"

using namespace TagLib;

Playlist::Playlist(Jabdah *parent) :
    QWidget(parent),
    ui(new Ui::Playlist),
    playlistRow(0)
{
    ui->setupUi(this);
    ui->playlist->setColumnWidth(0, 25);
    ui->playlist->setColumnWidth(1, 40);
    ui->playlist->setColumnHidden(0,true);
    jabdah = parent;
}

Playlist::~Playlist()
{
    delete ui;
}

void Playlist::on_addButton_clicked()
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
            bool emptylist = false;
            if(ui->playlist->topLevelItemCount() == 0) emptylist = true;
            int cannotOpen = 0;

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
                        ui->playlist->addTopLevelItem(song);
                    }
                    else cannotOpen++;
                }
                else{
                    QTreeWidgetItem * song = new QTreeWidgetItem();
                    song->setText(1, QFileInfo(filelist.at(x)).fileName());
                    song->setText(7, filelist.at(x));
                    ui->playlist->addTopLevelItem(song);
                }
            }
            if(cannotOpen > 0) QMessageBox::warning(NULL, "Warning", "Couldn't load " + QString::number(cannotOpen) + " file(s)!");
            if(emptylist && cannotOpen < filelist.count() && stream::audiofile == 0 && stream::modfile == 0){
                playlistRow = 0;
                ui->playlist->setItemSelected(ui->playlist->topLevelItem(playlistRow), true);
                jabdah->load_Song(this, false);
            }
            else if(emptylist && cannotOpen < filelist.count()){
                playlistRow = 0;
                ui->playlist->setItemSelected(ui->playlist->topLevelItem(playlistRow), true);
            }
        }
}

void Playlist::on_removeButton_clicked()
{
    bool playing = false;
    if(BASS_ChannelIsActive(stream::audiofile) == BASS_ACTIVE_PLAYING || BASS_ChannelIsActive(stream::modfile) == BASS_ACTIVE_PLAYING) playing = true;
    //Check if one of the selected items is playing and if it is, then remove the song
    bool currentSelected = ui->playlist->isItemSelected(ui->playlist->topLevelItem(playlistRow));
    if(currentSelected){
        if(stream::audiofile != 0){
            BASS_StreamFree(stream::audiofile);
            stream::audiofile = 0;
        }
        else{
            BASS_StreamFree(stream::modfile);
            stream::modfile = 0;
        }
        jabdah->changeSongInfoText("");
    }

    //Remove selected item
    for(int x = 0; x < ui->playlist->topLevelItemCount(); x++){
        if(ui->playlist->isItemSelected(ui->playlist->topLevelItem(x))){
            QTreeWidgetItem *item = ui->playlist->takeTopLevelItem(x);
            delete item;
            break;
        }
    }

    //If there are items left on the playlist but we are not playing
    if(((ui->playlist->topLevelItemCount() > 0 && stream::audiofile == 0) || (ui->playlist->topLevelItemCount() > 0 && stream::modfile == 0)) && currentSelected){
        if(playing) jabdah->load_Song(this, true);
        else jabdah->load_Song(this, false);
    }
    else if(ui->playlist->topLevelItemCount() == 0) jabdah->changeSongTimeText("00:00:00/00:00:00");
}

void Playlist::on_moveDownButton_clicked()
{
    //Checks every item and moves every selected one down by one.
    for(int x = 0; x < ui->playlist->topLevelItemCount(); x++){
        if(ui->playlist->isItemSelected(ui->playlist->topLevelItem(x))){
            QTreeWidgetItem *item = ui->playlist->takeTopLevelItem(x);
            ui->playlist->insertTopLevelItem(x+1, item);
            for(int y = 0; y < ui->playlist->topLevelItemCount(); y++){
                ui->playlist->setItemSelected(item, 0);
            }
            ui->playlist->setItemSelected(item, true);
            break;
        }
    }
}

void Playlist::on_moveUpButton_clicked()
{
    //Checks every item and moves every selected one up by one.
    for(int x = 0; x < ui->playlist->topLevelItemCount(); x++){
        if(ui->playlist->isItemSelected(ui->playlist->topLevelItem(x))){
            QTreeWidgetItem *item = ui->playlist->takeTopLevelItem(x);
            ui->playlist->insertTopLevelItem(x-1, item);
            for(int y = 0; y < ui->playlist->topLevelItemCount(); y++){
                ui->playlist->setItemSelected(item, 0);
            }
            ui->playlist->setItemSelected(item, true);
            break;
        }
    }
}

void Playlist::on_playlist_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    playlistRow = ui->playlist->indexOfTopLevelItem(item);

    //Deselect everything
    for(int x = 0; x < ui->playlist->topLevelItemCount(); x++){
        ui->playlist->topLevelItem(x)->setSelected(false);
        ui->playlist->topLevelItem(x)->setIcon(0, QIcon());
    }

    //Move the select box to highlight the appropriate list item
    ui->playlist->topLevelItem(playlistRow)->setIcon(0, QIcon(":/icons/play.svg"));
    ui->playlist->topLevelItem(playlistRow)->setSelected(true);

    jabdah->load_Song(this, true);
}

QString Playlist::getText(int row, int column){
    return ui->playlist->topLevelItem(row)->text(column);
}

int Playlist::getItemCount(){
    return ui->playlist->topLevelItemCount();
}

void Playlist::clear(){
    ui->playlist->clear();
}

void Playlist::addItem(QTreeWidgetItem *item){
    ui->playlist->addTopLevelItem(item);
}

void Playlist::setSelected(QTreeWidgetItem *item, bool value){
    ui->playlist->setItemSelected(item, value);
}

bool Playlist::isRepeated(){
    return ui->repeatPlaylist->isChecked();
}

QTreeWidgetItem * Playlist::getItem(int row){
    return ui->playlist->topLevelItem(row);
}
