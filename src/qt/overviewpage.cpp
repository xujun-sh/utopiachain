
// Copyright (c) 2011-2017 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/overviewpage.h>
#include <qt/forms/ui_overviewpage.h>

#include <qt/utopiacoinunits.h>
#include <qt/clientmodel.h>
#include <qt/guiconstants.h>
#include <qt/guiutil.h>
#include <qt/optionsmodel.h>
#include <qt/platformstyle.h>
#include <qt/transactionfilterproxy.h>
#include <qt/transactiontablemodel.h>
#include <qt/walletmodel.h>
#include <qt/rpcconsole.h>
#include <validation.h>

#include <QAbstractItemDelegate>
#include <QPainter>

#define DECORATION_SIZE 54
#define NUM_ITEMS 5

class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    explicit TxViewDelegate(const PlatformStyle *_platformStyle, QObject *parent=nullptr):
        QAbstractItemDelegate(parent), unit(UtopiacoinUnits::UTC),
        platformStyle(_platformStyle)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(TransactionTableModel::RawDecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon = platformStyle->SingleColorIcon(icon);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(value.canConvert<QBrush>())
        {
            QBrush brush = qvariant_cast<QBrush>(value);
            foreground = brush.color();
        }

        painter->setPen(foreground);
        QRect boundingRect;
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address, &boundingRect);

        if (index.data(TransactionTableModel::WatchonlyRole).toBool())
        {
            QIcon iconWatchonly = qvariant_cast<QIcon>(index.data(TransactionTableModel::WatchonlyDecorationRole));
            QRect watchonlyRect(boundingRect.right() + 5, mainRect.top()+ypad+halfheight, 16, halfheight);
            iconWatchonly.paint(painter, watchonlyRect);
        }

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = UtopiacoinUnits::formatWithUnit(unit, amount, true, UtopiacoinUnits::separatorAlways);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;
    const PlatformStyle *platformStyle;

};
#include <qt/overviewpage.moc>

OverviewPage::OverviewPage(const PlatformStyle *platformStyle, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OverviewPage),
    clientModel(0),
    walletModel(0),
    currentBalance(-1),
    currentUnconfirmedBalance(-1),
    currentImmatureBalance(-1),
    currentWatchOnlyBalance(-1),
    currentWatchUnconfBalance(-1),
    currentWatchImmatureBalance(-1),
    txdelegate(new TxViewDelegate(platformStyle, this))
{
    ui->setupUi(this);

    // use a SingleColorIcon for the "out of sync warning" icon
    QIcon icon = platformStyle->SingleColorIcon(":/icons/warning");
    icon.addPixmap(icon.pixmap(QSize(64,64), QIcon::Normal), QIcon::Disabled); // also set the disabled icon because we are using a disabled QPushButton to work around missing HiDPI support of QLabel (https://bugreports.qt.io/browse/QTBUG-42503)
    ui->labelTransactionsStatus->setIcon(icon);
    ui->labelWalletStatus->setIcon(icon);

    // Recent transactions
    ui->listTransactions->setItemDelegate(txdelegate);
    ui->listTransactions->setIconSize(QSize(DECORATION_SIZE, DECORATION_SIZE));
    ui->listTransactions->setMinimumHeight(NUM_ITEMS * (DECORATION_SIZE + 2));
    ui->listTransactions->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->listTransactions, SIGNAL(clicked(QModelIndex)), this, SLOT(handleTransactionClicked(QModelIndex)));

    // start with displaying the "out of sync" warnings
    showOutOfSyncWarning(true);
    connect(ui->labelWalletStatus, SIGNAL(clicked()), this, SLOT(handleOutOfSyncWarningClicks()));
    connect(ui->labelTransactionsStatus, SIGNAL(clicked()), this, SLOT(handleOutOfSyncWarningClicks()));

//-------------------------------------------------------------------------------------------------------------------------
        /////////////////////////////////////General setting///////////////////////////////////////
        ui->tabWidget->setCurrentIndex(0);                                                        // always open the Browser page first.

                  //////Validator for number, string////////
        QDoubleValidator *pDoubleValidator = new QDoubleValidator(this);
            pDoubleValidator->setRange(0, 1000000000);                                            // double number
            pDoubleValidator->setNotation(QDoubleValidator::StandardNotation);
            pDoubleValidator->setDecimals(8);
        ui->lineEdit_Bamount->setValidator(pDoubleValidator);
        ui->lineEdit_EEinamount->setValidator(pDoubleValidator);
        ui->lineEdit_EEoutamount->setValidator(pDoubleValidator);
        ui->lineEdit_ELwithUMpTamount->setValidator(pDoubleValidator);

        ui->lineEdit_ELtotalamount->setValidator(new QIntValidator(1, 2100000000));               // integer
        ui->lineEdit_EEouttimelimit->setValidator(new QIntValidator(1, 1440));

        QRegExp addrreg("^[1][123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$");    // prefix with 1, base58
        ui->lineEdit_EEinaddress->setValidator(new QRegExpValidator(addrreg, this));
        ui->lineEdit_ELowneraddress->setValidator(new QRegExpValidator(addrreg, this));

        QRegExp assetsymbolreg("^[123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$");// base58
        ui->lineEdit_ELassetsymbol->setValidator(new QRegExpValidator(assetsymbolreg, this));

        QRegExp searchreg("^[0123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$");    // base58 and 0
        ui->lineEdit_Bsearch->setValidator(new QRegExpValidator(searchreg, this));
                  ////////////treewidget///////////
        ui->treeWidget_EA->header()->setSortIndicator(0,Qt::AscendingOrder);                      // sort the asset from 0 up

        for (int i=0; i< ui->treeWidget_EA->topLevelItemCount(); ++i){
             QString itemSymboltxt= ui->treeWidget_EA->topLevelItem(i)->text(1);                  // List toplevel i's col 1
             ui->comboBox_Bsymbol->setItemText(i+1, itemSymboltxt);
             ui->comboBox_ESassetsymbol->setItemText(i, itemSymboltxt);
             ui->comboBox_EEinassetsymbol->setItemText(i, itemSymboltxt);
             ui->comboBox_EEoutassetsymbol->setItemText(i, itemSymboltxt);
        }
    ///////////////////////////////////Browser/////////////////////////////////////////////////
         // result window's default result: All asset, Opening, Sell & Buy, All amount

    ////////////////////Accounting & all comboBox of asset symbol//////////////////////////////
        ui->tableWidget_Aaccountwindow->setEditTriggers(QAbstractItemView::NoEditTriggers);       // edit not allowed
        ui->tableWidget_Aaccountwindow->resizeColumnsToContents();                                // horizental size fit contents
        ui->tableWidget_Aaccountwindow->horizontalHeader()->setDefaultSectionSize(190);           // column width
        ui->tableWidget_Aaccountwindow->verticalHeader()->setDefaultSectionSize(25);

        freshaccount();                                                                           // fresh data in account page
//--------------------------------------------------------------------------------------------------------------------------
}


void OverviewPage::handleTransactionClicked(const QModelIndex &index)
{
    if(filter)
        Q_EMIT transactionClicked(filter->mapToSource(index));
}

void OverviewPage::handleOutOfSyncWarningClicks()
{
    Q_EMIT outOfSyncWarningClicked();
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(const CAmount& balance, const CAmount& unconfirmedBalance, const CAmount& immatureBalance, const CAmount& watchOnlyBalance, const CAmount& watchUnconfBalance, const CAmount& watchImmatureBalance)
{
    int unit = walletModel->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    currentWatchOnlyBalance = watchOnlyBalance;
    currentWatchUnconfBalance = watchUnconfBalance;
    currentWatchImmatureBalance = watchImmatureBalance;
    ui->labelBalance->setText(UtopiacoinUnits::formatWithUnit(unit, balance, false, UtopiacoinUnits::separatorAlways));
    ui->labelUnconfirmed->setText(UtopiacoinUnits::formatWithUnit(unit, unconfirmedBalance, false, UtopiacoinUnits::separatorAlways));
    ui->labelImmature->setText(UtopiacoinUnits::formatWithUnit(unit, immatureBalance, false, UtopiacoinUnits::separatorAlways));
    ui->labelTotal->setText(UtopiacoinUnits::formatWithUnit(unit, balance + unconfirmedBalance + immatureBalance, false, UtopiacoinUnits::separatorAlways));
    ui->labelWatchAvailable->setText(UtopiacoinUnits::formatWithUnit(unit, watchOnlyBalance, false, UtopiacoinUnits::separatorAlways));
    ui->labelWatchPending->setText(UtopiacoinUnits::formatWithUnit(unit, watchUnconfBalance, false, UtopiacoinUnits::separatorAlways));
    ui->labelWatchImmature->setText(UtopiacoinUnits::formatWithUnit(unit, watchImmatureBalance, false, UtopiacoinUnits::separatorAlways));
    ui->labelWatchTotal->setText(UtopiacoinUnits::formatWithUnit(unit, watchOnlyBalance + watchUnconfBalance + watchImmatureBalance, false, UtopiacoinUnits::separatorAlways));

    // only show immature (newly mined) balance if it's non-zero, so as not to complicate things
    // for the non-mining users
    bool showImmature = immatureBalance != 0;
    bool showWatchOnlyImmature = watchImmatureBalance != 0;

    // for symmetry reasons also show immature label when the watch-only one is shown
    ui->labelImmature->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelImmatureText->setVisible(showImmature || showWatchOnlyImmature);
    ui->labelWatchImmature->setVisible(showWatchOnlyImmature); // show watch-only immature balance
}

// show/hide watch-only labels
void OverviewPage::updateWatchOnlyLabels(bool showWatchOnly)
{
    ui->labelSpendable->setVisible(showWatchOnly);      // show spendable label (only when watch-only is active)
    ui->labelWatchonly->setVisible(showWatchOnly);      // show watch-only label
    //ui->lineWatchBalance->setVisible(showWatchOnly);    // show watch-only balance separator line
    ui->labelWatchAvailable->setVisible(showWatchOnly); // show watch-only available balance
    ui->labelWatchPending->setVisible(showWatchOnly);   // show watch-only pending balance
    ui->labelWatchTotal->setVisible(showWatchOnly);     // show watch-only total balance

    if (!showWatchOnly)
        ui->labelWatchImmature->hide();
}

void OverviewPage::setClientModel(ClientModel *model)
{
    this->clientModel = model;
    if(model)
    {
        // Show warning if this is a prerelease version
        connect(model, SIGNAL(alertsChanged(QString)), this, SLOT(updateAlerts(QString)));
        updateAlerts(model->getStatusBarWarnings());
    }
}

void OverviewPage::setWalletModel(WalletModel *model)
{
    this->walletModel = model;
    if(model && model->getOptionsModel())
    {
        // Set up transaction list
        filter.reset(new TransactionFilterProxy());
        filter->setSourceModel(model->getTransactionTableModel());
        filter->setLimit(NUM_ITEMS);
        filter->setDynamicSortFilter(true);
        filter->setSortRole(Qt::EditRole);
        filter->setShowInactive(false);
        filter->sort(TransactionTableModel::Date, Qt::DescendingOrder);

        ui->listTransactions->setModel(filter.get());
        ui->listTransactions->setModelColumn(TransactionTableModel::ToAddress);

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getUnconfirmedBalance(), model->getImmatureBalance(),
                   model->getWatchBalance(), model->getWatchUnconfirmedBalance(), model->getWatchImmatureBalance());
        connect(model, SIGNAL(balanceChanged(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)), this, SLOT(setBalance(CAmount,CAmount,CAmount,CAmount,CAmount,CAmount)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

        updateWatchOnlyLabels(model->haveWatchOnly());
        connect(model, SIGNAL(notifyWatchonlyChanged(bool)), this, SLOT(updateWatchOnlyLabels(bool)));
    }

    // update the display unit, to not use the default ("UTC")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(walletModel && walletModel->getOptionsModel())
    {
        if(currentBalance != -1)
            setBalance(currentBalance, currentUnconfirmedBalance, currentImmatureBalance,
                       currentWatchOnlyBalance, currentWatchUnconfBalance, currentWatchImmatureBalance);

        // Update txdelegate->unit with the current unit
        txdelegate->unit = walletModel->getOptionsModel()->getDisplayUnit();

        ui->listTransactions->update();
    }
}

void OverviewPage::updateAlerts(const QString &warnings)
{
    this->ui->labelAlerts->setVisible(!warnings.isEmpty());
    this->ui->labelAlerts->setText(warnings);
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    ui->labelWalletStatus->setVisible(fShow);
    ui->labelTransactionsStatus->setVisible(fShow);
}

void OverviewPage::setRPCConsole(RPCConsole *_rpcconsole)
{
    if (_rpcconsole)
    {
        this->rpcconsole = _rpcconsole;
 connect(this,SIGNAL(sendRPCRequest(QString)),_rpcconsole,SLOT(receiveCommand(QString)));
connect(_rpcconsole,SIGNAL(sendOutput(QString)),this,SLOT(updateResultWindow(QString)));

    }
}

//--------------------------------------------------------------------------------------------------------

void OverviewPage::freshaccount()
{
    for (int i=0; i< ui->treeWidget_EA->topLevelItemCount(); ++i){
        // input Asset Symbol col - Asset List Tree
        QString itemSymboltxt= ui->treeWidget_EA->topLevelItem(i)->text(1);           // List toplevel i's col 1
        QTableWidgetItem *itemSymbol = new QTableWidgetItem();
        itemSymbol->setText(itemSymboltxt);                                           // get the asset symbol
        ui->tableWidget_Aaccountwindow->setItem(i, 0, itemSymbol);

        // input total col for each asset
        QString itemTotaltxt = "1234567890.12345678";                                 // need RPC
        QTableWidgetItem *itemTotal = new QTableWidgetItem;
        itemTotal->setText(itemTotaltxt);
        ui->tableWidget_Aaccountwindow->setItem(i, 3, itemTotal);

        // input available col for each asset
        QString itemAvailabletxt = "24.87654321";                                     // need RPC
        QTableWidgetItem *itemAvailable = new QTableWidgetItem;
        itemAvailable->setText(itemAvailabletxt);
        ui->tableWidget_Aaccountwindow->setItem(i, 1, itemAvailable);

        // input pending col for each asset
        double itemPendingdouble = itemTotaltxt.toDouble() - itemAvailabletxt.toDouble();
        QString itemPendingtxt = QString::number(itemPendingdouble, 'f', 8);           // get the pending value
//        QString itemPendingtxt = QString("%1").arg(itemPendingdouble, 0, 'f', 8);
        QTableWidgetItem *itemPending = new QTableWidgetItem;
        itemPending->setText(itemPendingtxt);
        ui->tableWidget_Aaccountwindow->setItem(i, 2, itemPending);
    }
}

void OverviewPage::addrecipientitem(int i)                                                 // for sending asset page
{
    if (i >1 && i <11)
        ui->tableWidget_ESaddmorerecipient->setRowCount(ui->tableWidget_ESaddmorerecipient->rowCount()+3);

    QDoubleValidator *pDoubleValidator = new QDoubleValidator(this);
        pDoubleValidator->setRange(0, 1000000000);                                          // set double number
        pDoubleValidator->setNotation(QDoubleValidator::StandardNotation);
        pDoubleValidator->setDecimals(8);
    QRegExp addrreg("^[1][123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$");  // base58 and prefix 1

    QLabel *newrecipientaddress = new QLabel(QString ("Recipient Address%1:").arg(i));      // for sending
    QLabel *newreceivingamount = new QLabel(QString("  Reciving Amount:"));                 // for sending
    QPushButton *newchooseaddress = new QPushButton(QString("Choose Address"));             // for sending
    QLineEdit *lineEdit_newrecipientaddress = new QLineEdit();
       lineEdit_newrecipientaddress->setClearButtonEnabled(true);
       lineEdit_newrecipientaddress->setMaxLength(35);
       lineEdit_newrecipientaddress->setValidator(new QRegExpValidator(addrreg, this));
    QLineEdit *lineEdit_newreceivingamount = new QLineEdit();
       lineEdit_newreceivingamount->setClearButtonEnabled(true);
       lineEdit_newreceivingamount->setMaxLength(19);
       lineEdit_newreceivingamount->setValidator(pDoubleValidator);

    QFrame *line0 = new QFrame;
    line0->setFrameShape(QFrame::HLine);
    QFrame *line1 = new QFrame;
    line1->setFrameShape(QFrame::HLine);
    QFrame *line2 = new QFrame;
    line2->setFrameShape(QFrame::HLine);

    ui->tableWidget_ESaddmorerecipient->setColumnWidth(0,130);
    ui->tableWidget_ESaddmorerecipient->setRowHeight((i-1)*3,10);
    ui->tableWidget_ESaddmorerecipient->setColumnWidth(1,380);
    ui->tableWidget_ESaddmorerecipient->setRowHeight((i-1)*3+1,22);
    ui->tableWidget_ESaddmorerecipient->setColumnWidth(2,115);
    ui->tableWidget_ESaddmorerecipient->setRowHeight((i-1)*3+2,22);

    ui->tableWidget_ESaddmorerecipient->setCellWidget((i-1)*3,0,line0);
    ui->tableWidget_ESaddmorerecipient->setCellWidget((i-1)*3,1,line1);
    ui->tableWidget_ESaddmorerecipient->setCellWidget((i-1)*3,2,line2);

    ui->tableWidget_ESaddmorerecipient->setCellWidget((i-1)*3+1,0,newrecipientaddress);
    ui->tableWidget_ESaddmorerecipient->setCellWidget((i-1)*3+1,1,lineEdit_newrecipientaddress);
    ui->tableWidget_ESaddmorerecipient->setCellWidget((i-1)*3+1,2,newchooseaddress);

    ui->tableWidget_ESaddmorerecipient->setCellWidget((i-1)*3+2,0,newreceivingamount);
    ui->tableWidget_ESaddmorerecipient->setCellWidget((i-1)*3+2,1,lineEdit_newreceivingamount);
}

///////////////////// Browser page //////////////////////////////////////////////////////////

void OverviewPage::on_pushButton_Bcleanallsetting_clicked()
{    // clean all setting
     ui->comboBox_Bsymbol->setCurrentIndex(0);
     ui->comboBox_Btime->setCurrentIndex(0);
     ui->comboBox_Btype->setCurrentIndex(0);
     ui->lineEdit_Bamount->clear();
     ui->lineEdit_Bsearch->clear();
     ui->textEdit_Bresultwindow->clear();
     ui->textEdit_Bsearchsettingsummary->clear();
}

void OverviewPage::on_pushButton_Breviewsearchsetting_clicked()
{
    ui->textEdit_Bsearchsettingsummary->clear();
    QString searchsettingsummary = "                                             <<Search Setting Summary>> \n";
    QString b_assetsymbol = ui->comboBox_Bsymbol->currentText();
    QString b_time = ui->comboBox_Btime->currentText();
    QString b_type = ui->comboBox_Btype->currentText();
    QString b_amount = ui->lineEdit_Bamount->text();
    QString b_search = ui->lineEdit_Bsearch->text();
    if (!b_search.isEmpty()){   // txid and block hash are both 32 bytes 64 characters     // address is 26-35 alphanumeric characters
        ui->comboBox_Bsymbol->setCurrentIndex(0);
        ui->comboBox_Btime->setCurrentIndex(0);
        ui->comboBox_Btype->setCurrentIndex(0);
        ui->lineEdit_Bamount->setText("");
        b_amount = QString();
        if (b_search.length() == 64){
            searchsettingsummary += QString("Search for the block/transaction that with:\n    block hash / transaction ID (%1).").arg(b_search);
        } else if (b_search.length() < 9 ){
            searchsettingsummary += QString("Search for the block that with:\n    block height (%1).").arg(b_search);
        } else if (b_search.length() > 25 && b_search.length() < 36){
            searchsettingsummary += QString("Search for the address that with:\n    address (%1).").arg(b_search);
        } else {
            searchsettingsummary = QString("Wrong!\n input must be block height, or block hash, or txid, or address.");
            ui->textEdit_Bsearchsettingsummary->setText(searchsettingsummary);
            return;
        }
    } else {
        if (b_time  == "Opening" && ui->comboBox_Btype->currentIndex() < 3) {
            searchsettingsummary += QString(" Search for opening");
        } else if (b_time != "Opening" && ui->comboBox_Btype->currentIndex() > 3){
            searchsettingsummary += QString(" Search for ");
        } else {
            searchsettingsummary = QString("Wrong! \n only For Buy, For Sell, and Buy & Sell 3 kinds of type match to Opening time.");
            ui->textEdit_Bsearchsettingsummary->setText(searchsettingsummary);
            return;
        }
        searchsettingsummary += QString("Transactions with:\n Asset symbol as <%1>; \n All (%2) type opening transactions; \n Asset amount")
                                        .arg(b_assetsymbol).arg(b_type);
        if (b_amount == ""){
            searchsettingsummary += QString(" for all.");
        } else {
            searchsettingsummary += QString(" no more than (%3).").arg(b_amount);
        }
    }
    ui->textEdit_Bsearchsettingsummary->setText(searchsettingsummary);
}

void OverviewPage::on_pushButton_Bgetsearchingresult_clicked()
{    // search required and show the result in the textEdit.

//       #include <validation.h>
       
//ui->textEdit_Bresultwindow->setText(QString::fromStdString(chainActive.Tip()->GetBlockHash().GetHex()));

    // lineEdit_search input is exclusived with all other settings (Asset, Time, Type, and or Amount).
    QString b_assetsymbol = ui->comboBox_Bsymbol->currentText();
    QString b_time =        ui->comboBox_Btime->currentText();
    QString b_type =        ui->comboBox_Btype->currentText();
    QString b_amount =      ui->lineEdit_Bamount->text();
    QString b_search =      ui->lineEdit_Bsearch->text();
    QString b_searchresult = QString();
/*
CBlockIndex *pindexStart = chainActive.Genesis();
    if (!b_search.isNull()) {
        pindexStart = chainActive[b_search.toInt()];
        if (!pindexStart) {
            ui->textEdit_Bresultwindow->setText(QString::fromStdString("Invalid height"));
        }else{
	    ui->textEdit_Bresultwindow->setText(QString::fromStdString(pindexStart->ToString()));
	}
    }
*/
    if (b_search != ""){   // for search input only
        if (b_search.length() == 64){
             if (b_search.left(2) == "00"){
//               bsearchresult = getblock(b_search, 1);                                                // need RPC
                 //return;
		Q_EMIT sendRPCRequest(QString("getblock(") + b_search + QString("1)"));
             } else {
//               b_searchresult = getblock(getblockhash(b_search), 1);                                 // need RPC
		Q_EMIT sendRPCRequest(QString("getblock(getblockhash(") + b_search + QString("))"));
                 if (b_searchresult.isNull()){
			//gettransaction
//                   b_searchresult = getrawtransaction(b_search, 1);                                  // need RPC
			Q_EMIT sendRPCRequest(QString("getrawtransaction(") + b_search + QString("))"));
                  }
             }
        }
    }
    else if (b_time == "Opening" && ui->comboBox_Btype->currentIndex() < 3) {

    }
    else if (b_time != "Opening" && ui->comboBox_Btype->currentIndex() > 3 && b_amount == "") {

    }
Q_EMIT sendRPCRequest(QString("getblock(getblockhash(") + b_search + QString("))"));
//QString executableCommand = " getblockhash 0 ";
//rpcconsole->message(RPCConsole::CMD_REQUEST, executableCommand);


/*
std::string result;
std::string executableCommand = " getblockhash(0) ";
if(!RPCConsole::RPCExecuteCommandLine(result, executableCommand)){
	ui->textEdit_Bresultwindow->setText(QString::fromStdString(result));
}else{
	ui->textEdit_Bresultwindow->setText(b_search);
}
*/
    //ui->textEdit_Bresultwindow->setText(b_search);
     // rpc: listsinceblock (getbestblockhash);                       // default result for browser    need RPC
}

void OverviewPage::updateResultWindow(const QString &result)
{
    ui->textEdit_Bresultwindow->setText(result);
}

///////////////////// Accounting page ////////////////////////////////////////////////////////
void OverviewPage::on_pushButton_Arefreshaccount_clicked()
{
   freshaccount();
}

void OverviewPage::on_pushButton_Areceivingaddressbook_clicked()
{  // open Receiving Address Book

}

void OverviewPage::on_pushButton_Asendingaddressbook_clicked()
{  // open Sending Address Book

}

///////////////////// Exchange page //////////////////////////////////////////////////////////
void OverviewPage::on_tabWidget_tabBarClicked(int index)                                     // if any top level tab is clicked 
{
    on_pushButton_Bcleanallsetting_clicked();
    on_pushButton_EScleanallsendinginput_clicked();
    on_pushButton_ELcleanalllistinginputs_clicked();
    on_pushButton_EEcleanallexchanginginputs_clicked();
    on_pushButton_EPcleanannouncinginput_clicked();

    ui->tabWidget_Eexchange->setCurrentIndex(0);                                              // open the Sending page first.
    ui->label_EStotalrecipientnumber->setText("1 Recipient");
}

              /////// Sending page ///////
int              totalrecipientnumber = 1;
QVector<QString> vector_recipientaddress(10);
QVector<QString> vector_recipientamount(10);
double           sendingamountsummary = 0.00000000;
QString          sendingsummary = QString();
void OverviewPage::on_pushButton_ESaddmorerecipient_clicked()
{    // once click will add one more recipient (one groupBox_singlerecipientinfo)
    totalrecipientnumber++;
    if (totalrecipientnumber > 10)                                                               // limit the recipient to 10
        return;
    addrecipientitem(totalrecipientnumber);
    ui->label_EStotalrecipientnumber->setText((QString::number(totalrecipientnumber)) + " Recipients");
}

void OverviewPage::on_pushButton_EScleanallsendinginput_clicked()
{    // clean all input data.
    ui->comboBox_ESassetsymbol->setCurrentIndex(0);
    ui->tableWidget_ESaddmorerecipient->clearContents();
    ui->tableWidget_ESaddmorerecipient->setRowCount(3);
    addrecipientitem(1);
    ui->label_EStotalrecipientnumber->setText("1 Recipient");
    vector_recipientaddress.fill("", -1);
    vector_recipientamount.fill("", -1);
    sendingamountsummary = 0.00000000;
    ui->textEdit_ESsendingsummary->clear();
}

void OverviewPage::on_pushButton_ESchooseaddress_clicked()
{    // open the Sending Address Book to chose a address for sending asset
}

void OverviewPage::on_pushButton_ESreviewsendingorder_clicked()
{
    QString symbol = ui->comboBox_ESassetsymbol->currentText();
    int nullrecivientnum = 0;
    sendingsummary += QString("Send the asset symbol <%1> to:\n").arg(symbol);
    for (int i = 1; i<=ui->tableWidget_ESaddmorerecipient->rowCount()/3; ++i){
        QWidget   *widgetaddress=ui->tableWidget_ESaddmorerecipient->cellWidget((i-1)*3+1,1);    //create a widget
        QWidget   *widgetamount=ui->tableWidget_ESaddmorerecipient->cellWidget((i-1)*3+2,1);
        QLineEdit *recipientaddress1=(QLineEdit*)widgetaddress;                                  //force to change to type QLineEdit
        QLineEdit *recipientamount1=(QLineEdit*)widgetamount;
        QString   tempaddress = recipientaddress1->text();
        QString   tempamount = recipientamount1->text();

        vector_recipientaddress[i-1] = tempaddress;
        vector_recipientamount[i-1] = tempamount;
        if (!tempaddress.isNull() && !tempamount.isNull()){
            sendingsummary += QString("   %2: address/amount  (%3 / %4).\n").arg(i).arg(tempaddress).arg(tempamount);
            sendingamountsummary += tempamount.toDouble();  
        } else  
            nullrecivientnum++;               
                       
    }

    QString a = QString::number(sendingamountsummary,'f',8);

    sendingsummary += QString("The total sending address is (%1), the total sending amount is (%2).")
                              .arg(ui->tableWidget_ESaddmorerecipient->rowCount()/3-nullrecivientnum).arg(a);
    if (sendingamountsummary > 0) {
        ui->textEdit_ESsendingsummary->setText(sendingsummary);
        sendingamountsummary = 0.00000000;
        sendingsummary = "";
        nullrecivientnum = 0;      
    } else  
        on_pushButton_EScleanallsendinginput_clicked(); 
    
}

void OverviewPage::on_pushButton_ESputsendingorder_clicked()
{// build the sending asset transaction and sent it out
}
              /////// Exchanging page ///////
void OverviewPage::on_pushButton_EEchoosereceivingaddress_clicked()
{  // open Receiving Address Book to chose a receiving address

}

void OverviewPage::on_pushButton_EEreviewexchangingorder_clicked()
{  // construct exchanging transactions and send them.
    QString exchangingsummary = QString();
    QString outsymbol = ui->comboBox_EEoutassetsymbol->currentText();
    QString insymbol =  ui->comboBox_EEinassetsymbol->currentText();
    double outamount =  ui->lineEdit_EEoutamount->text().toDouble();
    double inamount =   ui->lineEdit_EEinamount->text().toDouble();
    QString soutamount =QString::number(outamount,'f',8);
    QString sinamount = QString::number(inamount,'f',8);
    QString inaddress = ui->lineEdit_EEinaddress->text();
    int outtimelimit =  ui->lineEdit_EEouttimelimit->text().toInt();

    exchangingsummary = QString("Exchange out: amount (%1) of asset symbol <%2>"
                              "\n                                          <--->   "
                              "\nExchange in:  amount (%3) of asset symbol <%4> and save to address (%5)\n")
                              .arg(soutamount).arg(outsymbol).arg(sinamount).arg(insymbol).arg(inaddress);
    if (outtimelimit>0)
        exchangingsummary += QString("\nThe time limit: for this exchange is %1 block (about 10 minute per block).\n").arg(outtimelimit); 
    
    if (ui->checkBox_EEinpartially->isChecked())
        exchangingsummary += QString("The exchange amount is flexable (<=%1) at the same exchange rate of (%2 <%3> / %4 <%5>)")
                                    .arg(sinamount).arg(sinamount).arg(insymbol).arg(soutamount).arg(outsymbol);     
    
    if(outamount > 0 && inamount > 0 && inaddress != "" )   {
        ui->textEdit_EEexchangingsummary->setText(exchangingsummary);
        exchangingsummary = QString();
    } else  
        on_pushButton_EEcleanallexchanginginputs_clicked();
    
}

void OverviewPage::on_pushButton_EEcleanallexchanginginputs_clicked()
{   // clean all exchanging inputs
    ui->comboBox_EEoutassetsymbol->setCurrentIndex(0);
    ui->lineEdit_EEouttimelimit->clear();
    ui->lineEdit_EEoutamount->clear();
    ui->comboBox_EEinassetsymbol->setCurrentIndex(0);
    ui->lineEdit_EEinamount->clear();
    ui->checkBox_EEinpartially->setChecked(false);
    ui->lineEdit_EEinaddress->clear();
    ui->textEdit_EEexchangingsummary->clear();
}

void OverviewPage::on_pushButton_EEputexchangingorder_clicked()
{// construct exchanging transactions and send them.

}
              /////// Listing page ///////
double totalamount = 0;
double smallestunit = 1.00000000;
QString sneededUMpSamount = QString();
QString ssmallestunit = QString();

void OverviewPage::on_comboBox_ELsamllestunit_currentTextChanged(const QString &arg1){
    smallestunit = arg1.toDouble();
    double neededUMpSamount = totalamount / smallestunit;
    QString sneededUMpSamount = QString::number(neededUMpSamount,'f',0);
    ui->label_ELamountofUMpSneeded->setText(sneededUMpSamount);
    sneededUMpSamount = QString();
}

void OverviewPage::on_lineEdit_ELtotalamount_editingFinished()    {
    totalamount = ui->lineEdit_ELtotalamount->text().toDouble();
    ssmallestunit = QString::number(smallestunit,'f',8);
    on_comboBox_ELsamllestunit_currentTextChanged(ssmallestunit);
    ssmallestunit = 1.00000000;
}

void OverviewPage::on_pushButton_ELcreateowneraddress_clicked()
{    // generate a grand new  address for the new listing asset owner

}

void OverviewPage::on_radioButton_ELexchange_pressed()  { // show the Available UMpT amount

}

void OverviewPage::on_radioButton_ELyourwallet_pressed()          {
    ui->lineEdit_ELwithUMpTamount->clear();                                // if your wallet button selected, clear With UMpT Amount
    ui->label_ELavailableUMpT->setText("Available UMpT");
}

void OverviewPage::on_pushButton_ELreviewlistingorder_clicked()
{
    QString listingsummary = QString();
    QString assetsymbol =   ui->lineEdit_ELassetsymbol->text();
    QString assetname =     ui->lineEdit_ELassetname->text();
    QString owneraddress =  ui->lineEdit_ELowneraddress->text();
    QString description =   ui->lineEdit_ELdescription->text();
    double withUMpTamount = ui->lineEdit_ELwithUMpTamount->text().toDouble();
    QString swithUMpTamount =   QString::number(withUMpTamount,'f',8);
    QString stotalamount =      QString::number(totalamount,'f',0);

    listingsummary = QString("Listing new exchangable asset:\n"
                             "   asset symbol: <%1>;   asset name: (%2);   total amount: (%3);   smallest unit:  (%4);  \n"
                             "   owner address: (%5);\n   description: (%6).\nThis listing needed amount (%7) of UMpS <221> that from ")
                             .arg(assetsymbol).arg(assetname).arg(stotalamount).arg(ssmallestunit).arg(owneraddress).arg(description).arg(sneededUMpSamount);
    if (ui->radioButton_ELexchange->isChecked())  {
        listingsummary += QString("exchanged with amount (%1) of UMpT <222>.").arg(swithUMpTamount);   
    } else  
        listingsummary += QString("your wallet.");        
    if (assetsymbol != "" && totalamount > 0 && smallestunit > 0 && owneraddress != ""){
        ui->textEdit_ELlistingsummary->setText(listingsummary);
        listingsummary = QString();                                                           
    } else  
        on_pushButton_ELcleanalllistinginputs_clicked();   
}

void OverviewPage::on_pushButton_ELcleanalllistinginputs_clicked(){          // clean all listing inputs
    ui->lineEdit_ELassetsymbol->clear();
    ui->lineEdit_ELassetname->clear();
    ui->lineEdit_ELtotalamount->clear();
    ui->comboBox_ELsamllestunit->setCurrentIndex(0);
    ui->lineEdit_ELowneraddress->clear();
    ui->lineEdit_ELdescription->clear();
    ui->lineEdit_ELwithUMpTamount->clear();
    ui->radioButton_ELexchange->setChecked(true);
    ui->textEdit_ELlistingsummary->clear();
    ui->label_ELamountofUMpSneeded->setText("Amount of UMpS Needed");
}

void OverviewPage::on_pushButton_ELputlistingorder_clicked()
{ // construct new listing new asset transaction and send them out.

}

            /////// Announcing page (Postimng) ///////
double postingfeerate = 0.01;                  // the rate is the UMpT number per 100 bytes of input.
double postingfee = 0.00;
int postingtextnumber = 0;
void OverviewPage::on_textEdit_EPannouncementwindow_textChanged()
{
    postingtextnumber = ui->textEdit_EPannouncementwindow->toPlainText().length();
    postingfee = postingtextnumber * postingfeerate;
    ui->label_EPannouncementfee->setText(QString::number(postingfee));
}

void OverviewPage::on_pushButton_EPreviewannouncingorder_clicked()
{
    QString postingsummary = QString();
    postingsummary = QString("   This posting has total (%1) charactor, space, and symbol.\n"
                             "   This posting costs total (%2) of UMpT - asset symbol 222\n\n"
                             "   Now the posting fee rate is (%3)").arg(postingtextnumber).arg(postingfee).arg(postingfeerate);
    if (postingtextnumber>0)  
        ui->textEdit_EPpostingsummary->setText(postingsummary);
}

void OverviewPage::on_pushButton_EPcleanannouncinginput_clicked()
{   // clean your announcement input
    ui->textEdit_EPannouncementwindow->clear();
    ui->label_EPannouncementfee->clear();
    ui->textEdit_EPpostingsummary->clear();
}

void OverviewPage::on_pushButton_EPpostannouncement_clicked()
{// construct the posting announcement transaction and send it out.

}

///////////////////////// Manager page ///////////////////////////////////////////
void OverviewPage::on_pushButton_Mbackupwallet_clicked()
{    // backup your wallet
    QString directorystr = ui->lineEdit_Mdirectory->text();
    QString result = "1";
//    result = backupwallet directorystr;                                                 // need RPC
    if (result == "")
        QMessageBox::warning(this,"","wallet is backuped.");
}

void OverviewPage::on_pushButton_Mencryptwallet_clicked()
{    // encrypt your wallet
    QString thepassphrasestr = ui->lineEdit_Msetpassphrase->text();
    QString result = "1";
//    result = encryptwallet thepassphrasestr;                                             // need RPC
    if (result == "")
        QMessageBox::warning(this,"","wallet encrypted; Bitcoin server stopping, restart to run with encrypted wallet. The keypool has been flushed, you need to make a new backup.");
}

void OverviewPage::on_pushButton_Mchangepassphrase_clicked()
{    // change your wallet passphrase
    QString oldpassphrasestr=ui->lineEdit_Moldpassphrase->text();
    QString newpassphrasestr=ui->lineEdit_Mnewpassphrase->text();
    QString result = "1";
//    result = walletpassphrasechange oldpassphrasestr newpassphrasestr;                   // need RPC
    if (result == "")
        QMessageBox::warning(this,"","pass phrase changed.");
}

void OverviewPage::on_pushButton_Msignmessage_clicked()
{    // open sign message window     in base64
    QString addresssignstr = ui->lineEdit_Maddresssign->text();
    QString messagesignstr=ui->textEdit_Mmessagesign->toPlainText();
    QString result = QString();
//    result = signmessage addresssignstr messagesignstr;                                  // need RPC
    if (result != "")
        ui->lineEdit_Msignaturesign->setText(result);
}

void OverviewPage::on_pushButton_Mverifymessage_clicked()
{    // open verify message window, like File->Verify message. ----signverifymessagedialog.ui
    QString addressverifystr = ui->lineEdit_Maddressverify->text();
    QString signatureverifystr = ui->lineEdit_Msignatureverify->text();
    QString messageverifystr = ui->textEdit_Mmessageverify->toPlainText();
    QString result = "false";
//    result = verifymessage addressverifystr signatureverifystr messageverifystr         // need RPC
    if (result == "true")
        QMessageBox::warning(this, "", "Message Verified.");
    else
        QMessageBox::warning(this, "", "Message Not Verified.");
}

void OverviewPage::on_checkBox_Munactivenetwork_stateChanged(int arg1)
{
    ;//if (arg1 == 0)
//        setnetworkactive true;                                                         // need RPC
    //else
//        setnetworkactive false;                                                        // need RPC
}

void OverviewPage::on_pushButton_Mnetworkinformation_clicked()
{                   // open network information window, like Help->Debug Window->Information and Network Traffic part
//    QString networkinginfomationstr = getnetworkinfo;                                   //RPC
//    QMessageBox::information(this, tr("Networking Information:\n"), tr("%1").arg(networkinginfomationstr));
}

void OverviewPage::on_pushButton_Mmanagepeers_clicked()
{                     // open mangae peers window
//    QString managepeersstr = getpeerinfo;                                                // need RPC
//    QMessageBox::information(this, tr("Peers Information:\n"), tr("%1").arg(managepeersstr));
}

void OverviewPage::on_pushButton_Mbannedlist_clicked()
{
//    QString bannedliststr = listbanned;                                                  // need RPC
//    QMessageBox::information(this, tr("Networking Information:\n"), tr("%1").arg(bannedliststr));
}

void OverviewPage::on_pushButton_Maddnode_clicked()
{
      QString addstr = ui->comboBox_Maddnode->currentText();
      QString nodestr = ui->lineEdit_Maddnode->text();
      if (addstr == "To Add Node:"){
          addstr = "add";

      }else if(addstr == "Remove Node:"){
          addstr = "remove";
      }else {
          addstr = "onetry";
      }
//      addnode nodestr addstr;                                                            // need RPC
}

void OverviewPage::on_pushButton_Mdisconnect_clicked()
{
       QString nodestr = ui->lineEdit_Mdisconnectnode->text();
//       disconnectnode nodestr;                                                          // need RPC
}

void OverviewPage::on_pushButton_Maddbanned_clicked()
{
    QString addstr = ui->comboBox_Msetbannedip->currentText();
    QString bannedstr = ui->lineEdit_Msetbannedrip->text();
    QString timestr = ui->lineEdit_Mbannedtime->text();
    if (addstr == "Add IP/Subnet to banned list:") 
        addstr = "add";
    else 
        addstr = "remove";
//    setban bannedstr addstr timestr;                                                     // need RPC
}

void OverviewPage::on_pushButton_Mclearallbanned_clicked()
{
//    clearbanned;                                                                         // need RPC
}

void OverviewPage::on_spinBox_MsetofDBcache_valueChanged(const QString &arg1){

}

void OverviewPage::on_comboBox_Minterfacelanguage_activated(const QString &arg1){

}
