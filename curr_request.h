#ifndef CURR_REQUEST_H
#define CURR_REQUEST_H
#include <QDate>

class Curr_request
{
public:
    Curr_request(){
        start_date = QDate::currentDate();
        end_date = QDate::currentDate();
    }
    Curr_request(QDate first_date,QDate second_date){
           start_date = first_date;
           end_date = second_date;
    }
    Curr_request(QDate first_date){
           start_date = first_date;
           end_date = first_date;
    }
    QDate getStart_date() const;
    QDate getEnd_date() const;
private:
    QDate start_date;
    QDate end_date;
};

#endif // CURR_REQUEST_H
