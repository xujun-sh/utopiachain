#ifndef UTOPIACOIN_QT_INTRO_H
#define UTOPIACOIN_QT_INTRO_H

#include <QMutex>
#include <QThread>

#include <QDialog>

#include <QMainWindow>
#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTableView>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QProgressBar>
#include <QFile>
#include <QFileInfo>
#include <QTabWidget>
#include <QVector>

static const bool DEFAULT_CHOOSE_DATADIR = false;
class FreespaceChecker;

namespace Ui {
class Intro;
}

class Intro : public QDialog
{
    Q_OBJECT

public:
    explicit Intro(QWidget *parent = 0);
    ~Intro();

    QString getDataDirectory();
    void setDataDirectory(const QString &dataDir);
    static bool pickDataDirectory();
    static QString getDefaultDataDirectory();

    void recipientitem1(int i);

Q_SIGNALS:
    void requestCheck();
    void stopThread();

public Q_SLOTS:
    void setStatus(int status, const QString &message, quint64 bytesAvailable);

private Q_SLOTS:
    void on_dataDirectory_textChanged(const QString &arg1);
    void on_ellipsisButton_clicked();
    void on_dataDirDefault_clicked();
    void on_dataDirCustom_clicked();

    ///////////////////// Browser page //////////////////////////
    void on_pushButton_cleanallsetting_clicked();
    void on_pushButton_reviewsearchsetting_clicked();
    void on_pushButton_getsearchingresult_clicked();

    ///////////////////// Accounting page ///////////////////////
    void on_pushButton_receivingaddressbook_clicked();
    void on_pushButton_sendingaddressbook_clicked();

    ///////////////////// Exchange page /////////////////////////
    void on_tabWidget_tabBarClicked(int index);
                  /////// Sending page ///////
    void on_pushButton_chooseaddress_clicked();
    void on_pushButton_addmorerecipient_clicked();
    void on_pushButton_cleanallsendinginput_clicked();
    void on_pushButton_reviewsendingorder_clicked();
    void on_pushButton_putsendingorder_clicked();
                  /////// Exchanging page ///////
    void on_pushButton_choosereceivingaddress_clicked();
    void on_pushButton_cleanallexchanginginputs_clicked();
    void on_pushButton_reviewexchangingorder_clicked();
    void on_pushButton_putexchangingorder_clicked();
                  /////// Listing page ///////
    void on_pushButton_createowneraddress_clicked();
    void on_pushButton_cleanalllistinginputs_clicked();
    void on_pushButton_reviewlistingorder_clicked();
    void on_pushButton_putlistingorder_clicked();

    void on_lineEdit_totalamount_editingFinished();
    void on_comboBox_samllestunit_currentTextChanged(const QString &arg1);
    void on_radioButton_yourwallet_pressed();
    void on_radioButton_exchange_pressed();
                  /////// Announcing page ///////
    void on_textEdit_announcementwindow_textChanged();
    void on_pushButton_cleanannouncinginput_clicked();
    void on_pushButton_reviewannouncingorder_clicked();
    void on_pushButton_postannouncement_clicked();

    ///////////////////// Manager page //////////////////////////
    void on_pushButton_leaveutopiamarketplace_clicked();
    void on_pushButton_backupwallet_clicked();
    void on_pushButton_encryptwallet_clicked();
    void on_pushButton_changepassphrase_clicked();
    void on_pushButton_signmessage_clicked();
    void on_pushButton_verifymessage_clicked();
    void on_pushButton_networkinformation_clicked();
    void on_pushButton_managepeers_clicked();

private:
    Ui::Intro *ui;

    QThread *thread;
    QMutex mutex;
    bool signalled;
    QString pathToCheck;

    void startThread();
    void checkPath(const QString &dataDir);
    QString getPathToCheck();

    friend class FreespaceChecker;
};

#endif // UTOPIACOIN_QT_INTRO_H
