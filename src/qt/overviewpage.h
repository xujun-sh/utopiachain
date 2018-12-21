// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UTOPIACOIN_QT_OVERVIEWPAGE_H
#define UTOPIACOIN_QT_OVERVIEWPAGE_H

#include <amount.h>

#include <QWidget>
#include <memory>

// ------------------------
#include <QDialog>
#include <QMainWindow>
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
#include <QMessageBox>
//------------------------
#include <qt/rpcconsole.h>


class ClientModel;
class TransactionFilterProxy;
class TxViewDelegate;
class PlatformStyle;
class WalletModel;

namespace Ui {
    class OverviewPage;
}

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** Overview ("home") page widget */
class OverviewPage : public QWidget
{
    Q_OBJECT

public:
    explicit OverviewPage(const PlatformStyle *platformStyle, QWidget *parent = 0);
    ~OverviewPage();

    void freshaccount();                 // --
    void addrecipientitem(int i);        // --

    void setClientModel(ClientModel *clientModel);
    void setWalletModel(WalletModel *walletModel);
    void showOutOfSyncWarning(bool fShow);
void setRPCConsole(RPCConsole *rpcconsole);

public Q_SLOTS:
    void setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance,
                    const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance);
void updateResultWindow(const QString &result);

Q_SIGNALS:
    void transactionClicked(const QModelIndex &index);
    void outOfSyncWarningClicked();
void sendRPCRequest(const QString &command);
	
private:
    Ui::OverviewPage *ui;
    ClientModel *clientModel;
    WalletModel *walletModel;
    CAmount currentBalance;
    CAmount currentUnconfirmedBalance;
    CAmount currentImmatureBalance;
    CAmount currentWatchOnlyBalance;
    CAmount currentWatchUnconfBalance;
    CAmount currentWatchImmatureBalance;

    TxViewDelegate *txdelegate;
    std::unique_ptr<TransactionFilterProxy> filter;
RPCConsole *rpcconsole;
	
private Q_SLOTS:
    // ------------------------------------------------------------
    void updateDisplayUnit();
    void handleTransactionClicked(const QModelIndex &index);
    void updateAlerts(const QString &warnings);
    void updateWatchOnlyLabels(bool showWatchOnly);
    void handleOutOfSyncWarningClicks();


        ///////////////////// Browser page //////////////////////////
    void on_pushButton_Bcleanallsetting_clicked();
    void on_pushButton_Breviewsearchsetting_clicked();
    void on_pushButton_Bgetsearchingresult_clicked();

    ///////////////////// Accounting page ///////////////////////
    void on_pushButton_Arefreshaccount_clicked();
    void on_pushButton_Areceivingaddressbook_clicked();
    void on_pushButton_Asendingaddressbook_clicked();

    ///////////////////// Exchange page /////////////////////////
    void on_tabWidget_tabBarClicked(int index);
                  /////// Sending page ///////
    void on_pushButton_ESchooseaddress_clicked();
    void on_pushButton_ESaddmorerecipient_clicked();
    void on_pushButton_ESreviewsendingorder_clicked();
    void on_pushButton_EScleanallsendinginput_clicked();
    void on_pushButton_ESputsendingorder_clicked();
                  /////// Exchanging page ///////
    void on_pushButton_EEchoosereceivingaddress_clicked();
    void on_pushButton_EEreviewexchangingorder_clicked();
    void on_pushButton_EEcleanallexchanginginputs_clicked();
    void on_pushButton_EEputexchangingorder_clicked();
                  /////// Listing page ///////
    void on_lineEdit_ELtotalamount_editingFinished();
    void on_comboBox_ELsamllestunit_currentTextChanged(const QString &arg1);
    void on_pushButton_ELcreateowneraddress_clicked();
    void on_radioButton_ELyourwallet_pressed();
    void on_radioButton_ELexchange_pressed();
    void on_pushButton_ELreviewlistingorder_clicked();
    void on_pushButton_ELcleanalllistinginputs_clicked();
    void on_pushButton_ELputlistingorder_clicked();
                  /////// Announcing page (Posting) ///////
    void on_textEdit_EPannouncementwindow_textChanged();
    void on_pushButton_EPreviewannouncingorder_clicked();
    void on_pushButton_EPcleanannouncinginput_clicked();
    void on_pushButton_EPpostannouncement_clicked();

    ///////////////////// Manager page //////////////////////////
    void on_pushButton_Mbackupwallet_clicked();
    void on_pushButton_Mencryptwallet_clicked();
    void on_pushButton_Mchangepassphrase_clicked();

    void on_pushButton_Msignmessage_clicked();
    void on_pushButton_Mverifymessage_clicked();

    void on_checkBox_Munactivenetwork_stateChanged(int arg1);
    void on_pushButton_Mnetworkinformation_clicked();
    void on_pushButton_Mmanagepeers_clicked();
    void on_pushButton_Mbannedlist_clicked();
    void on_pushButton_Maddnode_clicked();
    void on_pushButton_Mdisconnect_clicked();
    void on_pushButton_Maddbanned_clicked();
    void on_pushButton_Mclearallbanned_clicked();

    void on_spinBox_MsetofDBcache_valueChanged(const QString &arg1);
    void on_comboBox_Minterfacelanguage_activated(const QString &arg1);
    // --------------------------------------------------------------------
};

#endif // UTOPIACOIN_QT_OVERVIEWPAGE_H
