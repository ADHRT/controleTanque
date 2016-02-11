#ifndef SUPERVISORIO_H
#define SUPERVISORIO_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"


namespace Ui {
class supervisorio;
}

class supervisorio : public QMainWindow
{
    Q_OBJECT

public:
    explicit supervisorio(QWidget *parent = 0);
    ~supervisorio();
     void setupPlot1(QCustomPlot *customPlot);
     void setupPlot2(QCustomPlot *customPlo2);
     void updatePlot1(double timeStamp, double redPlot, double bluePlot);
     void updatePlot2(double timeStamp, double redPlot, double bluePlot, double greenPlot, double orangePlot);
     double lockSignal(double sinalCalculado, double nivelTanque1);

private slots:
  void screenUpdateSlot();
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



  void on_horizontalSlider_4_sliderReleased();

  void on_spinBox_valueChanged(int arg1);

  void on_doubleSpinBox_4_valueChanged(double arg1);

  void on_horizontalSlider_5_sliderReleased();

  void on_doubleSpinBox_5_valueChanged(double arg1);

  void on_horizontalSlider_6_sliderReleased();

private:
    Ui::supervisorio *ui;
    QString demoName;
    QTimer dataTimer;
    bool plot1Enable[2], plot2Enable[4];
    double lastTimeStamp;
    double sinalCalculado;
    double timeToNextRandomNumber;

};

#endif // SUPERVISORIO_H



