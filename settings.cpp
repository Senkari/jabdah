#include "settings.h"

#include <QString>
#include <QFile>
#include <QTextStream>

Settings::Settings()
{
}

void Settings::loadSettings(){
    bool writevalue = false;
    QFile settingsfile("settings.dat");
    QString attrib;
    QString property;
    QString value;

    //Read settings file
    settingsfile.open(QIODevice::ReadOnly);
    QTextStream readsettings(&settingsfile);
    while(true){
        attrib = readsettings.readLine();
        if(attrib == 0) break;
        property = "";
        value = "";
        writevalue = false;
        for(int x = 0; x < attrib.count(); x++){
            if(attrib.at(x) == 0) break;
            else if(attrib.at(x) == QChar(61)) writevalue = true;
            else if(writevalue) value.append(attrib.at(x));
            else property.append(attrib.at(x));
        }
        if(property == QString("theme")) theme = value;
        else if(property == QString("timelineDragPauseMusic")) timelineDragPauseMusic = value.toInt();
        else if(property == QString("volume")) volume = value.toInt();
        else if(property == QString("repeat")) repeat = value.toInt();
        else if(property == QString("audioBalance")) balance = value.toInt();
    }

    return;
}

void Settings::saveSettings(){
    QFile settingsfile("settings.dat");
    if(settingsfile.open(QIODevice::ReadOnly)) settingsfile.remove();
    settingsfile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream writesettings(&settingsfile);

    writesettings << "theme=" << theme << endl
                  << "timelineDragPauseMusic=" << QString::number(timelineDragPauseMusic) << endl
                  << "volume=" << QString::number(volume) << endl
                  << "repeat=" << QString::number(repeat) << endl
                  << "audioBalance=" << QString::number(balance) << endl;
    settingsfile.close();
}

QString Settings::getTheme() const{
    return theme;
}

QStringList Settings::getLocations() const{
    return locations;
}

int Settings::getVolume() const{
    return volume;
}

int Settings::getRepeatState() const{
    return repeat;
}

int Settings::getBalance() const{
    return balance;
}

bool Settings::isMusicPausedWhenTimelineIsDragged() const{
    return timelineDragPauseMusic;
}

bool Settings::isDirBrowserEnabled() const{
    return dirBrowserEnabled;
}

bool Settings::isMuted() const{
    return mute;
}

void Settings::setTheme(const QString appliedTheme){
    theme = appliedTheme;
}

void Settings::setLocationList(const QStringList locationList){
    locations = locationList;
}

void Settings::setOnTimelineDragPauseMusic(const bool state){
    timelineDragPauseMusic = state;
}

void Settings::setVolume(const int value){
    volume = value;
}

void Settings::setRepeatState(const int state){
    repeat = state;
}

void Settings::setBalance(const int state){
    balance = state;
}

void Settings::setDirBrowserEnabled(const bool state){
    dirBrowserEnabled = state;
}

void Settings::setMute(const bool state){
    mute = state;
}

void Settings::addLocation(const QString location){
    locations.append(location);
}

QString Settings::theme;
QStringList Settings::locations;
int Settings::volume;
int Settings::repeat;
int Settings::balance;
bool Settings::timelineDragPauseMusic;
bool Settings::dirBrowserEnabled;
bool Settings::mute;
