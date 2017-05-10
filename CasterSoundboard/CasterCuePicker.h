#ifndef CASTERCUEPICKER_H
#define CASTERCUEPICKER_H
#include <QDialog>

//forward declarations
class QMediaPlayer;
class QWidget;
class QSlider;
class QLineEdit;
class QPushButton;
class QLabel;
class QString;
class QVBoxLayout;
class QHBoxLayout;



class CasterCuePicker : public QDialog
{
    Q_OBJECT
public:
    //Constructor
    CasterCuePicker(int _startTime, int _stopTime); //don't forget to pass the parent

    //PROPERTIES
    bool ok;//DiagResult
    int duration;
    int startTime;// Milliseconds
    int stopTime;// Milliseconds
    bool loop;
    QMediaPlayer *player;//Media Player

    //METHODS
    void setVolume(int _volume);
    void setFilePath(QString *_filePath, int _duration, bool _loop);
    void setStartTime(int _startTime);
    void setStopTime(int _stopTime);

    //WIDGETS


protected:
    //PROPERTIES

    //METHODS
    void closeEvent(QCloseEvent *event);

    //WIDGETS

private:
    //PROPERTIES

    //METHODS
    QString getTimeString(int timeInMilliseconds);

    //WIDGETS
    QLabel *label_StartTimeText;
    QLabel *label_StopTimeText;
    QLabel *label_StartTime;
    QLabel *label_StopTime;
    QSlider *slider_StartTime;
    QSlider *slider_StopTime;
    QPushButton *button_Preview;
    QPushButton *button_Cancel;
    QPushButton *button_Accept;

    QVBoxLayout *layout_main;
    QHBoxLayout *layout_startTimeWidgets;
    QHBoxLayout *layout_stopTimeWidgets;
    QHBoxLayout *layout_buttons;

signals:
    //SIGNALS

public slots:
    //SLOTS
    void slider_StartTime_valueChanged(int value);
    void slider_StopTime_valueChanged(int value);
    void button_Preview_Clicked();
    void button_Accept_Clicked();
    void button_Cancel_Clicked();
    void playerPositionChanged(qint64 position);

};

#endif // CASTERCUEPICKER_H
