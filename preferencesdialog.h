#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

#include "settings.h"

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT
    
public:

    void loadSettings();
    void scanThemes();
    explicit PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();
    
private slots:
    void on_pauseTrackWhenDraggingCheckbox_stateChanged(int arg1);

    void on_cancelButton_clicked();

    void on_OKButton_clicked();

    void on_applyButton_clicked();

    void on_panSlider_sliderMoved(int position);

    void on_resetButton_clicked();

private:

    bool timelineDragPauseMusic;
    Settings settings;
    Ui::PreferencesDialog *ui;
};

#endif // PREFERENCESDIALOG_H
