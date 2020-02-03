#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(bool* tPtr, int argc, char *argv[], QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    tt(),
    timePoint1(),
    timePoint2(),
    dt(),
    inputparameter(),
    triggerPointer(tPtr)
{
    ui->setupUi(this);

    tt.setLooping(true);
    tt.start();
    connect(&tt, SIGNAL(sendTimeUpdate(const std::tm*)), this, SLOT(receiveTimeUpdate(const std::tm*)));
    tt.forceTimeUpdate();

    *triggerPointer = false;
    shutdownTime = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    initiateInitParam(argc, argv[1], argv[2], argv[3], argv[4]);

    initiateProgram();

    triggerTimeBeforeNowTime = false;
}

MainWindow::~MainWindow()
{
    tt.setLooping(false);
    tt.exit();
    tt.quit();
    tt.terminate();

    delete ui;
}

void MainWindow::receiveTimeUpdate(const std::tm *time)
{
    this->currentTime = *time;
    timeWatchdog();
}

void MainWindow::initiateInitParam(int nParam, char *mode, char *hours, char *minutes, char *seconds)
{
    nParam--;

    if(nParam == 4)
    {
        inputparameter.inhaleParameter(mode, hours, minutes, seconds);

        // Wenn Input- Parameter
        switch (inputparameter.mode)
        {
            default: this->mode = SShutdownMode::TimePoint; break;
            case (0): this->mode = SShutdownMode::TimePoint; break;
            case (1): this->mode = SShutdownMode::Timer; break;
        }

        if(inputparameter.hours != inputparam::INPUT_UNASSIGNED)
        {
            shutdownTime.tm_hour = inputparameter.hours;
        }
        if(inputparameter.minutes != inputparam::INPUT_UNASSIGNED)
        {
            shutdownTime.tm_min = inputparameter.minutes;
        }
        if(inputparameter.seconds != inputparam::INPUT_UNASSIGNED)
        {
            shutdownTime.tm_sec = inputparameter.seconds;
        }

        if(this->mode == SShutdownMode::TimePoint)
        {
            on_shutdownMode_TimePoint_released();
        }
        if(this->mode == SShutdownMode::Timer)
        {
            on_shutdownMode_Timer_released();
        }
        else
        {
            on_shutdownMode_TimePoint_released();
        }

        updateTimeEdits();

        on_startButton_released();
    }
    else
    {
        on_shutdownMode_TimePoint_released();
        on_stopButton_released();
    }
}

void MainWindow::initiateProgram()
{
    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::updateTimeEdits()
{
    ui->hoursEdit->setTime(QTime(shutdownTime.tm_hour, 0, 0));
    ui->minutesEdit->setTime(QTime(0, shutdownTime.tm_min, 0));
    ui->secondsEdit->setTime(QTime(0, 0, shutdownTime.tm_sec));
}

void MainWindow::fetchTimeEdits()
{
    std::tm temp;
    temp = {0, 0, 0, 0, 0, 0, 0, 0, 0};

    temp.tm_hour = QTime(ui->hoursEdit->time()).hour();
    temp.tm_min = QTime(ui->minutesEdit->time()).minute();
    temp.tm_sec = QTime(ui->secondsEdit->time()).second();

    // If the time the pc should shut down is already smaller than the current time, the pc should probably not shut down
    if(convert_to_sec(temp.tm_hour, temp.tm_min, temp.tm_sec) <= convert_to_sec(currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec))
    {
        triggerTimeBeforeNowTime = true;
    }

    shutdownTime = temp;

    //qDebug() << "h: " << shutdownTime.tm_hour << " | min: " << shutdownTime.tm_min << " | sec: " << shutdownTime.tm_sec;
}

void MainWindow::timeWatchdog()
{
    bool triggered = false;

    if(startEnabled)
    {
        if(mode == SShutdownMode::TimePoint)
        {
            triggered = testTimeAboveTimePoint();
        }
        if(mode == SShutdownMode::Timer)
        {
            triggered = timerDroppedToZero();
        }
    }
    if(triggered)
    {
        // Shut down PC
        *triggerPointer = true;
        this->close();
    }
}

bool MainWindow::testTimeAboveTimePoint()
{
    int shutdownTime = convert_to_sec(this->shutdownTime.tm_hour, this->shutdownTime.tm_min, this->shutdownTime.tm_sec);
    int currentTime = convert_to_sec(this->currentTime.tm_hour, this->currentTime.tm_min, this->currentTime.tm_sec);

    qDebug() << "triggerTimeBeforeNowTime = " << triggerTimeBeforeNowTime;

    // If time is after midnight
    if(currentTime >= convert_to_sec(0,0,0) && currentTime < shutdownTime)
    {
        triggerTimeBeforeNowTime = false;
    }

    if(triggerTimeBeforeNowTime)
    {
       qDebug() << "1" << endl;
        return false;
    }

    if(currentTime > shutdownTime)
    {
        qDebug() << "2" << endl;
        return true;
    }
    else
    {
        qDebug() << "3" << endl;
        return false;
    }
}

bool MainWindow::timerDroppedToZero()
{
    /* TimePoint_old = TimePoint;
     * TimePoint = now();
     * dt = TimePoint - TimePoint_old;
     *
     * if(dt >= 1 && shutdownTime > 0)
     * { do "here" }
    */

    // -> Bind "shutdownTime--" to seconds elapsed

    if(firstTime)
    {
        firstTime = false;

        timePoint1 = std::chrono::steady_clock::now();
        timePoint2 = std::chrono::steady_clock::now();
    }

    timePoint2 = timePoint1;
    timePoint1 = std::chrono::steady_clock::now();
    dt = timePoint1 - timePoint2;
    auto _dt = std::abs(dt.count()/1000000);

    qDebug() << "dt: " << _dt;

    int shutdownTime = convert_to_sec(this->shutdownTime.tm_hour, this->shutdownTime.tm_min, this->shutdownTime.tm_sec);

    if(shutdownTime > 0)
    {
        if(_dt && _dt <= 1000 + TimeThread::REFRESH_RATE_TOLERANCE)
        {
            shutdownTime--;
        }
        else
        {
            shutdownTime -= 2;
        }

        QTime temp = convert_to_full(shutdownTime);

        this->shutdownTime.tm_hour = temp.hour();
        this->shutdownTime.tm_min = temp.minute();
        this->shutdownTime.tm_sec = temp.second();

        ui->hoursEdit->setTime(QTime(this->shutdownTime.tm_hour, 0 , 0));
        ui->minutesEdit->setTime(QTime(0, this->shutdownTime.tm_min , 0));
        ui->secondsEdit->setTime(QTime(0, 0 , this->shutdownTime.tm_sec));

        return false;
    }

    return true;
}

int MainWindow::convert_to_sec(int h, int min, int sec)
{
    return ((60 * 60 * h) + (60 * min) + sec);
}

QTime MainWindow::convert_to_full(int seconds)
{
    int hours, minutes;

    hours = seconds / (60*60);
    seconds -= hours * (60*60);

    minutes = seconds / (60);
    seconds -= minutes * (60);

    //qDebug() << "h: " << hours << " | min: " << minutes << " | sec: " << seconds;

    return QTime(hours, minutes, seconds);
}

//==[ UI- Buttons ]============================================================
void MainWindow::on_startButton_released()
{
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);

    ui->shutdownMode_Timer->setEnabled(false);
    ui->shutdownMode_TimePoint->setEnabled(false);

    ui->hoursIncrementButton->setEnabled(false);
    ui->hoursDecrementButton->setEnabled(false);
    ui->hoursEdit->setEnabled(false);

    ui->minutesIncrementButton->setEnabled(false);
    ui->minutesDecrementButton->setEnabled(false);
    ui->minutesEdit->setEnabled(false);

    ui->secondsIncrementButton->setEnabled(false);
    ui->secondsDecrementButton->setEnabled(false);
    ui->secondsEdit->setEnabled(false);

    // Start watchdog
    startEnabled = true;
    qDebug() << "now:\t" << currentTime.tm_hour << ":" << currentTime.tm_min << ":" << currentTime.tm_sec;
    qDebug() << "sd:\t" << shutdownTime.tm_hour << ":" << shutdownTime.tm_min << ":" << shutdownTime.tm_sec;
    qDebug() << "";
}

void MainWindow::on_stopButton_released()
{
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);

    ui->shutdownMode_Timer->setEnabled(true);
    ui->shutdownMode_TimePoint->setEnabled(true);

    ui->hoursIncrementButton->setEnabled(true);
    ui->hoursDecrementButton->setEnabled(true);
    ui->hoursEdit->setEnabled(true);

    ui->minutesIncrementButton->setEnabled(true);
    ui->minutesDecrementButton->setEnabled(true);
    ui->minutesEdit->setEnabled(true);

    ui->secondsIncrementButton->setEnabled(true);
    ui->secondsDecrementButton->setEnabled(true);
    ui->secondsEdit->setEnabled(true);

    // Stop watchdog
    startEnabled = false;
    firstTime = true;
}

void MainWindow::on_hoursIncrementButton_released()
{
    shutdownTime.tm_hour++;
    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_hoursDecrementButton_released()
{
    shutdownTime.tm_hour--;
    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_minutesIncrementButton_released()
{
    shutdownTime.tm_min++;
    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_minutesDecrementButton_released()
{
    shutdownTime.tm_min--;
    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_secondsIncrementButton_released()
{
    shutdownTime.tm_sec++;
    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_secondsDecrementButton_released()
{
    shutdownTime.tm_sec--;
    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_hoursEdit_editingFinished()
{
    QTime temp = ui->hoursEdit->time();
    shutdownTime.tm_hour = temp.hour();

    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_minutesEdit_editingFinished()
{
    QTime temp = ui->minutesEdit->time();
    shutdownTime.tm_min = temp.minute();

    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_secondsEdit_editingFinished()
{
    QTime temp = ui->secondsEdit->time();
    shutdownTime.tm_sec = temp.second();

    updateTimeEdits();
    fetchTimeEdits();
}

void MainWindow::on_shutdownMode_TimePoint_released()
{
    ui->shutdownMode_TimePoint->setChecked(Qt::CheckState::Checked);
    ui->shutdownMode_Timer->setChecked(Qt::CheckState::Unchecked);

    mode = MainWindow::SShutdownMode::TimePoint;
}

void MainWindow::on_shutdownMode_Timer_released()
{
    ui->shutdownMode_TimePoint->setChecked(Qt::CheckState::Unchecked);
    ui->shutdownMode_Timer->setChecked(Qt::CheckState::Checked);

    mode = MainWindow::SShutdownMode::Timer;
}
