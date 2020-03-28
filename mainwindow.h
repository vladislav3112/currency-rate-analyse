#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QNetworkReply>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QMap<QString,QString> currency_code;
    void xml_parse();
private slots:
    void replyFinished(QNetworkReply *reply);
    void on_Button_confirm_clicked();

private:

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
