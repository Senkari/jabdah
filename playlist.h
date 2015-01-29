#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QTreeWidgetItem>



namespace Ui {
class Playlist;
}

class Jabdah; //Forward declaration

class Playlist : public QWidget
{
    Q_OBJECT
public:
    explicit Playlist(Jabdah *parent = 0);
    int playlistRow;
    int getItemCount();
    QString getText(int row, int column);
    QTreeWidgetItem *getItem(int row);
    void clear();
    void addItem(QTreeWidgetItem *item);
    void setSelected(QTreeWidgetItem *item, bool value);
    bool isRepeated();
    ~Playlist();


private slots:

    void on_addButton_clicked();

    void on_removeButton_clicked();

    void on_moveDownButton_clicked();

    void on_moveUpButton_clicked();

    void on_playlist_itemDoubleClicked(QTreeWidgetItem *item, int column);

private:
    Ui::Playlist *ui;
    Jabdah *jabdah;
};

Q_DECLARE_METATYPE(Playlist*)

#endif // PLAYLIST_H
