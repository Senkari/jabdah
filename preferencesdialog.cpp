#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"
#include "definitions.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include "bass.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::loadSettings(){

    ui->themeComboBox->setCurrentText(settings.getTheme());
    ui->panSlider->setValue(settings.getBalance());

    timelineDragPauseMusic = settings.isMusicPausedWhenTimelineIsDragged();
    if(settings.isMusicPausedWhenTimelineIsDragged()) ui->pauseTrackWhenDraggingCheckbox->setChecked(1);

    return;
}

void PreferencesDialog::scanThemes(){

    QDir themeDir("themes");

    ui->themeComboBox->clear();
    QStringList themeList = themeDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    ui->themeComboBox->addItem("System");
    ui->themeComboBox->addItems(themeList);

    return;
}

void PreferencesDialog::on_pauseTrackWhenDraggingCheckbox_stateChanged(int arg1)
{
    timelineDragPauseMusic = arg1;
}

void PreferencesDialog::on_cancelButton_clicked()
{
    BASS_ChannelSetAttribute(stream::audiofile, BASS_ATTRIB_PAN, settings.getBalance());
    this->close();
}

void PreferencesDialog::on_OKButton_clicked()
{
    settings.setTheme(ui->themeComboBox->currentText());
    settings.setOnTimelineDragPauseMusic(timelineDragPauseMusic);
    settings.setBalance(ui->panSlider->value());

    qApp->setStyleSheet("");
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

    this->close();
}

void PreferencesDialog::on_applyButton_clicked()
{
    QString theme = settings.getTheme();
    settings.setTheme(ui->themeComboBox->currentText());
    settings.setOnTimelineDragPauseMusic(timelineDragPauseMusic);
    settings.setBalance(ui->panSlider->value());

    if(settings.getTheme() != QString("System")){
        QString themepath = QString("themes/") + settings.getTheme() + QString("/theme.style");
        //Load a theme from a file
        QFile stylefile(themepath);
        if(stylefile.open(QIODevice::ReadOnly)){
            QTextStream ts(&stylefile);
            QString style = ts.readAll();
            //Set that theme and then close theme file
            qApp->setStyleSheet("");
            qApp->setStyleSheet(style);
            stylefile.close();
        }
        else{
            QMessageBox::warning(NULL, "Error!", "Could not load theme. Reverting to previous theme.");
            settings.getTheme() = theme;
            ui->themeComboBox->setCurrentText(settings.getTheme());
        }
    }
    else qApp->setStyleSheet("");
}

void PreferencesDialog::on_panSlider_sliderMoved(int position)
{
    if(stream::audiofile != 0) BASS_ChannelSetAttribute(stream::audiofile, BASS_ATTRIB_PAN, position);
}

void PreferencesDialog::on_resetButton_clicked()
{
    if (QMessageBox::question(NULL, "Confirmation",
                              "Are you sure you want to reset all your preferences?\nYou can still keep your original settings after pressing yes by pressing Cancel in the preferences window.",
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes){
        timelineDragPauseMusic = 1;
        ui->pauseTrackWhenDraggingCheckbox->setChecked(TRUE);

        ui->themeComboBox->setCurrentText("Default");

        ui->panSlider->setValue(0);
        if(stream::audiofile != 0) BASS_ChannelSetAttribute(stream::audiofile, BASS_ATTRIB_PAN, 0);
    }
}
