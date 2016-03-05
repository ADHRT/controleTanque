#ifndef SUPERVISORIO_H
#define SUPERVISORIO_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"
#include "commthread.h"


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
    void updatePlot1(double timeStamp, double redPlot, double bluePlot);
    void updatePlot2(double timeStamp, double redPlot, double bluePlot, double greenPlot, double orangePlot);
    double lockSignal(double sinalCalculado, double nivelTanque1);
    commThread *cThread;

public slots:
     void onPlotValues(double timeStamp, double sinalCalculado, double sinalSaturado, double nivelTanque1, double nivelTanque2, double setPoint, double erro);

private slots:
  void on_doubleSpinBox_valueChanged(double arg1);
  void on_doubleSpinBox_2_valueChanged(double arg1);


  void on_horizontalSlider_sliderReleased();

  void on_horizontalSlider_2_sliderReleased();

  void on_doubleSpinBox_3_valueChanged(double arg1);

  void on_horizontalSlider_3_sliderReleased();

  void on_comboBox_currentIndexChanged(int index);

  void on_pushButton_2_clicked();

  void on_pushButton_3_clicked();

  void on_pushButton_4_clicked();

  void on_pushButton_5_clicked();

  void on_pushButton_6_clicked();

  void on_pushButton_7_clicked();

  void on_radioButton_9_clicked();

  void on_doubleSpinBox_4_valueChanged(double arg1);

  void on_horizontalSlider_5_sliderReleased();

  void on_doubleSpinBox_5_valueChanged(double arg1);

  void on_horizontalSlider_6_sliderReleased();

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

private:
    Ui::supervisorio *ui;
    QString demoName;
    QTimer dataTimer;
    bool plot1Enable[2], plot2Enable[4];
    double lastTimeStamp;
    double sinalCalculado;
    double timeToNextRandomNumber;

    double frequencia;
    double amplitude;
    double offset;
    double duracaoMax;
    double duracaoMin;
    double kp, ki, kd;

    int nextWave;
    int wave;

    Control nextControl;
    Control control;

    int plotRange;

    int channel;

    void setLayout(bool frequencia, bool amplitude, bool offset, bool duracao);
    void setControlParams(bool kp, bool ki, bool kd);

    void setTickStep();

//signals:

};

#endif // SUPERVISORIO_H



