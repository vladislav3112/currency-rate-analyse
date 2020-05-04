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
//plot features
#include <qwt_plot.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_grid.h>
#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_dict.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_magnifier.h>
//date
#include <qwt_date.h>
#include <qwt_date_scale_engine.h>
#include <qwt_date_scale_draw.h>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->dateEdit->setDate(QDate::currentDate());
    ui->dateEdit_2->setDate(QDate::currentDate());

    ui->comboBox->addItem("доллар");
    ui->comboBox->addItem("евро");
    ui->comboBox->addItem("фунт стерлингов");
    ui->comboBox->addItem("швейцарский франк (за 10)");
    ui->comboBox->addItem("австралийский доллар");
    ui->comboBox->addItem("бразильский реал");
    ui->comboBox->addItem("датских крон (за 10)");
    ui->comboBox->addItem("казахстанских тенге (за 100)");
    ui->comboBox->addItem("японских иен (за 100)");
    ui->comboBox->addItem("турецкая лира");

    currency_code.insert("R01235","доллар");
    currency_code.insert("R01239","евро");
    currency_code.insert("R01035","фунт стерлингов");
    currency_code.insert("R01775","швейцарский франк (за 10)");
    currency_code.insert("R01010","австралийский доллар");
    currency_code.insert("R01115","бразильский реал");
    currency_code.insert("R01215","датских крон (за 10)");
    currency_code.insert("R01335","казахстанских тенге (за 100)");
    currency_code.insert("R01820","японских иен (за 100)");
    currency_code.insert("R01700J","турецкая лира");

    ui->qwtPlot->setTitle( "Курсы валют" );
    ui->qwtPlot->setAxisTitle(QwtPlot::yLeft, "Стоимомть");
    ui->qwtPlot->setAxisTitle(QwtPlot::xBottom, "Дни");
    ui->qwtPlot->insertLegend(new QwtLegend());

    ui->qwtPlot->setAxisScaleDraw( QwtPlot::xBottom, new QwtDateScaleDraw);
    ui->qwtPlot->setAxisScaleEngine( QwtPlot::xBottom, new QwtDateScaleEngine);

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen( Qt::gray, 2 ));
    grid->attach(ui->qwtPlot);
    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(ui->qwtPlot->canvas());
    magnifier->setMouseButton(Qt::MidButton);

    //move plot enabling:
    QwtPlotPanner *d_panner = new QwtPlotPanner(ui->qwtPlot->canvas() );
    d_panner->setMouseButton( Qt::RightButton );

    ui->comboBox_line_color->addItem("blue");
    ui->comboBox_line_color->addItem("darkMagenta");
    ui->comboBox_line_color->addItem("magenta");
    ui->comboBox_line_color->addItem("green");
    ui->comboBox_line_color->addItem("darkGreen");
    ui->comboBox_line_color->addItem("purple");

    //point coordinates
    QwtPlotPicker *d_picker =
                new QwtPlotPicker(
                    QwtPlot::xBottom, QwtPlot::yLeft,
        QwtPlotPicker::CrossRubberBand,
        QwtPicker::ActiveOnly,
        ui->qwtPlot->canvas() );

        d_picker->setRubberBandPen( QColor( Qt::red ) );
        d_picker->setTrackerPen( QColor( Qt::black ) );
        d_picker->setStateMachine( new QwtPickerDragPointMachine() );
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::on_Button_confirm_clicked()
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QString code;
    QDate date1 = ui->dateEdit->date();
    QDate date2 = ui->dateEdit_2->date();
    QString str_date1 = date1.toString("dd/MM/yyyy");
    QString str_date2 = date2.toString("dd/MM/yyyy");
    code = currency_code.key(ui->comboBox->currentText());

    request = Curr_request(ui->dateEdit->date() , ui->dateEdit_2->date());
    //если первая дата - сб или вс, то запрос надо делать с пятницы
    if (date1.dayOfWeek() == 7 || date1.dayOfWeek() == 6)
        str_date1 = (date1.addDays(5 - date1.dayOfWeek())).toString("dd/MM/yyyy");
    QString url = "http://www.cbr.ru/scripts/XML_dynamic.asp?date_req1=" + str_date1 + "&date_req2=" + str_date2 + "&VAL_NM_RQ=" + code;
    manager->get(QNetworkRequest(QUrl(url)));
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));



    //http://www.cbr.ru/scripts/XML_dynamic.asp?date_req1=02/03/2001&date_req2=02/03/2001&VAL_NM_RQ=R01270 - example
}
void MainWindow::replyFinished(QNetworkReply *reply)
{
    QString answer;
    QFile file ("valute_rate.xml");
    file.open(QIODevice::WriteOnly);


        switch (reply->error())
        {
        case 0:
            answer = QString::fromLatin1(reply->readAll());
            file.write(answer.toLatin1());
            break;
        default:
            qDebug()<<"Невозможно открыть XML-файл";
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
    QDate curr_date;
    QDate prev_date = request.getStart_date().addDays(-1);
    QFile* file = new QFile("valute_rate.xml");
        if (!file->open(QIODevice::ReadOnly | QIODevice::Text)) qDebug()<<"Невозможно открыть XML-файл";
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
                                curr_date = QDate::fromString(attributes.value("Date").toString(),"dd.MM.yyyy");
                                xml.readNext();
                                src_rate = xml.text().toString();
                                src_rate.replace(",",".");       //src data correction
                                currency_rate.push_back(src_rate.toDouble());
                                if(prev_date.daysTo(curr_date)>1)
                                    for(int i = 0; i < prev_date.daysTo(curr_date)-1; i++){
                                       if(currency_rate.size() == 1) currency_rate.push_back(currency_rate.back());
                                       else currency_rate.insert(currency_rate.size()-2,currency_rate[currency_rate.size()-2]);//weekend days insert
                                    }
                                prev_date = curr_date;

                            }
                        }

                        xml.readNext();
                    }
      if (xml.hasError()) {
             qDebug()<<"ошибка xml - структуры";
      }
    }


    xml.readNext();

}
    file->close();
    //weekend days handling

    if(currency_rate.size()!=1 + request.getStart_date().daysTo(request.getEnd_date()) && (request.getStart_date().dayOfWeek() == 6 || request.getStart_date().dayOfWeek() == 7)) currency_rate.pop_front();
    if(currency_rate.size()!=1 + request.getStart_date().daysTo(request.getEnd_date()) && request.getStart_date().dayOfWeek() == 7) currency_rate.pop_front();

    while(currency_rate.size()< 1 + request.getStart_date().daysTo(request.getEnd_date())){
        currency_rate.push_back(currency_rate.back());
    }
    addNewCurve(ui->comboBox->currentText(),currency_rate,ui->comboBox_line_color->currentText());//add new line.
}

void MainWindow::addNewCurve(QString currency_name, QVector<double> currency_rate, QString color){//add color chooser
    QwtPlotCurve *curve = new QwtPlotCurve();
        curve->setTitle(currency_name);
        curve->setPen( QColor(color), 6 );
        curve->setRenderHint( QwtPlotItem::RenderAntialiased, true ); // alising

        QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
            QBrush( Qt::yellow ), QPen( Qt::red, 2 ), QSize( 8, 8 ) );
        curve->setSymbol( symbol );

        // add points for curve
        QPolygonF points;

         for(int i = 0; i < currency_rate.size(); i++){
            points << QPointF( QwtDate::toDouble(QDateTime(request.getStart_date().addDays(i))), currency_rate[i]);
         }

         curve->setSamples( points ); // points assotiation
         curve->attach(ui->qwtPlot);
         ui->qwtPlot->replot();
};

void MainWindow::on_Button_clr_plot_clicked()
{
    ui->qwtPlot->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen(QPen( Qt::gray, 2 ));
    grid->attach(ui->qwtPlot);
}
