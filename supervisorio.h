#ifndef SUPERVISORIO_H
#define SUPERVISORIO_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include "commthread.h"
#include "analist.h"

namespace Ui {
class supervisorio;
}

class supervisorio : public QMainWindow
{
    Q_OBJECT

public:
    explicit supervisorio(QWidget *parent = 0);
    ~supervisorio();
    enum Wave { degrau, senoidal, quadrada, serra, aleatorio };
    enum Control { P, PI, PD, PID, PI_D, SEM };
    void setupPlot1(QCustomPlot *customPlot);
    void setupPlot2(QCustomPlot *customPlo2);
    void updatePlot1(double timeStamp, double redPlot, double bluePlot, double greenPlot, double orangePlot, double blue2Plot, double green2Plot, double orange2Plot);
    void updatePlot2(double timeStamp, double redPlot, double bluePlot, double greenPlot, double orangePlot);
    double lockSignal(double sinalCalculado, double nivelTanque1);
    commThread *cThread;
    Analist *analist;

public slots:
     void onPlotValues(double timeStamp, double sinalCalculadoMestre, double sinalCalculadoEscravo, double sinalSaturado, double nivelTanque1, double nivelTanque2, double setPoint, double erro, double iMestre, double iEscravo, double dMestre, double dEscravo);

private slots:
  void on_comboBox_currentIndexChanged(int index);

  void on_pushButton_2_clicked();

  void on_pushButton_3_clicked();

  void on_pushButton_4_clicked();

  void on_pushButton_5_clicked();

  void on_pushButton_6_clicked();

  void on_pushButton_7_clicked();

  void on_radioButton_9_clicked();

  void on_radioButton_10_clicked();

  void on_pushButton_8_clicked();

  void on_scaleValue_valueChanged(int value);

  void on_connect_clicked(bool checked);

  void on_spinBox_valueChanged(int arg1);

  void on_demo_clicked(bool checked);

  void on_canal0_clicked();

  void on_canal1_clicked();

  void on_canal2_clicked();

  void on_canal3_clicked();

  void on_canal4_clicked();

  void on_canal5_clicked();

  void on_canal6_clicked();

  void on_canal7_clicked();

  void on_comboBox_6_currentIndexChanged(int index);

  void on_comboBox_tipoControle_currentIndexChanged(int index);

  void on_comboBox_3_currentIndexChanged(int index);

  void on_comboBox_4_currentIndexChanged(int index);

  void on_pushButton_10_clicked();

  void on_pushButton_9_clicked();

  void on_radioButton_tanque1_clicked();

  void on_radioButton_tanque2_clicked();

  void on_comboBox_tr_currentIndexChanged(int index);

  void on_comboBox_ts_currentIndexChanged(int index);

  void on_button_limpar_clicked();

  void on_pushButton_zerar_clicked();

  void on_checkBox_9_clicked(bool checked);

  void on_comboBox_tipoControle_2_currentIndexChanged(int index);

  void on_comboBox_8_currentIndexChanged(int index);

  void on_comboBox_7_currentIndexChanged(int index);

  void on_demo_clicked();

  void on_pushButton_11_clicked(bool checked);

  void on_pushButton_12_clicked(bool checked);

  void on_pushButton_13_clicked(bool checked);

private:
    Ui::supervisorio *ui;
    QString demoName;
    QTimer dataTimer;
    bool plot1Enable[7], plot2Enable[4];
    double lastTimeStamp;
    double sinalCalculado[2];
    double timeToNextRandomNumber;

    double frequencia;
    double amplitude;
    double offset;
    double duracaoMax;
    double duracaoMin;
    double kp[2], ki[2], kd[2], taw[2];
    bool windup[2], conditionalIntegration[2];
    bool cascade;

    int nextWave;
    int wave;

    Control nextControl[2];
    Control control[2];

    int plotRange;

    int channel;

    void setLayout(bool frequencia, bool amplitude, bool offset, bool duracao);
    void setControlParams(bool kp, bool ki, bool kd);
    void setControlParamsSlave(bool kp, bool ki, bool kd);

    void setTickStep();

//signals:
};

#endif // SUPERVISORIO_H



