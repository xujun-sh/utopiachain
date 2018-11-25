#include "qt/utopiacoingui.h"
#include "qt/ui_utopiacoingui.h"

const std::string UtopiacoinGUI::DEFAULT_UIPLATFORM =
#if defined(Q_OS_MAC)
        "macosx"
#elif defined(Q_OS_WIN)
        "windows"
#else
        "other"
#endif
        ;

/** Display name for default wallet name. Uses tilde to avoid name
 * collisions in the future with additional wallets */
const QString UtopiacoinGUI::DEFAULT_WALLET = "~Default";


UtopiacoinGUI::UtopiacoinGUI(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::UtopiacoinGUI)
{
    ui->setupUi(this);


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

        QRegExp assetsymbolreg("^[123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$");     // set base58
        ui->lineEdit_assetsymbol->setValidator(new QRegExpValidator(assetsymbolreg, this));     // Exchange - listing       set base58

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

UtopiacoinGUI::~UtopiacoinGUI()
{
    delete ui;
}

void UtopiacoinGUI::recipientitem1(int i)
{
    if (i >1 && i <11)
        ui->tableWidget_addmorerecipient->setRowCount(ui->tableWidget_addmorerecipient->rowCount()+3);

    QDoubleValidator *pDoubleValidator = new QDoubleValidator(this);
        pDoubleValidator->setRange(0, 1000000000);                                         // set double number
        pDoubleValidator->setNotation(QDoubleValidator::StandardNotation);
        pDoubleValidator->setDecimals(8);
    QRegExp addrreg("^[1][123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz]+$");

    QLabel *newrecipientaddress = new QLabel(QString ("Recipient Address%1:").arg(i));
    QLabel *newreceivingamount = new QLabel(QString("  Reciving Amount:"));
    QPushButton *newchooseaddress = new QPushButton(QString("Choose Address"));
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

void UtopiacoinGUI::on_pushButton_cleanallsetting_clicked()
{    // clean all setting
     ui->comboBox_asset->setCurrentIndex(0);
     ui->comboBox_time->setCurrentIndex(0);
     ui->comboBox_type->setCurrentIndex(0);
     ui->lineEdit_amount->clear();
     ui->lineEdit_search->clear();
     ui->textEdit_resultwindow->clear();
     ui->textEdit_searchsettingsummary->clear();
}

QString searchsettingsummary = "";
void UtopiacoinGUI::on_pushButton_reviewsearchsetting_clicked()
{
    QString bassetsymbol = ui->comboBox_asset->currentText();
    QString btime =        ui->comboBox_time->currentText();
    QString btype =        ui->comboBox_type->currentText();
    QString bamount =      ui->lineEdit_amount->text();
    QString bsearch =      ui->lineEdit_search->text();
    if (bsearch != ""){   // txid and block hash are both 32 bytes 64 characters     // address is 26-35 alphanumeric characters
        if (bsearch.length() == 64){
            // search as block hash first if the hash not prefix with 00, then search as txid
            // rpc: getblock bsearch 1;                   // json block
            // rpc: gettransaction bsearch;               // tx in wallet
            // rpc: getrawtransaction(bsearch 1);         // tx in blockchain
            searchsettingsummary += QString("Search for the block/transaction info with:\n block hash / transaction ID (%1).").arg(bsearch); }
        else if (bsearch.length() < 26){
            // search as block height
            // rpc:    getblock(getblockhash(bsearch) 1);  //json
                searchsettingsummary += QString("Search for the block info with:\n block height (%1).").arg(bsearch); }
             else if (bsearch.length() <36){
                  // search as address            // rpc: getreceivedbyaccount bsearch 6;  // after 6 confirm
                     searchsettingsummary += QString("Search for the address info with:\n address (%1).").arg(bsearch); }
                  else{ searchsettingsummary += QString("Wrong!\n input must be block height, or block hash, or txid, or address."); }
    }
    else if (btime  == "Opening" && (btype == "For Buy" || btype == "For Sell" || btype == "Buy & Sell")) {
        if (bamount == ""){ bamount = "all amount"; }
            searchsettingsummary += QString("Search all opening (%1) transaction info with:\n asset symbol (%2), asset amount no more than (%3).")
                                            .arg(btype).arg(bassetsymbol).arg(bamount);                                         }
         else if (btime != "Opening" && (btype != "For Buy" && btype != "For Sell" && btype != "Buy & Sell" && bamount == ""))
                 {// PRC: listtransactions (bassetsymbol)    // PRC: listsinceblock (getblockhash(block height))      // if btime != Opening, calculate btime to block height
                 // if btype != Total, filter with btype
                 searchsettingsummary += QString("Search for all transaction info with:\n asset symbol (%1), time limited to (%2), transaction type is (%3)")
                                                 .arg(bassetsymbol).arg(btime).arg(btype); }
              else {searchsettingsummary += QString("Wrong! \n only For Buy, For Sell, or Buy & Sell match with Opening that can has Amount."); }
    ui->textEdit_searchsettingsummary->setText(searchsettingsummary);
    searchsettingsummary = "";
}

void UtopiacoinGUI::on_pushButton_getsearchingresult_clicked()
{    // search required and show the result in the textEdit.
     // lineEdit_search input is exclusived with all other settings (Asset, Time, Type, and or Amount).
     // rpc: listsinceblock (getbestblockhash);                       // default result for browser
}

// show textEdit with :  search result of Block, Transaction, or Address. Default show the Opening Transaction of all asset and all amount.


///////////////////// Accounting page ////////////////////////////////////////////////////////
void UtopiacoinGUI::on_pushButton_receivingaddressbook_clicked()
{  // open Receiving Address Book}


}

void UtopiacoinGUI::on_pushButton_sendingaddressbook_clicked()
{  // open Sending Address Book

}


///////////////////// Exchange page //////////////////////////////////////////////////////////
void UtopiacoinGUI::on_tabWidget_tabBarClicked(int index)
{
    ui->tabWidget_exchange->setCurrentIndex(0);                                                  // when tabWidget_exchange clicked, always open the Sending page first.
    recipientitem1(1);
    ui->label_totalrecipientnumber->setText("1 Recipient");
}

              /////// Sending page ///////
int              totalrecipientnumber = 1;
QVector<QString> vector_recipientaddress(10);
QVector<QString> vector_recipientamount(10);
double           sendingamountsummary = 0.00000000;
QString          sendingsummary = "";
void UtopiacoinGUI::on_pushButton_addmorerecipient_clicked()
{    // once click will add one more recipient (one groupBox_singlerecipientinfo)
    totalrecipientnumber++;
    if (totalrecipientnumber > 10)                                                               // limit the recipient to 10
        return;
    recipientitem1(totalrecipientnumber);
    ui->label_totalrecipientnumber->setText((QString::number(totalrecipientnumber)) + " Recipients");
}

void UtopiacoinGUI::on_pushButton_cleanallsendinginput_clicked()
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

void UtopiacoinGUI::on_pushButton_chooseaddress_clicked()
{    // open the Sending Address Book to chose a address for sending asset
}
void UtopiacoinGUI::on_pushButton_putsendingorder_clicked()
{    // build the sending asset transaction and sent it out
    QString symbol = ui->comboBox_assetsymbol->currentText();
    int nullrecivientnum = 0;
    sendingsummary += QString("Send the asset symbol (%1) to:\n").arg(symbol);
    for (int i = 1; i<=ui->tableWidget_addmorerecipient->rowCount()/3; ++i){
        QWidget   *widgetaddress=ui->tableWidget_addmorerecipient->cellWidget((i-1)*3+1,1);    //获得widget
        QWidget   *widgetamount=ui->tableWidget_addmorerecipient->cellWidget((i-1)*3+2,1);
        QLineEdit *recipientaddress1=(QLineEdit*)widgetaddress;                                //强制转化为QLineEdit
        QLineEdit *recipientamount1=(QLineEdit*)widgetamount;
        QString   tempaddress = recipientaddress1->text();
        QString   tempamount = recipientamount1->text();
        vector_recipientaddress[i-1] = tempaddress;
        vector_recipientamount[i-1] = tempamount;
        if (tempaddress!="" && tempamount>0){
            sendingsummary += QString("   %2: address/amount  (%3 / %4).\n").arg(i).arg(tempaddress).arg(tempamount);
            sendingamountsummary += tempamount.toDouble();  }
        else { nullrecivientnum++;                          }               }
    QString a=QString::number(sendingamountsummary,'f',8);
    sendingsummary += QString("The total sending address is (%1), the total sending amount is (%2).").arg(ui->tableWidget_addmorerecipient->rowCount()/3-nullrecivientnum).arg(a);
    if (sendingamountsummary > 0) {
        ui->textEdit_sendingsummary->setText(sendingsummary);
        sendingamountsummary = 0.00000000;
        sendingsummary = "";
        nullrecivientnum = 0;      }
    else{ UtopiacoinGUI::on_pushButton_cleanallsendinginput_clicked(); }
}

              /////// Exchanging page ///////
void UtopiacoinGUI::on_pushButton_choosereceivingaddress_clicked()
{  // open Receiving Address Book to chose a receiving address

}
void UtopiacoinGUI::on_pushButton_cleanallexchanginginputs_clicked()
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
void UtopiacoinGUI::on_pushButton_putexchangingorder_clicked()
{  // construct exchanging transactions and send them.
    QString outsymbol = ui->comboBox_outassetsymbol->currentText();
    QString insymbol =  ui->comboBox_inassetsymbol->currentText();
    double outamount =  ui->lineEdit_outamount->text().toDouble();
    double inamount =   ui->lineEdit_inamount->text().toDouble();
    QString soutamount =QString::number(outamount,'f',8);
    QString sinamount = QString::number(inamount,'f',8);
    QString inaddress = ui->lineEdit_inaddress->text();
    int outtimelimit =  ui->lineEdit_outtimelimit->text().toInt();
    exchangingsummary = QString("Want to use:        amount (%1) of asset symbol (%2)"
                              "\nTo exchange:       amount (%3) of asset symbol (%4) "
                              "\nReceiving with:    address (%6) for the asset symbol (%5)\n")
                              .arg(soutamount).arg(outsymbol).arg(sinamount).arg(insymbol).arg(insymbol).arg(inaddress);
    if (outtimelimit>0){
        exchangingsummary += QString("The time limit:     for this exchange is %1 block (about 10 minute per block).\n").arg(outtimelimit); }
    if (ui->checkBox_inpartially->isChecked()){
        exchangingsummary += QString("The amount:        for this exchange is changable (<%3) at the same exchange rate of (%1 / %2)")
                                    .arg(soutamount).arg(sinamount).arg(sinamount);                                     }
    if(outamount > 0 && inamount > 0 && inaddress != "" )   {
        ui->textEdit_exchangingsummary->setText(exchangingsummary);
        exchangingsummary = "";                              }
    else { on_pushButton_cleanallexchanginginputs_clicked(); }

}

              /////// Listing page ///////
void UtopiacoinGUI::on_pushButton_createowneraddress_clicked()
{    // generate a grand new  address for the new listing asset owner

}
double totalamount = 0;
double smallestunit = 0.00000000;
double neededUMpSamount = 0.00000000;
QString sneededUMpSamount = "";
QString ssmallestunit = "";
void UtopiacoinGUI::on_comboBox_samllestunit_currentTextChanged(const QString &arg1){
    smallestunit = arg1.toDouble();
    neededUMpSamount = totalamount / smallestunit;
    sneededUMpSamount = QString::number(neededUMpSamount,'f',0);
    ui->label_amountofUMpSneeded->setText(sneededUMpSamount);                    }
void UtopiacoinGUI::on_lineEdit_totalamount_editingFinished()    {
    totalamount = ui->lineEdit_totalamount->text().toDouble();
    ssmallestunit = QString::number(smallestunit,'f',8);
    on_comboBox_samllestunit_currentTextChanged(ssmallestunit);
}
void UtopiacoinGUI::on_radioButton_yourwallet_pressed()          {
    ui->lineEdit_withUMpTamount->clear();                                // if your wallet button selected, clear With UMpT Amount
    ui->label_availablesendamount->setText("Available UMpT"); }
void UtopiacoinGUI::on_radioButton_exchange_pressed()  { // show the Available UMpT amount
                                                    }
void UtopiacoinGUI::on_pushButton_cleanalllistinginputs_clicked(){          // clean all listing inputs
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
void UtopiacoinGUI::on_pushButton_putlistingorder_clicked()
{    // construct new listing new asset transaction and send them out.
    QString assetsymbol =   ui->lineEdit_assetsymbol->text();
    QString assetname =     ui->lineEdit_assetname->text();
    QString owneraddress =  ui->lineEdit_owneraddress->text();
    QString description =   ui->lineEdit_description->text();
    double withUMpTamount = ui->lineEdit_withUMpTamount->text().toDouble();
    QString swithUMpTamount =   QString::number(withUMpTamount,'f',8);
    QString stotalamount =      QString::number(totalamount,'f',0);
    listingsummary = QString("The new listing exchangable asset:\n"
                             "    asset symbol: (%1);          asset name: (%2);       total amount: (%3);\n"
                             "    smallest unit:  (%4);  owner address: (%5);\n"
                             "    description:    (%6).\n"
                             "This listing needed UMpS (%7) by ")
                             .arg(assetsymbol).arg(assetname).arg(stotalamount).arg(ssmallestunit)
                             .arg(owneraddress).arg(description).arg(sneededUMpSamount);
    if (ui->radioButton_exchange->isChecked())  {
        listingsummary += QString("exchanging with amount (%1) of UMpT.").arg(swithUMpTamount);   }
    else {  listingsummary += QString("your wallet.");    }
    if (assetsymbol != "" && totalamount > 0 && smallestunit > 0 && owneraddress != ""){
        ui->textEdit_listingsummary->setText(listingsummary);
        listingsummary = "";                                                           }
    else { on_pushButton_cleanalllistinginputs_clicked(); }
}

            /////// Posting page ///////
double postingfeerate = 0.01;                                             // the rate is the UMpT number per 100 bytes of input.
double postingfee = 0.00;
int postingtextnumber = 0;
void UtopiacoinGUI::on_textEdit_announcementwindow_textChanged()
{
    postingtextnumber = ui->textEdit_announcementwindow->toPlainText().length();
    postingfee = postingtextnumber * postingfeerate;
    ui->label_announcementfee->setText(QString::number(postingfee));
}
void UtopiacoinGUI::on_pushButton_cleanannouncinginput_clicked()
{   // clean your announcement input
    ui->textEdit_announcementwindow->clear();
    ui->label_announcementfee->clear();
    ui->textEdit_postingsummary->clear();
}
QString postingsummary = "";
void UtopiacoinGUI::on_pushButton_postannouncement_clicked()
{   // construct the posting announcement transaction and send it out.
    postingsummary = QString("   This posting has total (%1) charactor, space, and symbol.\n"
                             "   This posting costs total (%2) of UMpT - asset symbol 222\n\n"
                             "   Now the posting fee rate is (%3)").arg(postingtextnumber).arg(postingfee).arg(postingfeerate);
    if (postingtextnumber>0)  {
        ui->textEdit_postingsummary->setText(postingsummary);
        postingsummary = "";  }
}


///////////////////////// Manager page ///////////////////////////////////////////
void UtopiacoinGUI::on_pushButton_leaveutopiamarketplace_clicked()
{   // close utopia market system and leave the marketplace
    close();

}

void UtopiacoinGUI::on_pushButton_backupwallet_clicked()
{    // backup your wallet, like former File->Backup Wallet.

}

void UtopiacoinGUI::on_pushButton_encryptwallet_clicked()
{    // encrypt your wallet, like Setting->Encrypt Wallet.

}

void UtopiacoinGUI::on_pushButton_changepassphrase_clicked()
{    // change your wallet passphrase, like Setting->Change Passphrase. ---- askpassphrasedialog.ui

}

void UtopiacoinGUI::on_pushButton_signmessage_clicked()
{    // open sign message window, like File->Sign message.  ---- signverifymessagedialog.ui

}

void UtopiacoinGUI::on_pushButton_verifymessage_clicked()
{   // open verify message window, like File->Verify message. ----signverifymessagedialog.ui

}

void UtopiacoinGUI::on_pushButton_networkinformation_clicked()
{   // open network information window, like Help->Debug Window->Information and Network Traffic part

}

void UtopiacoinGUI::on_pushButton_managepeers_clicked()
{   // open mangae peers window

}
