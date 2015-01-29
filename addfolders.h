#ifndef ADDFOLDERS_H
#define ADDFOLDERS_H

#include <QDialog>
#include <QCloseEvent>

#include "settings.h"

namespace Ui {
class Addfolders;
}

class Addfolders : public QDialog
{
    Q_OBJECT
    
public:
    explicit Addfolders(QWidget *parent = 0);
    ~Addfolders();
    
private slots:

    virtual void closeEvent(QCloseEvent *event);

    void on_okButton_clicked();

    void on_addDirectoryButton_clicked();

    void on_skipButton_clicked();

private:
    Ui::Addfolders *ui;
    Settings settings;
};

#endif // ADDFOLDERS_H
