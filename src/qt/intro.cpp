#if defined(HAVE_CONFIG_H)
#include <config/utopiacoin-config.h>
#endif

#include <fs.h>
#include <qt/intro.h>
#include <qt/forms/ui_intro.h>

#include <qt/guiutil.h>

#include <util.h>

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include <cmath>
#include <qt/rpcconsole.h>

static const uint64_t GB_BYTES = 1000000000LL;
static const uint64_t BLOCK_CHAIN_SIZE = 150;
static const uint64_t CHAIN_STATE_SIZE = 3;
static uint64_t requiredSpace;

class FreespaceChecker : public QObject
{
    Q_OBJECT

public:
    explicit FreespaceChecker(Intro *intro);

    enum Status {
        ST_OK,
        ST_ERROR
    };

public Q_SLOTS:
    void check();

Q_SIGNALS:
    void reply(int status, const QString &message, quint64 available);

private:
    Intro *intro;
};

#include <qt/intro.moc>

FreespaceChecker::FreespaceChecker(Intro *_intro)
{
    this->intro = _intro;
}

void FreespaceChecker::check()
{
    QString dataDirStr = intro->getPathToCheck();
    fs::path dataDir = GUIUtil::qstringToBoostPath(dataDirStr);
    uint64_t freeBytesAvailable = 0;
    int replyStatus = ST_OK;
    QString replyMessage = tr("A new data directory will be created.");

    /* Find first parent that exists, so that fs::space does not fail */
    fs::path parentDir = dataDir;
    fs::path parentDirOld = fs::path();
    while(parentDir.has_parent_path() && !fs::exists(parentDir))
    {
        parentDir = parentDir.parent_path();

        /* Check if we make any progress, break if not to prevent an infinite loop here */
        if (parentDirOld == parentDir)
            break;

        parentDirOld = parentDir;
    }

    try {
        freeBytesAvailable = fs::space(parentDir).available;
        if(fs::exists(dataDir))
        {
            if(fs::is_directory(dataDir))
            {
                QString separator = "<code>" + QDir::toNativeSeparators("/") + tr("name") + "</code>";
                replyStatus = ST_OK;
                replyMessage = tr("Directory already exists. Add %1 if you intend to create a new directory here.").arg(separator);
            } else {
                replyStatus = ST_ERROR;
                replyMessage = tr("Path already exists, and is not a directory.");
            }
        }
    } catch (const fs::filesystem_error&)
    {
        /* Parent directory does not exist or is not accessible */
        replyStatus = ST_ERROR;
        replyMessage = tr("Cannot create data directory here.");
    }
    Q_EMIT reply(replyStatus, replyMessage, freeBytesAvailable);
}

Intro::Intro(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Intro),
    thread(0),
    signalled(false)
{
    ui->setupUi(this);

    ui->buttonBox->setVisible(false);
    ui->dataDirCustom->setVisible(false);
    ui->dataDirDefault->setVisible(false);
    ui->dataDirectory->setVisible(false);
    ui->ellipsisButton->setVisible(false);
    ui->lblExplanation1->setVisible(false);
    ui->lblExplanation3->setVisible(false);
    ui->lblExplanation2->setVisible(false);
    ui->storageLabel->setVisible(false);
    ui->welcomeLabel->setVisible(false);

    ui->welcomeLabel->setText(ui->welcomeLabel->text().arg(tr(PACKAGE_NAME)));
    ui->storageLabel->setText(ui->storageLabel->text().arg(tr(PACKAGE_NAME)));

    ui->lblExplanation1->setText(ui->lblExplanation1->text()
        .arg(tr(PACKAGE_NAME))
        .arg(BLOCK_CHAIN_SIZE)
        .arg(2009)
        .arg(tr("Utopiacoin"))
    );
    ui->lblExplanation2->setText(ui->lblExplanation2->text().arg(tr(PACKAGE_NAME)));

    uint64_t pruneTarget = std::max<int64_t>(0, gArgs.GetArg("-prune", 0));
    requiredSpace = BLOCK_CHAIN_SIZE;
    QString storageRequiresMsg = tr("At least %1 GB of data will be stored in this directory, and it will grow over time.");
    if (pruneTarget) {
        uint64_t prunedGBs = std::ceil(pruneTarget * 1024 * 1024.0 / GB_BYTES);
        if (prunedGBs <= requiredSpace) {
            requiredSpace = prunedGBs;
            storageRequiresMsg = tr("Approximately %1 GB of data will be stored in this directory.");
        }
        ui->lblExplanation3->setVisible(true);
    } else {
        ui->lblExplanation3->setVisible(false);
    }
    requiredSpace += CHAIN_STATE_SIZE;
    //ui->sizeWarningLabel->setText(
    //    tr("%1 will download and store a copy of the Utopiacoin block chain.").arg(tr(PACKAGE_NAME)) + " " +
    //    storageRequiresMsg.arg(requiredSpace) + " " +
    //    tr("The wallet will also be stored in this directory.")
    //);
    startThread();

    /////////////////////////////////////General setting///////////////////////////////////////
        ui->tabWidget->setCurrentIndex(0);                                                     // always open the Browser page first.

                  //////Validator for number, string////////
        QDoubleValidator *pDoubleValidator = new QDoubleValidator(this);
            pDoubleValidator->setRange(0, 1000000000);                                         // set double number
            pDoubleValidator->setNotation(QDoubleValidator::StandardNotation);
            pDoubleValidator->setDecimals(8);
        ui->lineEdit_amount->setValidator(pDoubleValidator);                                   // Browser                   set double
        ui->lineEdit_inamount->setValidator(pDoubleValidator);                                 // Exchange - exchanging     set double
        ui->lineEdit_outamount->setValidator(pDoubleValidator);                                // Exchange - exchanging     set double
        ui->lineEdit_withUMpTamount->setValidator(pDoubleValidator);                           // Exchange - listing        set double

        ui->lineEdit_totalamount->setValidator(new QIntValidator(1, 2100000000));              // Exchange - listing        set integer
        ui->lineEdit_outtimelimit->setValidator(new QIntValidator(1, 1440));                   // Exchange - exchanging     set integer

        QRegExp addrreg("^[1][123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$");  // prefix with 1, set base58
        ui->lineEdit_inaddress->setValidator(new QRegExpValidator(addrreg, this));              // Exchange - exchanging
        ui->lineEdit_owneraddress->setValidator(new QRegExpValidator(addrreg, this));           // Exchange - listing

        QRegExp assetsymbolreg("^[123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$"); // set base58
        ui->lineEdit_assetsymbol->setValidator(new QRegExpValidator(assetsymbolreg, this));     // Exchange - listing       set base58

        QRegExp searchreg("^[0123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$");  // base58 and 0 for search input
        ui->lineEdit_search->setValidator(new QRegExpValidator(searchreg, this));               // Browser - search

                  ////////////treewidget///////////
        ui->treeWidget->header()->setSortIndicator(0,Qt::AscendingOrder);                       // sort the asset from 0 up

    ///////////////////////////////////Browser/////////////////////////////////////////////////
         // Here use RPC get and post on result window of the default result: All asset, Opening, Sell & Buy, All amount

    ////////////////////Accounting & all comboBox of asset symbol//////////////////////////////
        ui->tableWidget_accountwindow->setEditTriggers(QAbstractItemView::NoEditTriggers);       // set edit not allowed
        ui->tableWidget_accountwindow->resizeColumnsToContents();                                // set the horizental size to fit contents
        ui->tableWidget_accountwindow->horizontalHeader()->setDefaultSectionSize(190);           // set every column width
        ui->tableWidget_accountwindow->verticalHeader()->setDefaultSectionSize(25);

        for (int i=0; i< ui->treeWidget->topLevelItemCount(); ++i){
             QTableWidgetItem *itemSymbol, *itemAvailable, *itemPending, *itemTotal;

             // get Asset Symbol from Asset List Tree.  Done
             itemSymbol = new QTableWidgetItem;
             QString itemSymboltxt= ui->treeWidget->topLevelItem(i)->text(1);                    // toplevel i's col 1
             itemSymbol->setText(itemSymboltxt);
             ui->tableWidget_accountwindow->setItem(i, 0, itemSymbol);                           // Account's Symbol col

             ui->comboBox_asset->setItemText(i+1, itemSymboltxt);                                // Browser
             ui->comboBox_assetsymbol->setItemText(i, itemSymboltxt);                            // Exchange - Sending
             ui->comboBox_inassetsymbol->setItemText(i, itemSymboltxt);                          // Exchange - Exchanging - in
             ui->comboBox_outassetsymbol->setItemText(i, itemSymboltxt);                         // Exchange - Exchanging - out

             // get available amount for each asset
             itemAvailable = new QTableWidgetItem;
             QString childitemSymboltxt1= "RPC";                                                 // get the value with RPC
             itemAvailable->setText(childitemSymboltxt1);
             ui->tableWidget_accountwindow->setItem(i, 1, itemAvailable);                        // Account's Available col

             // get pending amount for each asset
             itemPending = new QTableWidgetItem;
             QString itemPendingtxt = "1234567890.12345678";                                     // get the value with RPC
             itemPending->setText(itemPendingtxt);
             ui->tableWidget_accountwindow->setItem(i, 2, itemPending);                          // Account's Pending col

             // get total amount for each asset
             itemTotal = new QTableWidgetItem;
             QString itemTotaltxt = "1234567890.12345678";                                       // get the value with RPC
             itemTotal->setText(itemTotaltxt);
             ui->tableWidget_accountwindow->setItem(i, 3, itemTotal);                            // Account's Total col

        }

    ////////////////////////////////////////Exchange////////////////////////////////////////////



    ////////////////////////////////////////Manager/////////////////////////////////////////////


}

Intro::~Intro()
{
    delete ui;
}



void Intro::recipientitem1(int i)
{
    if (i >1 && i <11)
        ui->tableWidget_addmorerecipient->setRowCount(ui->tableWidget_addmorerecipient->rowCount()+3);

    QDoubleValidator *pDoubleValidator = new QDoubleValidator(this);
        pDoubleValidator->setRange(0, 1000000000);                                         // set double number
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

    ui->tableWidget_addmorerecipient->setColumnWidth(0,145);
    ui->tableWidget_addmorerecipient->setRowHeight((i-1)*3,10);
    ui->tableWidget_addmorerecipient->setColumnWidth(1,440);
    ui->tableWidget_addmorerecipient->setRowHeight((i-1)*3+1,22);
    ui->tableWidget_addmorerecipient->setColumnWidth(2,145);
    ui->tableWidget_addmorerecipient->setRowHeight((i-1)*3+2,22);

    ui->tableWidget_addmorerecipient->setCellWidget((i-1)*3,0,line0);
    ui->tableWidget_addmorerecipient->setCellWidget((i-1)*3,1,line1);
    ui->tableWidget_addmorerecipient->setCellWidget((i-1)*3,2,line2);

    ui->tableWidget_addmorerecipient->setCellWidget((i-1)*3+1,0,newrecipientaddress);
    ui->tableWidget_addmorerecipient->setCellWidget((i-1)*3+1,1,lineEdit_newrecipientaddress);
    ui->tableWidget_addmorerecipient->setCellWidget((i-1)*3+1,2,newchooseaddress);

    ui->tableWidget_addmorerecipient->setCellWidget((i-1)*3+2,0,newreceivingamount);
    ui->tableWidget_addmorerecipient->setCellWidget((i-1)*3+2,1,lineEdit_newreceivingamount);
}

///////////////////// Browser page //////////////////////////////////////////////////////////

void Intro::on_pushButton_cleanallsetting_clicked()
{    // clean all setting
     ui->comboBox_asset->setCurrentIndex(0);
     ui->comboBox_time->setCurrentIndex(0);
     ui->comboBox_type->setCurrentIndex(0);
     ui->lineEdit_amount->clear();
     ui->lineEdit_search->clear();
     ui->textEdit_resultwindow->clear();
     ui->textEdit_searchsettingsummary->clear();
}

QString b_assetsymbol =   "All";
QString b_time =          "Opening";
QString b_type =          "Buy & Sell";
QString b_amount =        "All amount";
QString b_search =        "";
QString searchsettingsummary = "                                             <<Search Setting Summary>> \n\n";
void Intro::on_pushButton_reviewsearchsetting_clicked()
{
    b_assetsymbol = ui->comboBox_asset->currentText();
    b_time =        ui->comboBox_time->currentText();
    b_type =        ui->comboBox_type->currentText();
    b_amount =      ui->lineEdit_amount->text();
    b_search =      ui->lineEdit_search->text();
    if (b_search != ""){   // txid and block hash are both 32 bytes 64 characters     // address is 26-35 alphanumeric characters
        ui->comboBox_asset->setCurrentIndex(0);
        ui->comboBox_time->setCurrentIndex(0);
        ui->comboBox_type->setCurrentIndex(0);
        b_amount = "";
        if (b_search.length() == 64){
             if (b_search.left(2) == "00"){searchsettingsummary+= QString("Search for the block info with:\n    block hash (%1).").arg(b_search);}
             searchsettingsummary += QString("Search for the block/transaction that with:\n    block hash / transaction ID (%1).").arg(b_search);}
        else if (b_search.length() < 9 ){
             searchsettingsummary += QString("Search for the block that with:\n    block height (%1).").arg(b_search); }
        else if (b_search.length() > 25 && b_search.length() < 36){
             searchsettingsummary += QString("Search for the address that with:\n    address (%1).").arg(b_search); }
        else{searchsettingsummary += QString("Wrong!\n input must be block height, or block hash, or txid, or address."); }
    }
    else if (b_time  == "Opening" && ui->comboBox_type->currentIndex() < 3) {
        if (b_amount == ""){ b_amount = "All amount"; }
        searchsettingsummary += QString(" Search for Asset symbol <%1>; \n All (%2) type opening transactions; \n Asset amount no more than (%3).")
                                         .arg(b_assetsymbol).arg(b_type).arg(b_amount);                                         }
    else if (b_time != "Opening" && ui->comboBox_type->currentIndex() > 3 && b_amount == "") {
        searchsettingsummary += QString(" Search for asset symbol <%1>; \n All (%2) type transactions; \n Time from (%3) to now.")
                                        .arg(b_assetsymbol).arg(b_type).arg(b_time); }
    else {searchsettingsummary += QString("Wrong! \n only For Buy, For Sell, and Buy & Sell 3 kinds of type can match with Opening and can has Amount."); }
    ui->textEdit_searchsettingsummary->setText(searchsettingsummary);
    searchsettingsummary =  "                                             <<Search Setting Summary>> \n\n";
}

void Intro::on_pushButton_getsearchingresult_clicked()
{    // search required and show the result in the textEdit.
     // lineEdit_search input is exclusived with all other settings (Asset, Time, Type, and or Amount).
    b_assetsymbol = ui->comboBox_asset->currentText();
    b_time =        ui->comboBox_time->currentText();
    b_type =        ui->comboBox_type->currentText();
    b_amount =      ui->lineEdit_amount->text();
    b_search =      ui->lineEdit_search->text();
    QString b_searchresult = "";
    if (b_search != ""){   // for search input only
        if (b_search.length() == 64){
             if (b_search.left(2) == "00"){
//               bsearchresult = getblock(b_search, 1);                                                //RPC
                 return;}
             else {
//               b_searchresult = getblock(getblockhash(b_search), 1);                                 //RPC
                 if (b_searchresult.isNull()){
//                   b_searchresult = getrawtransaction(b_search, 1);                                  //RPC
                  }
             }
        }
    }
    else if (b_time == "Opening" && ui->comboBox_type->currentIndex() < 3) {

    }
    else if (b_time != "Opening" && ui->comboBox_type->currentIndex() > 3 && b_amount == "") {

    }
	std::string result;
        std::string executableCommand = "listsinceblock (getbestblockhash) \n";

	if(!RPCConsole::RPCExecuteCommandLine(result, executableCommand))
        {
            result = "Parse error: unbalanced ' or \"";
        }

        //Q_EMIT reply(RPCConsole::CMD_REPLY, QString::fromStdString(result));
	ui->textEdit_resultwindow->setText(QString::fromStdString(result));
    //ui->textEdit_resultwindow->setText(b_search);
     // rpc: listsinceblock (getbestblockhash);                       // default result for browser
}

// show textEdit with :  search result of Block, Transaction, or Address. Default show the Opening Transaction of all asset and all amount.


///////////////////// Accounting page ////////////////////////////////////////////////////////
void Intro::on_pushButton_receivingaddressbook_clicked()
{  // open Receiving Address Book

}

void Intro::on_pushButton_sendingaddressbook_clicked()
{  // open Sending Address Book

}


///////////////////// Exchange page //////////////////////////////////////////////////////////
void Intro::on_tabWidget_tabBarClicked(int index)
{
    ui->tabWidget_exchange->setCurrentIndex(0);                                              // when tabWidget_exchange clicked, always open the Sending page first.
    recipientitem1(1);
    ui->label_totalrecipientnumber->setText("1 Recipient");

            ///////////////// Manager///////////////
//       QString versionstr = getnetworkinfo()[version];                                     // RPC
//       ui->lineEdit_version->setText(versionstr);
//       QString therunningtimestr = uptime;                                                 // RPC
//       ui->lineEdit_therunningtime->setText(therunningtimestr);
//       QString currentnumberofblockstr = getblockchaininfo()[blocks];                      // RPC
//       ui->lineEdit_currentnumberofblock->setText(currentnumberofblockstr);
//       QString lastblocktimestr = getblockchaininfo()[mediantime];                         // RPC
//       ui->lineEdit_lastblocktime->setText(lastblocktimestr);
//       QString currentnumberoftransactionstr = getmempoolinfo()[size];                     // RPC
//       ui->lineEdit_currentnumberoftransaction->setText(currentnumberoftransactionstr);
//       QString memoryusagestr = getmempoolinfo()[usage];                                   // RPC
//       ui->lineEdit_memoryusage->setText(memoryusagestr);
}

              /////// Sending page ///////
int              totalrecipientnumber = 1;
QVector<QString> vector_recipientaddress(10);
QVector<QString> vector_recipientamount(10);
double           sendingamountsummary = 0.00000000;
QString          sendingsummary = "";
void Intro::on_pushButton_addmorerecipient_clicked()
{    // once click will add one more recipient (one groupBox_singlerecipientinfo)
    totalrecipientnumber++;
    if (totalrecipientnumber > 10)                                                               // limit the recipient to 10
        return;
    recipientitem1(totalrecipientnumber);
    ui->label_totalrecipientnumber->setText((QString::number(totalrecipientnumber)) + " Recipients");
}

void Intro::on_pushButton_cleanallsendinginput_clicked()
{    // clean all input data.
    ui->comboBox_assetsymbol->setCurrentIndex(0);
    ui->tableWidget_addmorerecipient->clearContents();
    ui->tableWidget_addmorerecipient->setRowCount(3);
    totalrecipientnumber = 1;
    recipientitem1(totalrecipientnumber);
    ui->label_totalrecipientnumber->setText("1 Recipient");
    vector_recipientaddress.fill("", -1);
    vector_recipientamount.fill("", -1);
    sendingamountsummary = 0.00000000;
    ui->textEdit_sendingsummary->clear();
}

void Intro::on_pushButton_chooseaddress_clicked()
{    // open the Sending Address Book to chose a address for sending asset
}

void Intro::on_pushButton_reviewsendingorder_clicked()
{
    QString symbol = ui->comboBox_assetsymbol->currentText();
    int nullrecivientnum = 0;
    sendingsummary += QString("Send the asset symbol <%1> to:\n").arg(symbol);
    for (int i = 1; i<=ui->tableWidget_addmorerecipient->rowCount()/3; ++i){
        QWidget   *widgetaddress=ui->tableWidget_addmorerecipient->cellWidget((i-1)*3+1,1);    //??widget
        QWidget   *widgetamount=ui->tableWidget_addmorerecipient->cellWidget((i-1)*3+2,1);
        QLineEdit *recipientaddress1=(QLineEdit*)widgetaddress;                                //?????QLineEdit
        QLineEdit *recipientamount1=(QLineEdit*)widgetamount;
        QString   tempaddress = recipientaddress1->text();
        QString   tempamount = recipientamount1->text();
        vector_recipientaddress[i-1] = tempaddress;
        vector_recipientamount[i-1] = tempamount;
        if (tempaddress!="" && tempamount>0){
            sendingsummary += QString("   %2: address/amount  (%3 / %4).\n").arg(i).arg(tempaddress).arg(tempamount);
            sendingamountsummary += tempamount.toDouble();  }
        else { nullrecivientnum++;                          }               }
    QString a = QString::number(sendingamountsummary,'f',8);
    sendingsummary += QString("The total sending address is (%1), the total sending amount is (%2).").arg(ui->tableWidget_addmorerecipient->rowCount()/3-nullrecivientnum).arg(a);
    if (sendingamountsummary > 0) {
        ui->textEdit_sendingsummary->setText(sendingsummary);
        sendingamountsummary = 0.00000000;
        sendingsummary = "";
        nullrecivientnum = 0;      }
    else{ Intro::on_pushButton_cleanallsendinginput_clicked(); }
}

void Intro::on_pushButton_putsendingorder_clicked()
{// build the sending asset transaction and sent it out
}

              /////// Exchanging page ///////
void Intro::on_pushButton_choosereceivingaddress_clicked()
{  // open Receiving Address Book to chose a receiving address

}


void Intro::on_pushButton_cleanallexchanginginputs_clicked()
{   // clean all exchanging inputs
    ui->comboBox_outassetsymbol->setCurrentIndex(0);
    ui->lineEdit_outtimelimit->clear();
    ui->lineEdit_outamount->clear();
    ui->comboBox_inassetsymbol->setCurrentIndex(0);
    ui->lineEdit_inamount->clear();
    ui->checkBox_inpartially->setChecked(false);
    ui->lineEdit_inaddress->clear();
    ui->textEdit_exchangingsummary->clear();
}

QString exchangingsummary = "";
void Intro::on_pushButton_reviewexchangingorder_clicked()
{  // construct exchanging transactions and send them.
    QString outsymbol = ui->comboBox_outassetsymbol->currentText();
    QString insymbol =  ui->comboBox_inassetsymbol->currentText();
    double outamount =  ui->lineEdit_outamount->text().toDouble();
    double inamount =   ui->lineEdit_inamount->text().toDouble();
    QString soutamount =QString::number(outamount,'f',8);
    QString sinamount = QString::number(inamount,'f',8);
    QString inaddress = ui->lineEdit_inaddress->text();
    int outtimelimit =  ui->lineEdit_outtimelimit->text().toInt();
    exchangingsummary = QString("Exchange out: amount (%1) of asset symbol <%2>"
                              "\n                                          <--->   "
                              "\nExchange in:  amount (%3) of asset symbol <%4> and save to address (%5)\n")
                              .arg(soutamount).arg(outsymbol).arg(sinamount).arg(insymbol).arg(inaddress);
    if (outtimelimit>0){
        exchangingsummary += QString("\nThe time limit: for this exchange is %1 block (about 10 minute per block).\n").arg(outtimelimit); }
    if (ui->checkBox_inpartially->isChecked()){
        exchangingsummary += QString("The exchange amount is flexable (<=%1) at the same exchange rate of (%2 <%3> / %4 <%5>)")
                                    .arg(sinamount).arg(sinamount).arg(insymbol).arg(soutamount).arg(outsymbol);     }
    if(outamount > 0 && inamount > 0 && inaddress != "" )   {
        ui->textEdit_exchangingsummary->setText(exchangingsummary);
        exchangingsummary = "";                              }
    else { on_pushButton_cleanallexchanginginputs_clicked(); }

}

void Intro::on_pushButton_putexchangingorder_clicked()
{// construct exchanging transactions and send them.

}
              /////// Listing page ///////
void Intro::on_pushButton_createowneraddress_clicked()
{    // generate a grand new  address for the new listing asset owner

}
double totalamount = 0;
double smallestunit = 0.00000000;
double neededUMpSamount = 0.00000000;
QString sneededUMpSamount = "";
QString ssmallestunit = "";
void Intro::on_comboBox_samllestunit_currentTextChanged(const QString &arg1){
    smallestunit = arg1.toDouble();
    neededUMpSamount = totalamount / smallestunit;
    sneededUMpSamount = QString::number(neededUMpSamount,'f',0);
    ui->label_amountofUMpSneeded->setText(sneededUMpSamount);                    }
void Intro::on_lineEdit_totalamount_editingFinished()    {
    totalamount = ui->lineEdit_totalamount->text().toDouble();
    ssmallestunit = QString::number(smallestunit,'f',8);
    on_comboBox_samllestunit_currentTextChanged(ssmallestunit);
}
void Intro::on_radioButton_yourwallet_pressed()          {
    ui->lineEdit_withUMpTamount->clear();                                // if your wallet button selected, clear With UMpT Amount
    ui->label_availablesendamount->setText("Available UMpT"); }
void Intro::on_radioButton_exchange_pressed()  { // show the Available UMpT amount
                                                    }
void Intro::on_pushButton_cleanalllistinginputs_clicked(){          // clean all listing inputs
    ui->lineEdit_assetsymbol->clear();
    ui->lineEdit_assetname->clear();
    ui->lineEdit_totalamount->clear();
    ui->comboBox_samllestunit->setCurrentIndex(0);
    ui->lineEdit_owneraddress->clear();
    ui->lineEdit_description->clear();
    ui->lineEdit_withUMpTamount->clear();
    ui->radioButton_exchange->setChecked(true);
    ui->textEdit_listingsummary->clear();                      }


QString listingsummary = "";
void Intro::on_pushButton_reviewlistingorder_clicked()
{
    QString assetsymbol =   ui->lineEdit_assetsymbol->text();
    QString assetname =     ui->lineEdit_assetname->text();
    QString owneraddress =  ui->lineEdit_owneraddress->text();
    QString description =   ui->lineEdit_description->text();
    double withUMpTamount = ui->lineEdit_withUMpTamount->text().toDouble();
    QString swithUMpTamount =   QString::number(withUMpTamount,'f',8);
    QString stotalamount =      QString::number(totalamount,'f',0);
    listingsummary = QString("Listing new exchangable asset:\n"
                             "   asset symbol: <%1>;   asset name: (%2);   total amount: (%3);   smallest unit:  (%4);  \n"
                             "   owner address: (%5);\n   description: (%6).\nThis listing needed amount (%7) of UMpS <221> that from ")
                             .arg(assetsymbol).arg(assetname).arg(stotalamount).arg(ssmallestunit).arg(owneraddress).arg(description).arg(sneededUMpSamount);
    if (ui->radioButton_exchange->isChecked())  {
        listingsummary += QString("exchanged with amount (%1) of UMpT <222>.").arg(swithUMpTamount);   }
    else {  listingsummary += QString("your wallet.");    }
    if (assetsymbol != "" && totalamount > 0 && smallestunit > 0 && owneraddress != ""){
        ui->textEdit_listingsummary->setText(listingsummary);
        listingsummary = "";                                                           }
    else { on_pushButton_cleanalllistinginputs_clicked(); }
}

void Intro::on_pushButton_putlistingorder_clicked()
{ // construct new listing new asset transaction and send them out.

}

            /////// Posting page ///////
double postingfeerate = 0.01;                                             // the rate is the UMpT number per 100 bytes of input.
double postingfee = 0.00;
int postingtextnumber = 0;
void Intro::on_textEdit_announcementwindow_textChanged()
{
    postingtextnumber = ui->textEdit_announcementwindow->toPlainText().length();
    postingfee = postingtextnumber * postingfeerate;
    ui->label_announcementfee->setText(QString::number(postingfee));
}
void Intro::on_pushButton_cleanannouncinginput_clicked()
{   // clean your announcement input
    ui->textEdit_announcementwindow->clear();
    ui->label_announcementfee->clear();
    ui->textEdit_postingsummary->clear();
}
QString postingsummary = "";
void Intro::on_pushButton_reviewannouncingorder_clicked()
{
    postingsummary = QString("   This posting has total (%1) charactor, space, and symbol.\n"
                             "   This posting costs total (%2) of UMpT - asset symbol 222\n\n"
                             "   Now the posting fee rate is (%3)").arg(postingtextnumber).arg(postingfee).arg(postingfeerate);
    if (postingtextnumber>0)  {
        ui->textEdit_postingsummary->setText(postingsummary);
        postingsummary = "";  }
}


void Intro::on_pushButton_postannouncement_clicked()
{// construct the posting announcement transaction and send it out.

}

///////////////////////// Manager page ///////////////////////////////////////////
void Intro::on_pushButton_backupwallet_clicked()
{    // backup your wallet
    QString directorystr = ui->lineEdit_thefiledirectory->text();
    QString result = "1";
//    result = backupwallet directorystr;                                                 //RPC
    if (result == "")
    QMessageBox::warning(this,"","wallet is backuped.");
}
void Intro::on_pushButton_encryptwallet_clicked()
{    // encrypt your wallet
    QString thepassphrasestr = ui->lineEdit_thepassphrase->text();
    QString result = "1";
//    result = encryptwallet thepassphrasestr;                                             //RPC
    if (result == "")
        QMessageBox::warning(this,"","wallet encrypted; Bitcoin server stopping, restart to run with encrypted wallet. The keypool has been flushed, you need to make a new backup.");
}
void Intro::on_pushButton_changepassphrase_clicked()
{    // change your wallet passphrase
    QString oldpassphrasestr=ui->lineEdit_oldpassphrase->text();
    QString newpassphrasestr=ui->lineEdit_newpassphrase->text();
    QString result = "1";
//    result = walletpassphrasechange oldpassphrasestr newpassphrasestr;                   //RPC
    if (result == "")
        QMessageBox::warning(this,"","pass phrase changed.");
}

void Intro::on_pushButton_signmessage_clicked()
{    // open sign message window     in base64
    QString addresssignstr = ui->lineEdit_addresssign->text();
    QString messagesignstr=ui->textEdit_messagesign->toPlainText();
    QString result = "";
//    result = signmessage addresssignstr messagesignstr;                                  //RPC
    if (result != "")
        ui->lineEdit_signaturesign->setText(result);
}
void Intro::on_pushButton_verifymessage_clicked()
{    // open verify message window, like File->Verify message. ----signverifymessagedialog.ui
    QString addressverifystr = ui->lineEdit_addressverify->text();
    QString signatureverifystr = ui->lineEdit_signatureverify->text();
    QString messageverifystr = ui->textEdit_messageverify->toPlainText();
    QString result = "false";
//    result = verifymessage addressverifystr signatureverifystr messageverifystr         //RPC
    if (result == "true")
        QMessageBox::warning(this, "", "Message Verified.");
    else
        QMessageBox::warning(this, "", "Message Not Verified.");
}
void Intro::on_checkBox_unactivenetwork_stateChanged(int arg1)
{
    if (arg1 == 0){
//        setnetworkactive true;                                                         // RPC
    }else{
//        setnetworkactive false;                                                        //RPC
    }
}
void Intro::on_pushButton_networkinformation_clicked()
{                   // open network information window, like Help->Debug Window->Information and Network Traffic part
//    QString networkinginfomationstr = getnetworkinfo;                                   //RPC
//    QMessageBox::information(this, tr("Networking Information:\n"), tr("%1").arg(networkinginfomationstr));
}
void Intro::on_pushButton_managepeers_clicked()
{                     // open mangae peers window
//    QString managepeersstr = getpeerinfo;                                                //RPC
//    QMessageBox::information(this, tr("Peers Information:\n"), tr("%1").arg(managepeersstr));
}
void Intro::on_pushButton_bannedlist_clicked()
{
//    QString bannedliststr = listbanned;                                                  //RPC
//    QMessageBox::information(this, tr("Networking Information:\n"), tr("%1").arg(bannedliststr));
}
void Intro::on_pushButton_addnode_clicked()
{
      QString addstr = ui->comboBox_addnode->currentText();
      QString nodestr = ui->lineEdit_addnode->text();
      if (addstr == "To Add Node:"){
          addstr = "add";

      }else if(addstr == "Remove Node:"){
          addstr = "remove";
      }else {
          addstr = "onetry";
      }
//      addnode nodestr addstr;                                                            //RPC
}
void Intro::on_pushButton_disconnect_clicked()
{
       QString nodestr = ui->lineEdit_disconnectnode->text();
//       disconnectnode nodestr;                                                          //RPC
}
void Intro::on_pushButton_addbanned_clicked()
{
    QString addstr = ui->comboBox_setbannedip->currentText();
    QString bannedstr = ui->lineEdit_setbannedrip->text();
    QString timestr = ui->lineEdit_bannedtime->text();
    if (addstr == "Add IP/Subnet to banned list:") {
        addstr = "add";
    }else {
        addstr = "remove";
    }
//    setban bannedstr addstr timestr;                                                     //RPC
}
void Intro::on_pushButton_clearallbanned_clicked()
{
//    clearbanned;                                                                         //RPC
}

void Intro::startThread()
{
    thread = new QThread(this);
    FreespaceChecker *executor = new FreespaceChecker(this);
    executor->moveToThread(thread);

    connect(executor, SIGNAL(reply(int,QString,quint64)), this, SLOT(setStatus(int,QString,quint64)));
    connect(this, SIGNAL(requestCheck()), executor, SLOT(check()));
    /*  make sure executor object is deleted in its own thread */
    connect(this, SIGNAL(stopThread()), executor, SLOT(deleteLater()));
    connect(this, SIGNAL(stopThread()), thread, SLOT(quit()));

    thread->start();
}

void Intro::checkPath(const QString &dataDir)
{
    mutex.lock();
    pathToCheck = dataDir;
    if(!signalled)
    {
        signalled = true;
        Q_EMIT requestCheck();
    }
    mutex.unlock();
}

QString Intro::getPathToCheck()
{
    QString retval;
    mutex.lock();
    retval = pathToCheck;
    signalled = false; /* new request can be queued now */
    mutex.unlock();
    return retval;
}

void Intro::setStatus(int status, const QString &message, quint64 bytesAvailable)
{
   ;
}

void Intro::on_dataDirectory_textChanged(const QString &dataDirStr)
{
    /* Disable OK button until check result comes in */
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    checkPath(dataDirStr);
}

void Intro::on_ellipsisButton_clicked()
{
    QString dir = QDir::toNativeSeparators(QFileDialog::getExistingDirectory(0, "Choose data directory", ui->dataDirectory->text()));
    if(!dir.isEmpty())
        ui->dataDirectory->setText(dir);
}

void Intro::on_dataDirDefault_clicked()
{
    setDataDirectory(getDefaultDataDirectory());
}

void Intro::on_dataDirCustom_clicked()
{
    ui->dataDirectory->setEnabled(true);
    ui->ellipsisButton->setEnabled(true);
}

void Intro::setDataDirectory(const QString &dataDir)
{
    ui->dataDirectory->setText(dataDir);
    if(dataDir == getDefaultDataDirectory())
    {
        ui->dataDirDefault->setChecked(true);
        ui->dataDirectory->setEnabled(false);
        ui->ellipsisButton->setEnabled(false);
    } else {
        ui->dataDirCustom->setChecked(true);
        ui->dataDirectory->setEnabled(true);
        ui->ellipsisButton->setEnabled(true);
    }
}

QString Intro::getDefaultDataDirectory()
{
    return GUIUtil::boostPathToQString(GetDefaultDataDir());
}

///////////////////////// Manager page ///////////////////////////////////////////
void Intro::on_pushButton_leaveutopiamarketplace_clicked()
{   // close utopia market system and leave the marketplace
    close();

}

