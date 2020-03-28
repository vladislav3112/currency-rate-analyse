#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QUrl>
#include <QFile>
#include <QDate>
#include <QXmlStreamReader>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
    ui->comboBox->addItem("доллар");
    ui->comboBox->addItem("евро");
    currency_code.insert("R01235","dollar");
    currency_code.insert("R01239","euro");
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_Button_confirm_clicked()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString code;
    QString date1 = ui->dateEdit->date().toString("dd/MM/yyyy");
    QString date2 = ui->dateEdit_2->date().toString("dd/MM/yyyy");
    if(ui->comboBox->currentText()=="доллар")code = currency_code.key("dollar");
    if(ui->comboBox->currentText()=="евро")code = currency_code.key("euro");
    manager->get(QNetworkRequest(QUrl("http://www.cbr.ru/scripts/XML_daily.asp?date_req="+date1)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    QString answer;
    QFile file ("valute_rate.xml");
    //file.reset();
    file.open(QIODevice::WriteOnly);


        switch (reply->error())
        {
        case 0:
            answer = QString::fromLatin1(reply->readAll());
            ui->textEdit->setText(answer);
            file.write(ui->textEdit->toPlainText().toLatin1());
            break;
        default:
            ui->textEdit->setText(this->trUtf8("Ошибка загрузки"));
            break;
        }

    file.close();
    reply->deleteLater();
    xml_parse();
}
void MainWindow::xml_parse(){
    //xml parse
    QVector<double> currency_rate;
    QString code;
    if(ui->comboBox->currentText()=="доллар") code = currency_code.key("dollar");
    if(ui->comboBox->currentText()=="евро") code = currency_code.key("euro");
    QString src_rate;

    QFile* file = new QFile("valute_rate.xml");
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<<"Невозможно открыть XML-конфиг";
        }
    QXmlStreamReader xml(file);

    while (!xml.atEnd()) {

            if(xml.name()=="Valute"){
                QXmlStreamAttributes attributes = xml.attributes();
             // do processing
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Valute"))
                    {
                        if (xml.tokenType() == QXmlStreamReader::StartElement &&
                               (code == attributes.value("ID").toString()))
                        {
                            if (xml.name() == "Value"){
                                xml.readNext();
                                src_rate = xml.text().toString();
                                src_rate.replace(",",".");       //src data correction
                                if(currency_rate.isEmpty())currency_rate.push_back(src_rate.toDouble());
                                else currency_rate[0] = src_rate.toDouble();
                            }
                        }
                        xml.readNext();
                    }
      if (xml.hasError()) {
             qDebug()<<"ошибка xml - структуры";// do error handling
      }
    }


    xml.readNext();

}

    file->close();
    ui->label_curr_rate->setText(QString::number(currency_rate[0]));
}
