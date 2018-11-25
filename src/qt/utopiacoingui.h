#ifndef UTOPIACOIN_QT_UTOPIACOINGUI_H
#define UTOPIACOIN_QT_UTOPIACOINGUI_H

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
#include <QDialog>
#include <QVector>

namespace Ui {
class UtopiacoinGUI;
}

class UtopiacoinGUI : public QMainWindow
{
    Q_OBJECT

public:
    static const QString DEFAULT_WALLET;
    static const std::string DEFAULT_UIPLATFORM;

    explicit UtopiacoinGUI(QWidget *parent = 0);
    ~UtopiacoinGUI();
    
    void recipientitem1(int i);

private Q_SLOTS:

    ///////////////////// Browser page //////////////////////////
    void on_pushButton_cleanallsetting_clicked();
    void on_pushButton_getsearchingresult_clicked();
    void on_pushButton_reviewsearchsetting_clicked();

    ///////////////////// Accounting page ///////////////////////
    void on_pushButton_receivingaddressbook_clicked();
    void on_pushButton_sendingaddressbook_clicked();

    ///////////////////// Exchange page /////////////////////////
    void on_tabWidget_tabBarClicked(int index);
                  /////// Sending page ///////
    void on_pushButton_chooseaddress_clicked();
    void on_pushButton_putsendingorder_clicked();
    void on_pushButton_addmorerecipient_clicked();
    void on_pushButton_cleanallsendinginput_clicked();

                  /////// Exchanging page ///////
    void on_pushButton_choosereceivingaddress_clicked();
    void on_pushButton_putexchangingorder_clicked();
    void on_pushButton_cleanallexchanginginputs_clicked();

                  /////// Listing page ///////
    void on_pushButton_createowneraddress_clicked();
    void on_pushButton_putlistingorder_clicked();
    void on_pushButton_cleanalllistinginputs_clicked();
    void on_lineEdit_totalamount_editingFinished();
    void on_comboBox_samllestunit_currentTextChanged(const QString &arg1);
    void on_radioButton_yourwallet_pressed();
    void on_radioButton_exchange_pressed();

                  /////// Announcing page ///////
    void on_textEdit_announcementwindow_textChanged();
    void on_pushButton_postannouncement_clicked();
    void on_pushButton_cleanannouncinginput_clicked();

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
    Ui::UtopiacoinGUI *ui;

};

#endif // MAINWINDOW_H
