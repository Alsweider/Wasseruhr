#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QDebug>
#include <QRandomGenerator>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    timer = new QTimer(this);

    connect(timer, &QTimer::timeout, this, &MainWindow::setTimer);

    styling();

    //Variablen initial definieren
    initVars();

    //Fokus auf das erste Einstellungsfeld setzen
    ui->spinBoxVolumeTotal->setFocus();
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_spinBoxVolumeTotal_valueChanged(int arg1)
{
    totalVolume = ui->spinBoxVolumeTotal->value();
    enableStart();
}


void MainWindow::on_spinBoxVolumeGlass_valueChanged(int arg1)
{
    glassVolume = ui->spinBoxVolumeGlass->value();
    enableStart();
}


void MainWindow::on_spinBoxFrequency_valueChanged(int arg1)
{
    secondsGlass = (ui->spinBoxFrequency->value() * 60);
    //ui->progressBar->setMaximum(ui->spinBoxFrequency->value() * 60);
    enableStart();
}


void MainWindow::on_pushButton_clicked()
{
    //alle Variablen auf genull Nau
    initVars();
    //alle Anzeigen auf genull Nau
    initDisplay();
    //aber ein paar Variablen brauchen wir schon
    totalVolume = ui->spinBoxVolumeTotal->value();
    glassVolume = ui->spinBoxVolumeGlass->value();
    secondsGlass = (ui->spinBoxFrequency->value() * 60);
    randomnessPercent = ui->spinBoxRandomness->value();

    //wenn alle Variablen einen Wert außer Nuhl enthalten
    if (totalVolume && secondsGlass && glassVolume){


        lastRoundActive = false;
        lastRoundCompleted = false;

        //Berechne Anzahl der Gläser: teile Gesamtmenge durch Glasgröße
        portions = (totalVolume / glassVolume);
        //Gesamtzeit berechnen
        secondsTotal = (portions * secondsGlass);
        //Fortschrittsbalken mit Gesamtzeit füllen
        secondsTotalStartwert = secondsTotal;
        ui->progressBarTotal->setMaximum(secondsTotalStartwert);
        //ui->progressBar->setMaximum(ui->spinBoxFrequency->value() * 60);

        //secondsLap = secondsGlass;
        //Zufallswert, falls eingestellt
        secondsLap = randomisedSecondsGlass();
        secondsLapStartwert = secondsLap;

        //Maximalwert des Fortschrittsbalkens der Rundenzeit anpassen
        ui->progressBar->setMaximum(secondsLapStartwert);

        //Zeitschaltuhr starten
        timer->start(1000);

        //Rundenzähler starten
        lapsCounter = 1;
        glassCounter = 0;

        ui->pushButtonLogGlass->setEnabled(true);

        //Pause-Schaltfläche aktivieren
        ui->pushButtonPause->setEnabled(true);

        ui->statusBar->showMessage(QString("Round: %1 | Glasses: %2").arg(lapsCounter).arg(glassCounter));

        beepMessage();
    }

    /* Teste für die Fehlersuche schon mal die
     * Extra-Wartezeit-Generierung, damit nicht
     * jedes mal alle Runden durchlaufen werden müssen */
    for (int i = 0; i < 100; i++){
        int testeWartezeit = generateChallengeSeconds();
        qDebug() << "Test-Wartezeit wäre: " << testeWartezeit / 60 << " Minuten.";
    }

}


void MainWindow::setTimer() {

    //Challenge/Extra-Wartezeit aktiviert
    if (challengeActive){
        if (challengeSeconds > 0){
            challengeSeconds--;
            updateChallengeDisplay();
        } else {
            //Wartezeit abgelaufen
            challengeActive = false;
            timer->stop();
            showChallengeMessage();
        }
        return;
    }

    if (lastRoundActive) {
        //Letzte Runde läuft noch
        if (secondsLap > 0) {
            secondsLap--;
            secondsTotal = qMax(0, secondsTotal - 1); //rein kosmetisch
            updateTimerDisplay();
        } else {
            //Letzte Runde ist nun abgeschlossen
            lastRoundCompleted = true;
            //beepMessage();
            timer->stop();
            updateTimerDisplay();
            //Nach Ende keine Gläser mehr aufzeichnen lassen.
            ui->pushButtonLogGlass->setEnabled(false);

            //Extra-Wartezeit?
            if (ui->checkBoxChallenge->isChecked()){
                startChallenge();
            } else {
                lastRoundActive = false;
                //showFinishPopup();
                beepMessage();
            }
        }
        return;
    }

    //Normale Rundenlogik
    if (secondsLap > 0) {
        secondsLap--;
        secondsTotal = qMax(0, secondsTotal - 1); //kosmetisch
    } else {
        //Runde abgeschlossen
        ui->pushButtonLogGlass->setEnabled(true);

        if (lapsCounter < portions) {
            //Nächste Runde vorbereiten
            secondsLap = randomisedSecondsGlass();
            secondsLapStartwert = secondsLap;

            //Maximalwert des Fortschrittsbalkens der Rundenzeit anpassen
            ui->progressBar->setMaximum(secondsLapStartwert);
            ui->progressBar->setValue(0);

            lapsCounter++;
            beepMessage();
        } else {
            //Letzte Runde einleiten
            lastRoundActive = true;
        }
    }

    updateTimerDisplay();

    //Statuszeile aktualisieren
    ui->statusBar->showMessage(QString("Round: %1 | Glasses: %2")
                                   .arg(lapsCounter)
                                   .arg(glassCounter));
}


void MainWindow::updateTimerDisplay()
{
    // Gesamtzeit
    int h = secondsTotal / 3600;
    int m = (secondsTotal % 3600) / 60;
    int s = secondsTotal % 60;
    ui->timeEditTotal->setTime(QTime(h, m, s));

    // Rundenzeit
    int lm = secondsLap / 60;
    int ls = secondsLap % 60;
    ui->timeEditLap->setTime(QTime(0, lm, ls));

    //Verhindert, dass der Fortschrittsbalken < 0 wird
    int value = qBound(0, secondsLapStartwert - secondsLap, secondsLapStartwert);
    //int value = qBound(0, secondsGlass - secondsLap, secondsGlass);
    ui->progressBar->setValue(value);

    //Fortschrittsbalken der Gesamtzeit aktualisieren
    int totalProgress = secondsTotalStartwert - secondsTotal;
    ui->progressBarTotal->setValue(qBound(0, totalProgress, secondsTotalStartwert));

    qDebug() << "secondsLap:" << secondsLap
             << "progressBar value:" << value
             << "progressBarTotal value:" << totalProgress
             << "maximum:" << ui->progressBar->maximum();

    qDebug() << "totalStart:" << secondsTotalStartwert
             << "totalRest:" << secondsTotal
             << "progressBarTotal value:" << totalProgress
             << "maximum:" << ui->progressBarTotal->maximum();
}


void MainWindow::on_pushButtonLogGlass_clicked()
{
    //Glaszähler erhöhen
    glassCounter++;
    //Statuszeile aktualisieren
    ui->statusBar->showMessage(QString("Round: %1 | Glasses: %2").arg(lapsCounter).arg(glassCounter));
    //Button wieder deaktivieren
    ui->pushButtonLogGlass->setEnabled(false);
}


//Alle Variablen initialisieren
void MainWindow::initVars(){
    totalVolume = 0;
    glassVolume = 0;
    secondsGlass = 0;
    secondsLap = 0;
    secondsTotal = 0;
    portions = 0.0;
    glassCounter = 0;
    lapsCounter = 0;
    isPaused = false;
    secondsTotalStartwert = 0;
    randomnessPercent = 0;
    lastRoundActive = false;
    lastRoundCompleted = false;
    secondsLapStartwert = 0;
    challengeActive = false;
    challengeSeconds = 0;

}


//Benachrichtigungston / Pop-up zum Rundenende
void MainWindow::beepMessage(){

    if(ui->checkBoxBeep->isChecked()){
        QApplication::beep();
    }

    if(ui->checkBoxPopup->isChecked()){

        QMessageBox msgBox(this);

        //Letzte Runde abgeschlossen?
        if (lastRoundCompleted) {
            msgBox.setWindowTitle("Finished");
            msgBox.setText("Last round completed.");
        } else {
            msgBox.setWindowTitle("Drinking Reminder");
            msgBox.setText("It is time to take a glass.");
        }

        msgBox.setIcon(QMessageBox::Information);

        //Als Top-Level-Fenster behandeln und nach vorn holen
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);
        msgBox.activateWindow();
        msgBox.raise();

        msgBox.exec();
    }
}


void MainWindow::on_pushButtonPause_clicked()
{
    if (isPaused) {
        //Timer fortsetzen
        timer->start(1000);
        ui->pushButtonPause->setText("Pause");
    } else {
        // Timer anhalten
        timer->stop();
        ui->pushButtonPause->setText("Resume");
    }

    isPaused = !isPaused; //Zustand umkehren
}


//Aussehen anpassen, z. B. Fortschrittsbalken
void MainWindow::styling(){
    ui->progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid #8F8F91;
            border-radius: 5px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #66CCFF;
            width: 20px;
        }
    )");

    ui->progressBarTotal->setStyleSheet(R"(
        QProgressBar {
            border: 2px solid #8F8F91;
            border-radius: 5px;
            text-align: center;
        }
        QProgressBar::chunk {
            background-color: #66CCFF;
            width: 20px;
        }
    )");

    //Zeitanzeige schminken
    QString timeEditStyle = R"(
    QTimeEdit::up-button, QTimeEdit::down-button {
        width: 0px;
        height: 0px;
        border: none;
    }
)";
    ui->timeEditTotal->setStyleSheet(timeEditStyle);
    ui->timeEditLap->setStyleSheet(timeEditStyle);

}


//Prüfen, ob alle notwendigen Werte gültig eingegeben wurden
bool MainWindow::startCheck(){
    bool inputComplete = false;
    if (glassVolume && totalVolume && secondsGlass){
        //Glas darf nicht größer als Gesamtmenge sein
        if (glassVolume <= totalVolume){
                    inputComplete = true;
        }

    } else {
        //ui->pushButton->setEnabled(false);
        inputComplete = false;
    }

    return inputComplete;
}


void MainWindow::enableStart(){
    if (startCheck()){
        ui->pushButton->setEnabled(true);
    } else {
        ui->pushButton->setEnabled(false);
    }
}


void MainWindow::on_spinBoxRandomness_valueChanged(int arg1)
{
    randomnessPercent = arg1;
}


int MainWindow::randomisedSecondsGlass()
{
    if (randomnessPercent == 0)
        return secondsGlass;

    //Zufälliger Wert zwischen -randomnessPercent und +randomnessPercent
    int delta = QRandomGenerator::global()->bounded(-randomnessPercent, randomnessPercent + 1);

    int adjusted = secondsGlass + (secondsGlass * delta / 100);

    return qMax(1, adjusted);
}


//Damit alles wieder frisch aussieht
void MainWindow::initDisplay(){
    ui->progressBar->setValue(0);
    ui->progressBarTotal->setValue(0);

    int h = 0;
    int m = 0;
    int s = 0;
    ui->timeEditTotal->setTime(QTime(h, m, s));
    ui->timeEditLap->setTime(QTime(h, m, s));

    ui->statusBar->showMessage("");
}

void MainWindow::startChallenge(){
    //Dauer berechnen
    challengeSeconds = generateChallengeSeconds();
    challengeSecondsStartwert = challengeSeconds;
    challengeActive = true;

    //Fortschrittsanzeige ggf. anpassen
    ui->progressBar->setMaximum(challengeSecondsStartwert);
    ui->progressBar->setValue(0);
    ui->progressBarTotal->setMaximum(challengeSecondsStartwert);
    ui->progressBarTotal->setValue(0);

    if (ui->checkBoxPopup->isChecked()){
        //Pop-up zur Ankündigung der Wartezeit
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Waiting Challenge");

        if (ui->checkBoxShowExtraTime->isChecked()){
            msgBox.setText(QString("Please wait for %1 minutes before going to the toilet.")
                               .arg(challengeSeconds / 60));
        } else {
            msgBox.setText(QString("Please wait for a while before going to the toilet."));
        }

        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);
        msgBox.activateWindow();
        msgBox.raise();
        msgBox.exec();
    }

    ui->statusBar->showMessage("Please wait for a while before going to the toilet.");

    // Timer fortsetzen
    timer->start(1000);
}

void MainWindow::updateChallengeDisplay(){
    //wenn die Zeit angezeigt werden soll
    if(ui->checkBoxShowExtraTime->isChecked()){
        //Fortschrittsbalken aktualisieren
        int value = challengeSecondsStartwert - challengeSeconds;
        ui->progressBar->setValue(value);
        ui->progressBarTotal->setValue(value);

        //Zeitdarstellung berechnen
        int h = challengeSeconds / 3600;
        int m = (challengeSeconds % 3600) / 60;
        int s = challengeSeconds % 60;

        //Statuszeile Wartezeit anzeigen
        ui->statusBar->showMessage(
            QString("Waiting time before toilet: %1:%2:%3")
                .arg(h, 2, 10, QChar('0'))
                .arg(m, 2, 10, QChar('0'))
                .arg(s, 2, 10, QChar('0')));

        //Zeit in Uhr darstellen
        ui->timeEditLap->setTime(QTime(h, m, s));
        ui->timeEditTotal->setTime(QTime(h, m, s));
    } else {
        //Nur Wartemeldung in Statuszeile
        ui->statusBar->showMessage("Please be patient...");
    }

}

void MainWindow::showChallengeMessage() {

    if (ui->checkBoxPopup->isChecked()){
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("You may go now");
        msgBox.setText("You may now go to the toilet. Challenge complete!");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowModality(Qt::ApplicationModal);
        msgBox.setWindowFlags(msgBox.windowFlags() | Qt::WindowStaysOnTopHint);
        msgBox.activateWindow();
        msgBox.raise();
        msgBox.exec();
    }

    ui->statusBar->showMessage("Challenge complete!");

    //Zyklus beenden
    initVars();
    initDisplay();
    ui->pushButton->setEnabled(true); //Startknopf aktivieren
}


void MainWindow::on_checkBoxChallenge_stateChanged(int arg1)
{
    //Die Checkbox zur Zeitanzeige sei nur anklickbar, wenn die Extra-Wartezeit aktiviert ist
    if (ui->checkBoxChallenge->isChecked()){
        ui->checkBoxShowExtraTime->setEnabled(true);
    } else {
        ui->checkBoxShowExtraTime->setEnabled(false);
    }
}


//Generiert die Wartezeit zufällig (Experimentell, evtl. Formel für Gewichtung ergänzen.)
int MainWindow::generateChallengeSeconds()
{
    int minWait = 3 * 60; //3 Minuten
    int maxWait = 3 * 60 * 60; //3 Stunden
    int wartezeit = QRandomGenerator::global()->bounded(minWait,  maxWait + 1);
    return wartezeit;
}


