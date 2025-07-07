#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

    void on_spinBoxVolumeGlass_valueChanged(int arg1);

    void on_spinBoxFrequency_valueChanged(int arg1);

    void setTimer();

    void on_spinBoxVolumeTotal_valueChanged(int arg1);

    void on_pushButtonLogGlass_clicked();

    void on_pushButtonPause_clicked();

    void on_spinBoxRandomness_valueChanged(int arg1);

    void on_checkBoxChallenge_stateChanged(int arg1);

private:
    Ui::MainWindow *ui;

    int totalVolume;
    int glassVolume;
    int secondsGlass;
    int secondsLap;
    int secondsTotal;
    int secondsTotalStartwert;
    double portions;
    QTimer *timer;
    void updateTimerDisplay();
    int glassCounter;
    int lapsCounter;
    void initVars();
    void beepMessage();
    bool isPaused;
    void styling();
    bool startCheck();
    void enableStart();
    int randomnessPercent;
    int randomisedSecondsGlass();
    bool lastRoundActive;
    bool lastRoundCompleted;
    void initDisplay();
    int secondsLapStartwert;
    bool challengeActive;
    int challengeSeconds;
    int challengeSecondsStartwert;
    void updateChallengeDisplay();
    void showChallengeMessage();
    void startChallenge();
    void showFinishPopup();
    int generateChallengeSeconds();


};
#endif // MAINWINDOW_H
