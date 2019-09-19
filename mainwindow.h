#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QDebug>

#include <iostream>
#include <chrono>
#include <cstring>

#include "timethread.h"
#include "inputparam.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum class SShutdownMode{TimePoint, Timer};
    //enum class ConversionMode{Hours, Minutes, Seconds};

public slots:
    void receiveTimeUpdate(const std::tm *time);

public:
    explicit MainWindow(bool* tPtr, int argc, char *argv[], QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_startButton_released();
    void on_stopButton_released();

    void on_hoursIncrementButton_released();
    void on_hoursDecrementButton_released();
    void on_minutesIncrementButton_released();
    void on_minutesDecrementButton_released();
    void on_secondsIncrementButton_released();
    void on_secondsDecrementButton_released();

    void on_hoursEdit_editingFinished();
    void on_minutesEdit_editingFinished();
    void on_secondsEdit_editingFinished();

    void on_shutdownMode_TimePoint_released();
    void on_shutdownMode_Timer_released();

private:
    void timeWatchdog();
    bool testTimeAboveTimePoint();
    bool timerDroppedToZero();

    void initiateProgram();
    void initiateInitParam(int nParam, char *mode, char *hours, char *minutes, char *seconds);
    void updateTimeEdits();
    void fetchTimeEdits();

    static int convert_to_sec(int h, int min, int sec);
    static QTime convert_to_full(int sec);

private:
    Ui::MainWindow *ui;

    TimeThread tt;

    std::chrono::steady_clock::time_point timePoint1;
    std::chrono::steady_clock::time_point timePoint2;
    std::chrono::steady_clock::duration dt;
    bool firstTime = true;

    inputparam inputparameter;

    SShutdownMode mode;
    std::tm currentTime;
    std::tm shutdownTime;

    bool *triggerPointer;

    bool startEnabled;
};

#endif // MAINWINDOW_H
