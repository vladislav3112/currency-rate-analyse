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
#include <qwt_plot.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>
//add
#include <qwt_plot_magnifier.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());
    ui->comboBox->addItem("доллар");
    ui->comboBox->addItem("евро");
    currency_code.insert("R01235","доллар");
    currency_code.insert("R01239","евро");

    ui->qwtPlot->setTitle( "Курсы валют" );
    ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, "Стоимомть");
    ui->qwtPlot->setAxisTitle(QwtPlot::xBottom, "Дни");
    ui->qwtPlot->insertLegend(new QwtLegend());
    // Включить сетку
    QwtPlotGrid *grid = new QwtPlotGrid(); //
    grid->setMajorPen(QPen( Qt::gray, 2 )); // цвет линий и толщина
    grid->attach(ui->qwtPlot); // добавить сетку к полю графика
    //picker_settings
    QwtPlotPicker *d_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,QwtPlotPicker::CrossRubberBand,
        QwtPicker::ActiveOnly, // включение/выключение
    ui->qwtPlot->canvas() ); // ассоциация с полем
    // Цвет перпендикулярных линий
    d_picker->setRubberBandPen( QColor( Qt::red ) );
    // цвет координат положения указателя
    d_picker->setTrackerPen( QColor( Qt::black ) );
    // непосредственное включение вышеописанных функций
    d_picker->setStateMachine(new QwtPickerDragPointMachine());

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
    code = currency_code.key(ui->comboBox->currentText());
    QString url = "http://www.cbr.ru/scripts/XML_dynamic.asp?date_req1=" + date1 + "&date_req2=" + date2 + "&VAL_NM_RQ=" + code;
    manager->get(QNetworkRequest(QUrl(url)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    //http://www.cbr.ru/scripts/XML_dynamic.asp?date_req1=02/03/2001&date_req2=02/03/2001&VAL_NM_RQ=R01235 - example
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
            file.write(answer.toLatin1());
            break;
        default:
            qDebug()<<"Невозможно открыть XML-конфиг";
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
    code = currency_code.key(ui->comboBox->currentText());
    QString src_rate;

    QFile* file = new QFile("valute_rate.xml");
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) qDebug()<<"Невозможно открыть XML-конфиг";
    QXmlStreamReader xml(file);

    while (!xml.atEnd()) {

            if(xml.name()=="Record"){
                QXmlStreamAttributes attributes = xml.attributes();
             // do processing
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Record"))
                    {
                        if (xml.tokenType() == QXmlStreamReader::StartElement)
                        {
                            if (xml.name() == "Value"){
                                xml.readNext();
                                src_rate = xml.text().toString();
                                src_rate.replace(",",".");       //src data correction
                                currency_rate.push_back(src_rate.toDouble());
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

    addNewCurve(ui->comboBox->currentText(),currency_rate);
}

void MainWindow::addNewCurve(QString currency_name, QVector<double> currency_rate){//add color chooser
    QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setTitle(currency_name);
        curve->setPen( Qt::blue, 6 ); // цвет и толщина кривой
        curve->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // сглаживание

        // Маркеры кривой
        QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
            QBrush( Qt::yellow ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
        curve->setSymbol( symbol );

        // Добавить точки на ранее созданную кривую
        QPolygonF points;

         for(int i = 0;i < currency_rate.size(); i++){
            points << QPointF( i, currency_rate[i]);
         }

         curve->setSamples( points ); // ассоциировать набор точек с кривой
         curve->attach(ui->qwtPlot);
         ui->qwtPlot->replot();
};
