#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QUrl>
#include <QFile>
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

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    //xml parse
    QMap<QString,QString> current_rate;
    QMap<QString,QString> etap;
    current_rate.insert("R01010","dollar");
    current_rate.insert("R01239","euro");


    QFile* file = new QFile("valute_rate.xml");
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
        {
            qDebug()<<"Невозможно открыть XML-конфиг";
        }
    QXmlStreamReader xml(file);


    while (!xml.atEnd()) {

            if(xml.name()=="Valute"){
                QXmlStreamAttributes attributes = xml.attributes();
                bool a = attributes.hasAttribute("ID");
             // do processing
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Valute"))
                    {
                        if (xml.tokenType() == QXmlStreamReader::StartElement &&
                                (current_rate.key("dollar") == attributes.value("ID").toString()
                                 ))
                        {
                            if (xml.name() == "Value"){
                                xml.readNext();
                                if(etap.isEmpty())etap.insert("dollar_rate",xml.text().toString());
                                else etap["dollar_rate"] = xml.text().toString();
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
    ui->label_3->setText(etap["dollar_rate"]);
}
void MainWindow::replyFinished(QNetworkReply *reply){
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

}

void MainWindow::on_Button_confirm_clicked()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString tmp = ui->dateEdit->date().toString("dd/MM/yyyy");
    manager->get(QNetworkRequest(QUrl("http://www.cbr.ru/scripts/XML_daily.asp?date_req="+tmp)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}
