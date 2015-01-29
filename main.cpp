#include "jabdah.h"
#include "ui_jabdah.h"
#include "addfolders.h"
#include "bass.h"
#include "settings.h"
#include <qmath.h>
#include <QApplication>
#include <QMessageBox>
#include <QTextStream>

int main(int argc, char *argv[])
{
    void BASS_InitPlugins();
    Settings settings;
    QApplication a(argc, argv);

    QString location;
    QFile locationsfile("locations.dat");
    if(!locationsfile.open(QIODevice::ReadOnly)){
        Addfolders w;
        w.exec();
    }
    else{
        while(true){
            location = locationsfile.readLine();
            if(location.isEmpty()) break;
            location.remove(location.count()-2, 2);
            if(location == 0) break;
            settings.addLocation(location);
        }
        if(settings.getLocations().count() == 0)
            settings.setDirBrowserEnabled(false);
        else
            settings.setDirBrowserEnabled(true);
    }

    //If settings file does not exist, create one using default values
    QFile settingsfile("settings.dat");
    if(!settingsfile.exists()){
        settingsfile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream writeSettings(&settingsfile);
        writeSettings << "theme=Default" << endl
                      << "timelineDragPauseMusic=1" << endl
                      << "volume=50" << endl
                      << "mute=false" << endl
                      << "repeat=0" << endl
                      << "audioBalance=0" << endl;
        settingsfile.close();
    }

    settings.loadSettings();

    if(settings.getTheme() != QString("System")){
        QString themepath = QString("themes/") + settings.getTheme() + QString("/theme.style");
        //Load a theme from a file
        QFile stylefile(themepath);
        if(stylefile.open(QIODevice::ReadOnly)){
            QTextStream ts(&stylefile);
            QString style = ts.readAll();
            //Set that theme and then close theme file
            qApp->setStyleSheet(style);
            stylefile.close();
        }
        else{
            QMessageBox::warning(NULL, "Error!", "Could not load theme. Reverting to system style.");
            settings.setTheme(QString("System"));
        }
    }

    BASS_Init(-1,44100,0,0,NULL);
    BASS_InitPlugins();

    QMessageBox::information(NULL, "DISCLAIMER!","This program is ALPHA! Bugs 'n shit exist, be careful.");

    Jabdah j;

    j.show();
    a.exec();
    BASS_Free();
    return 0;
}

//Init all BASS's plugins so we can get more file formats
void BASS_InitPlugins(){
    //AAC
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bass_aac.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbass_aac.so", 0);
    #endif

    //AC3
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bass_ac3.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbass_ac3.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbass_ac3.dylib", 0);
    #endif

    //ALAC
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bass_alac.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbass_alac.so", 0);
    #endif

    //APE
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bass_ape.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbass_ape.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbass_ape.dylib", 0);
    #endif

    //FLAC
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bassflac.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbassflac.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbassflac.dylib", 0);
    #endif

    //MIDI
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bassmidi.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbassmidi.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbassmidi.dylib", 0);
    #endif

    //MPC
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bass_mpc.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbass_mpc.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbass_mpc.dylib", 0);
    #endif

    //OPUS
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bassopus.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbassopus.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbassopus.dylib", 0);
    #endif

    //SPX
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bass_spx.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbass_spx.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbass_spx.dylib", 0);
    #endif

    //TTA
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/bass_tta.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbass_tta.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbass_tta.dylib", 0);
    #endif

    //WV
    #ifdef _WIN32 // Windows/CE
        BASS_PluginLoad("plugins/basswv.dll", 0);
    #elif defined(__linux__) // Linux
        BASS_PluginLoad("plugins/libbasswv.so", 0);
    #else // OSX
        BASS_PluginLoad("plugins/libbasswv.dylib", 0);
    #endif

    return;
}
