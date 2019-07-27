/***********************************************************************************
 * Description How to use Command line Interface of GbsDat reader(SQLCipher)
 * ALL Query SQLLite3 Supported
 * Demonstrate using API GBSDAT
 * author : adhitya_1978@ymail.com
 *
************************************************************************************/



#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QCryptographicHash>
#include <QRegExp>
#include <QHash>
#include <QDateTime>
#include <QTime>
#include <QTextStream>


#define CLI_TIMEOUT 5 * 60 * 10000
#define REGEX_LF  "[\\r\\n]"
static QString CMD_GET_ROW = "SELECT COUNT(*) FROM %1 WHERE ";
static QString CMD_GET_SHIFTTIME_BY_EMPID = "SELECT datetime(shiftTime, 'unixepoch', 'localtime') FROM employeeshift ORDER BY emplid=1";
static QStringList _table_list_Shift_time; /*temporary list called once time every table*/


/*redeclare methods*/
static QString getTotalStaff();
static int getTotalRow(QString dbName, QString columsName, QString tableName);
static int getOldBalance(int emplId);
static QString getOldbalance(QString emplid);
static void createShiftTImeList(QStringList &time);
static void splitShiftTime(QString shifttime,QString &rtime);
static QString get_total_row_by_date(QString sdate, QString edate);

////
/// \brief verify of CLI or console file
/// \param path of CLI File
/// \return 0 if success
///
int ReadCLIFile(QString path)
{
    QFile *f = new QFile(path);

    if(!f->open(QIODevice::ReadWrite))
    {
        qDebug() << QString("Fail open file: %1. tips ensure console.exe same directory").arg(path);
        return -1;
    }
    QByteArray a = QCryptographicHash::hash(f->readAll(), QCryptographicHash::Sha1).toHex().toUpper();
    /*change string SHA1 everytime console.exe is changed*/
    bool isvalid = (QString)a == "7039B16669076A80F6DCB5379B3BCBB7C54A7E65";
    if(!isvalid)
    {
        qDebug() << QString("Console.exe its not valid file. please use correct console.exe");
        f->flush(); f->close();
        return -1;
    }
    f->flush(); f->close();
    return 0;
}

////
/// \brief excuteCLISQL
/// \param argument for CLI
/// \param pointer of CLI
/// \return 0 if sucess or no error
///
int excuteCLISQL(QStringList argv, QStringList &m_out)
{
    QRegExp m_regex(REGEX_LF);
    QProcess process;

    process.setProcessChannelMode(QProcess::MergedChannels);
    // process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    process.start("console.exe", argv, QIODevice::ReadOnly);
    if (process.waitForStarted(CLI_TIMEOUT))
    {
        if (!process.waitForFinished(CLI_TIMEOUT))
        {
            process.kill();
        }
        QString error(process.readAllStandardError());
        QString output(process.readAllStandardOutput());

        QStringList m_output = output.split(m_regex, QString::SkipEmptyParts);
        QStringList m_error = error.split(m_regex, QString::SkipEmptyParts);
        if(m_error.count() > 0)
        {
            qDebug() << QString("Process Error. code: %1 ").arg(process.exitCode()).arg(m_error[0]);
        }

        if(m_output.count() > 0)
        {
            for(QStringList::const_iterator _lError = m_output.constBegin();
                _lError != m_output.constEnd(); ++_lError)
            {
                if(!((QString)*_lError).contains("not available") || ((QString)*_lError).contains("not db name") )
                    m_out << *_lError;
            }

        }

        process.close();
        return 0;
    }
    return -1;
}

////
/// \brief parsing_id_time
/// \param in_buffer
/// \param emplId
/// \param shifttime
///
void parsing_id_time(QStringList in_buffer, int &emplId, QDateTime &shifttime)
{
    QString format= "yyyy-MM-dd hh:mm:ss";
    for(QStringList::const_iterator _list = in_buffer.constBegin(); _list != in_buffer.constEnd();++_list)
    {
        QString temp = *_list;
        temp = temp.replace('"', "").trimmed();
        if(!temp.startsWith("Row") && !temp.startsWith("Column") )
        {
            if(QDateTime::fromString(temp, format).isValid())
            {
                shifttime = QDateTime::fromString(temp, format); /*init first time*/
            }
            else
            {
                emplId = temp.toInt();
            }
        }
    }
}

////
/// \brief get List StaffName
/// \param employee id id
/// \param list of staff
///
void getStaffName(int id, QString &staffName)
{
    QStringList argv;
    QString dbname = "Main.gbsdat";
    static QStringList m_buffer;
    argv << "-f"
         << ""
         << "-n"
         << dbname
         << "-c"
         << QString("SELECT  name FROM %1 WHERE Id=%2")
            .arg("Employees")
            .arg(id);
    excuteCLISQL(argv, m_buffer);
    for(QStringList::const_iterator _list = m_buffer.constBegin(); _list != m_buffer.constEnd();++_list)
    {
        QString temp = *_list;
        temp = temp.replace('"', "");
        if(!temp.startsWith("Row") && !temp.startsWith("Column") )
        {
            staffName = temp.trimmed();
        }
    }


}

///
/// \brief timeSummaryOfWorks
/// \param total
///
void timeSummaryOfWorks(QStringList &total)
{
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString columnName = "employeeShifts";
    static QStringList m_buffer;
    QHash<int,QDateTime> totaljamkerja;

    QDateTime shift_in;
    int employee_id =0, total_row =0;

    total_row = getTotalRow(dbname,"emplId" ,columnName);
    for(int i=0; i < total_row; i++)
    {
        argv << "-f"
             << ""
             << "-n"
             << dbname
             << "-c"
             << QString("SELECT  emplId,datetime(shiftTime, 'unixepoch', 'localtime') FROM %1 WHERE Id=%2")
                .arg(columnName)
                .arg(i);

        excuteCLISQL(argv, m_buffer);

        // id no 1
        if(i == 1)
        {
            parsing_id_time(m_buffer, employee_id, shift_in);
        }
        else
        {
            QDateTime next_shift;
            parsing_id_time(m_buffer, employee_id, next_shift);
            qint64 sec = shift_in.secsTo(next_shift); /*calculate start shift & next shift in sec*/
            shift_in = shift_in.addSecs(sec); /*update shift in*/
        }
        if(employee_id > 0)
        {
            if(totaljamkerja.contains(employee_id))
                totaljamkerja.remove(employee_id);
            totaljamkerja.insert(employee_id, shift_in); /*update*/
        }
        argv.clear();
        m_buffer.clear();
    }

    QString staff_name;
    QHashIterator<int, QDateTime> _list(totaljamkerja);
    while (_list.hasNext()) {
        _list.next();
        getStaffName(_list.key(), staff_name);
        total << staff_name + "-"
                 + QString("%1j").arg(_list.value().time().hour()) /*hour time*/
                 + QString("%1m").arg(_list.value().time().minute()) +"-"
                 + QString("Rp.%1").arg(getOldBalance(_list.key()));

    }

}

///
/// \brief create ShiftTIme List
/// \param out list time start in out
///
static void createShiftTImeList(QStringList &time)
{
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString tablename ="EmployeeShifts";
    QStringList m_Buffer;
    int total_row = 0, pos =0;
    QString m_list;


    total_row = getTotalRow(dbname,"emplId" ,tablename);
    for(int i =0; i < total_row + 1; i++)
    {
        argv << "-f"
             << ""
             << "-n"
             << dbname
             << "-c"
             << QString("SELECT datetime(shiftTime, 'unixepoch', 'localtime') FROM %1 where Id=%2")
                .arg(tablename)
                .arg(i);

        excuteCLISQL(argv, m_Buffer);

        for(QStringList::const_iterator _list = m_Buffer.constBegin(); _list != m_Buffer.constEnd();++_list)
        {
            QString temp = *_list;
            temp = temp.replace('"', "").trimmed();
            if(!temp.startsWith("Row") && !temp.startsWith("Column") && !temp.startsWith("Nothing") )
            {
                if(!m_list.endsWith("to"))
                {
                    m_list += temp;
                    m_list += "to";
                }
                else
                {
                    m_list += temp + ";";
                    m_list += temp + "to";
                }
                pos++;
            }
            if(pos == total_row)
                m_list += "end";
        }
        argv.clear();
        m_Buffer.clear();
    }
    time << "\0" ;
    m_list.replace("to", " to ");
    time += m_list.split(';');
}

////
/// \brief get List ShiftTimeby empl id
/// \param employee id
/// \return list shift time
///
QStringList getListShiftTime_by_id(int emplId)
{
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString tablename ="EmployeeShifts";
    QStringList m_Buffer;
    QStringList list_by_staff;


    argv << "-f"
         << ""
         << "-n"
         << dbname
         << "-c"
         << QString("SELECT id FROM %1 where emplId=%2")
            .arg(tablename)
            .arg(emplId);

    if(_table_list_Shift_time.count() == 0)
    {
        createShiftTImeList(_table_list_Shift_time);
    }


    excuteCLISQL(argv, m_Buffer);

    for(QStringList::const_iterator _list = m_Buffer.constBegin(); _list != m_Buffer.constEnd();++_list)
    {
        QString temp = *_list;
        temp = temp.replace('"', "").trimmed();
        if(!temp.startsWith("Row") && !temp.startsWith("Column") && !temp.startsWith("Nothing") )
        {
            list_by_staff << _table_list_Shift_time[temp.toInt()];
        }
    }
    return list_by_staff;
}

////
/// \brief printOutSummaryDetailShift
/// \param file name will be writen
///
void printOutSummaryDetailShift(QString fname)
{
    QStringList list;
    QHash<int, QStringList> m_detailShift;
    QString staffname;

    bool iswrite = fname.length() > 0;


    list = getTotalStaff().split(',');
    for(int i =0; i < list.count(); i++)
    {
        if(list[i] != NULL)
            m_detailShift.insert(list[i].toInt(), getListShiftTime_by_id(list[i].toInt()));
    }
    if(m_detailShift.count() < 0)
        return;

    QHashIterator<int,QStringList> i(m_detailShift);

    if(!iswrite)
    {
        while (i.hasNext()) {
            i.next();
            getStaffName(i.key(),staffname);
            qDebug() << staffname;
            for(QStringList::const_iterator m_time = i.value().constBegin();
                m_time != i.value().constEnd(); ++ m_time)
            {
                QString mTemp = *m_time;
                mTemp = mTemp.trimmed();
                splitShiftTime(mTemp, mTemp);
                // print list time
                qDebug() <<QString("           %1").arg(*m_time) << QString("   Rp.%1").arg(getOldbalance(mTemp));

            }
        }
    }
    else
    {
        QFile f(fname);
        if(!f.open(QIODevice::WriteOnly))
            return;
        QTextStream data(&f);

        while (i.hasNext()) {
            i.next();
            getStaffName(i.key(),staffname);
            data << staffname + "\r\n";
            for(QStringList::const_iterator m_time = i.value().constBegin();
                m_time != i.value().constEnd(); ++ m_time)
            {
                QString mTemp = *m_time;
                mTemp = mTemp.trimmed();
                splitShiftTime(mTemp, mTemp);
                // print list time
                data <<QString("           %1").arg(*m_time) << QString("   Rp.%1").arg(getOldbalance(mTemp));
                data << "\r\n";

            }
        }
        f.close();
    }


}

///
/// \brief splitShiftTime
/// \param shifttime
/// \param rtime
///
static void splitShiftTime(QString shifttime,QString &rtime)
{
    rtime = shifttime.split(" to ")[1];
}

////
/// \brief get no of table row
/// \return
///
static int getTotalRow(QString dbName, QString columsName, QString tableName)
{
    QStringList argv;
    int total_row =0;
    QStringList m_Buffer;

    argv << "-f"
         << ""
         << "-n"
         << dbName
         << "-c"
         << QString("SELECT COUNT(%1) FROM %2")
            .arg(columsName)
            .arg(tableName);

    excuteCLISQL(argv, m_Buffer);

    for(QStringList::const_iterator i = m_Buffer.constBegin(); i != m_Buffer.constEnd();++i)
    {
        QString temp = *i;
        if(!temp.startsWith("Row") && !temp.startsWith("Column") )
            total_row = temp.replace('"',"").toInt(0);
    }
    return total_row;
}

///
/// \brief getTotalRowByValue
/// \param dbname
/// \param value
/// \param tablename
/// \return
///
static int getTotalRowByValue(QString dbname, QString value, QString tablename)
{
    QStringList argv;
    int total_row =0;
    QStringList m_Buffer;
    QString column = "emplId";

    argv << "-f"
         << ""
         << "-n"
         << dbname
         << "-c"
         << QString("SELECT COUNT(%1) FROM %2 where %1=%3")
            .arg(column)
            .arg(tablename)
            .arg(value);

    excuteCLISQL(argv, m_Buffer);

    for(QStringList::const_iterator i = m_Buffer.constBegin(); i != m_Buffer.constEnd();++i)
    {
        QString temp = *i;
        if(!temp.startsWith("Row") && !temp.startsWith("Column") )
            total_row = temp.replace('"',"").toInt(0);
    }

    return total_row;



}

////
/// \brief get no of total staff
/// \return
///
static QString getTotalStaff()
{
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString tablename ="EmployeeShifts";
    QString total_staff =0;
    QStringList m_Buffer;

    argv << "-f"
         << ""
         << "-n"
         << dbname
         << "-c"
         << QString("SELECT DISTINCT emplId FROM %1").arg(tablename);

    excuteCLISQL(argv, m_Buffer);

    for(QStringList::const_iterator i = m_Buffer.constBegin(); i != m_Buffer.constEnd();++i)
    {
        QString temp = *i;
        temp = temp.replace('"',"").trimmed();
        if(!temp.startsWith("Row") && !temp.startsWith("Column"))
        {
            total_staff.append(temp);
            total_staff.append(',');
        }

    }
    return total_staff;
}

////
/// \brief printSummaryOfWorks
///
void printSummaryOfWorks()
{
    QStringList total;
    timeSummaryOfWorks(total);
    QString Header = "==Staff==     ==Total jam Kerja==     ==Pendapatan==     ==Refund==     ==PenggunaanAdmin==";

    qDebug() << Header;
    for(QStringList::const_iterator i = total.constBegin(); i != total.constEnd(); ++i)
    {
        QString temp = *i;
        qDebug() <<QString("%1     ").arg(temp.split('-')[0])
                << QString("%1  ").arg(temp.split('-')[1])
                <<QString("%1   ").arg(temp.split('-')[2]);
    }
    qDebug() <<"================================================================================";

}

///
/// \brief get amount Old Balance
/// \param emplId
/// \return
///
static int getOldBalance(int emplId)
{
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString tablename ="EmployeeShifts";
    QStringList m_Buffer;
    int oldAmountOldBalance =0;

    argv << "-f"
         << ""
         << "-n"
         << dbname
         << "-c"
         << QString("SELECT oldEndBalance FROM %1 where emplId=%2")
            .arg(tablename)
            .arg(emplId);

    excuteCLISQL(argv, m_Buffer);
    for(QStringList::const_iterator i = m_Buffer.constBegin(); i != m_Buffer.constEnd();++i)
    {
        QString temp = *i;
        temp = temp.replace('"', "").trimmed();
        if(!temp.startsWith("Row") && !temp.startsWith("Column") )
            oldAmountOldBalance += temp.toInt();
    }

    return oldAmountOldBalance;
}

///
/// \brief getoldbalance
/// \param dateshiftin
/// \return
///
static QString getOldbalance(QString dateshiftin)
{
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString tablename ="EmployeeShifts";
    QStringList m_Buffer;
    QString result;

    argv << "-f"
         << ""
         << "-n"
         << dbname
         << "-c"
         << QString("SELECT oldEndBalance FROM %1 where datetime(shifttime,'unixepoch', 'localtime')='%2'")
            .arg(tablename)
            .arg(dateshiftin);

    excuteCLISQL(argv,m_Buffer);
    for(QStringList::const_iterator i = m_Buffer.constBegin(); i != m_Buffer.constEnd();++i)
    {
        QString temp = *i;
        temp = temp.replace('"', "").trimmed();
        if(!temp.startsWith("Row") && !temp.startsWith("Column") )
            result = temp;
    }

    return result;

}

///
/// \brief writetofile
/// \param fname
///
static void writetofile(QString fname)
{
    QStringList total;
    QFile *f = new QFile(fname);

    if(!f->open(QIODevice::WriteOnly))
        return;

    QTextStream stream(f);

    timeSummaryOfWorks(total);
    QString Header = "==Staff==     ==Total jam Kerja==     ==Pendapatan==     ==Refund==     ==PenggunaanAdmin==";

    stream << Header;

    for(QStringList::const_iterator i = total.constBegin(); i != total.constEnd(); ++i)
    {
        QString temp = *i;
        stream << "\n";
        stream <<QString("%1     ").arg(temp.split('-')[0])
              << QString("%1  ").arg(temp.split('-')[1])
              <<QString("%1   ").arg(temp.split('-')[2]);
    }

    stream << "\n";
    stream <<"================================================================================";
    f->flush();
    f->close();

}

///
/// \brief get_list_date_by_date
/// \param start date
/// \param end date can be days or date
///
static void get_list_row_by_date(QString sdate, QString edate )
{
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString tablename ="EmployeeShifts";
    QDate inDate, endDate;
    int ldays;
    QStringList list_row;
    QStringList m_Buffer;
    bool isdays = false, isvalid = false;


    /*check date is valid with  format*/
    inDate = isvalid ? QDate::fromString(sdate, "dd-MM-yyyy")  : QDate::fromString(sdate, "yyyy-MM-dd");
    if(!inDate.isValid())
    {
        sdate = sdate.replace('/', '-');
        inDate = isvalid ? QDate::fromString(sdate, "dd-MM-yyyy") : QDate::fromString(sdate, "yyyy-MM-dd");
        if(!inDate.isValid())
        {
            qDebug() << QString("start date not valid. %1" ).arg(sdate);
            return;
        }
    }

    endDate = isvalid ? QDate::fromString(edate, "dd-MM-yyyy") : QDate::fromString(edate, "yyyy-MM-dd");
    if(!endDate.isValid())
    {
        edate = edate.replace('/', '-');
        endDate = isvalid ?  QDate::fromString(edate, "dd-MM-yyyy") : QDate::fromString(edate, "yyyy-MM-dd");
        if(!endDate.isValid())
        {
            //qDebug() << QString("input end is not date. %1" ).arg(edate);
            //qDebug() << "input with days. will add total days from start date. \r\n";
            ldays = edate.toInt(0);
            isdays = true;
        }
    }

    /*if is day add total day to indate*/
    if(isdays)
        endDate = inDate.addDays(ldays);

    list_row.clear();

    /*get total row & convert to unixepoch format*/
    QString temp_row = get_total_row_by_date(QDateTime(inDate).toString("yyyy-MM-dd hh:mm:ss"),
                                             QDateTime(endDate).toString("yyyy-MM-dd hh:mm:ss"));
    if(temp_row.length() > 1)
        list_row = temp_row.split(',');

    if(list_row.isEmpty())
    {
        qDebug() << QString("Warning !!");
        qDebug() << "0";
        return;
    }

    if(list_row.count() == 1)
    {
        for(int i =0; i < list_row.count();i++)
        {
            if(i == list_row.count() -1)
            {
                /*argument with query*/
                argv << "-n"
                     << dbname
                     << "-c"
                     << QString("SELECT oldEndBalance FROM %1 where id=%2")
                        .arg(tablename)
                        .arg(list_row[i].toInt());

                excuteCLISQL(argv,m_Buffer);

                for(QStringList::const_iterator m = m_Buffer.constBegin(); m != m_Buffer.constEnd();++m)
                {
                    QString temp = *m;
                    qDebug() << "Rp." + temp.replace('"', "").trimmed();

                }
            }
        }
    }
    else
    {
        qDebug() << QString("Please ganti shift.");
    }
}

static QString get_total_row_by_date(QString sdate, QString edate)
{
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString tablename ="EmployeeShifts";
    QString list_row = 0;
    QStringList m_Buffer;
    int pos =0;

    /*argument with query*/
    argv << "-f"
         << ""
         << "-n"
         << dbname
         << "-c"
         << QString("SELECT rowid FROM %1 where datetime(shifttime,'unixepoch','localtime') BETWEEN '%2' AND '%3'")
            .arg(tablename)
            .arg(sdate)
            .arg(edate);

    excuteCLISQL(argv,m_Buffer);

    for(QStringList::const_iterator i = m_Buffer.constBegin(); i != m_Buffer.constEnd(); ++i)
    {
        QString temp = *i;
        temp = temp.replace('"', "").trimmed();
        if(!temp.startsWith("Row") && !temp.startsWith("Column") )
        {
            pos++;
            list_row += temp;
        }
        if( pos != m_Buffer.count())
            list_row += ",";

    }

    return list_row.trimmed();

}

static void get_list_row_until_now(QString date)
{
    QDate inDate, endDate;
    QStringList list_row;
    bool isvalid = false;
    QStringList argv;
    QString dbname = "ServerHistory.gbsdat";
    QString tablename ="EmployeeShifts";
    QStringList m_Buffer;

    /*check date is valid with  format*/
    inDate = isvalid ? QDate::fromString(date, "dd-MM-yyyy")  : QDate::fromString(date, "yyyy-MM-dd");
    if(!inDate.isValid())
    {
        date = date.replace('/', '-');
        inDate = isvalid ? QDate::fromString(date, "dd-MM-yyyy") : QDate::fromString(date, "yyyy-MM-dd");
        if(!inDate.isValid())
        {
            qDebug() << QString("start date not valid. %1" ).arg(date);
            return;
        }
    }

    /*get total days until current*/
    int totaldays = inDate.daysTo(QDate::currentDate());

    for(int d = 0; d < totaldays; d++)
    {
        /*set end date */
        endDate = inDate.addDays(d);

        list_row.clear();

        /*get total row & convert to unixepoch format*/
        QString temp_row = get_total_row_by_date(QDateTime(inDate).toString("yyyy-MM-dd hh:mm:ss"),
                                                 QDateTime(endDate).toString("yyyy-MM-dd hh:mm:ss"));
        if(temp_row.length() > 1)
            list_row = temp_row.split(',');

        if(list_row.isEmpty())
        {
            qDebug() << "0";
        }
        if(list_row.count() == 1 || list_row.count() == 0)
        {
            qDebug() << QString("Warning !! Shift di tanggal:%1 & total shift: %2")
                        .arg(endDate.toString("yyyy-MM-dd"))
                        .arg(list_row.count());
            qDebug() << "lastbalance:";
        }

        for(int i =0; i < list_row.count();i++)
        {
            if(i == list_row.count() -1)
            {
                /*argument with query*/
                argv << "-n"
                     << dbname
                     << "-c"
                     << QString("SELECT oldEndBalance FROM %1 where id=%2")
                        .arg(tablename)
                        .arg(list_row[i].toInt());

                excuteCLISQL(argv,m_Buffer);

                for(QStringList::const_iterator m = m_Buffer.constBegin(); m != m_Buffer.constEnd();++m)
                {
                    QString temp = *m;
                    qDebug() << "Rp." + temp.replace('"', "").trimmed();

                }
            }
        }
    }

}

////
/// \brief main or entry point
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    /*Get root directory of main application*/
    QString m_path = QCoreApplication::applicationDirPath();
    QDir::setCurrent(m_path);

    if(ReadCLIFile(m_path + "/console.exe") != 0)
        return -1;
    if(argc < 0)
    {
        qDebug() << "usage "
                    "-s <start date> set start date "
                    "-p <filename> print summary "
                    "-d <total day>"
                    "-now description will calculate how many days from supply date ";
    }

    /*parse*/
    QString sDate, eDate, filename, totalday;

    QStringList argument = a.arguments();
    for(int i = 0; i < argument.count(); i++)
    {
        if(strcmp(QString(argument[i]).toStdString().c_str(), "-s") == 0)
        {
            sDate = argument[i + 1];
        }
        if(strcmp(QString(argument[i]).toStdString().c_str(), "-p") == 0)
        {
            filename = argument[i +1];
        }
        if(strcmp(QString(argument[i]).toStdString().c_str(), "-d") == 0)
        {
            totalday = argument[i +1];
        }
        if(strcmp(QString(argument[i]).toStdString().c_str(), "-now") == 0)
        {

        }

    }

    if(sDate.isEmpty())
    {
        qDebug() << "didnt provide any date.";
    }
    else if(!eDate.isEmpty() && !totalday.isEmpty())
    {
        qDebug() << "choose only 1. end by day or now";
    }
    else
    {
        if(!totalday.isEmpty())
            get_list_row_by_date(sDate.trimmed(),totalday.trimmed());
        if(!eDate.isEmpty())
            get_list_row_until_now(sDate);
    }

    /*print out detail of sumary shift time*/
    if(!filename.isEmpty())
        printOutSummaryDetailShift(filename);


    /*print out summary of working shift time*/
    //printSummaryOfWorks();
    //writetofile("Summary.txt");

    return -1;
}
