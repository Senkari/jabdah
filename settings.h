#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QStringList>

class Settings
{
private:
    static QString theme;
    static QStringList locations;
    static int volume;
    static int repeat;
    static int balance;
    static bool timelineDragPauseMusic;
    static bool dirBrowserEnabled;
    static bool mute;

public:
    //getters
    QString getTheme() const;
    QStringList getLocations() const;
    int getVolume() const;
    int getRepeatState() const;
    int getBalance() const;
    bool isMusicPausedWhenTimelineIsDragged() const;
    bool isDirBrowserEnabled() const;
    bool isMuted() const;

    //setters
    void setTheme(const QString appliedTheme);
    void setLocationList(const QStringList locationList);
    void setOnTimelineDragPauseMusic(const bool state);
    void setVolume(const int value);
    void setRepeatState(const int state);
    void setBalance(const int state);
    void setDirBrowserEnabled(const bool state);
    void setMute(const bool state);

    void addLocation(const QString location);

    void loadSettings();
    void saveSettings();
    Settings();
};

#endif // SETTINGS_H
