#include "addfolders.h"
#include "ui_addfolders.h"
#include <QMessageBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QTextStream>

#include "definitions.h"

Addfolders::Addfolders(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Addfolders)
{
    ui->setupUi(this);
}

Addfolders::~Addfolders()
{
    delete ui;
}

void Addfolders::on_okButton_clicked()
{
    if(ui->locationList->count() > 0) this->close();
    else{
        QMessageBox::information(NULL, "Hey!", "At least one folder must be added to the list.");
    }

}

void Addfolders::closeEvent(QCloseEvent *event){
    if(ui->locationList->count() > 0){
        QFile locationsfile("locations.dat");
        locationsfile.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream writeLocations(&locationsfile);
        for(int x = 0; x < ui->locationList->count(); x++){
            writeLocations << ui->locationList->item(x)->text() << endl;
            settings.addLocation(ui->locationList->item(x)->text());
        }
        locationsfile.close();
        settings.setDirBrowserEnabled(true);
        event->accept();
        this->close();
    }
    else if(QMessageBox::question(NULL, "Hey!",
                                  "The song browser is not available until at least one directory is added. If you want, you can add directories later from Settings -> Music location(s).\n\nAre you sure you want to skip this phase?",
                                  QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes){
        settings.setDirBrowserEnabled(false);
        event->accept();
        this->close();
    }
    else event->ignore();
}

void Addfolders::on_addDirectoryButton_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), QString());
    ui->locationList->addItem(dir);
}

void Addfolders::on_skipButton_clicked()
{
    if(ui->locationList->count() > 0){
        if(QMessageBox::question(NULL, "Hey!",
                            "The song browser is not available until at least one directory is added. If you want, you can add directories later from Settings -> Music location(s).\n\nAre you sure you want to skip this phase?",
                            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes){
            settings.setDirBrowserEnabled(false);
            this->close();
        }
    }
    else this->close();
}
