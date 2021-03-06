#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include <curr_request.h>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void xml_parse();
    Curr_request request;
private slots:
    void replyFinished(QNetworkReply *reply);
    void on_Button_confirm_clicked();

    void on_Button_clr_plot_clicked();

private:
    QMap<QString,QString> currency_code;
    void addNewCurve(QString currency_name, QVector<double> currency_rate, QString color);
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
