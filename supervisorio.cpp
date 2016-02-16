#include "supervisorio.h"
#include "ui_supervisorio.h"
#include <QtCore/qmath.h>
#include <cmath>


supervisorio::supervisorio(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::supervisorio)
{
    ui->setupUi(this);
    QMainWindow::showFullScreen();

    plotRange = 8;

    // Setup plots
    setupPlot1(ui->customPlot);
    setupPlot2(ui->customPlot2);

    //Inicialização para o plot randômico
    timeToNextRandomNumber=0;

    //Inicializa valores
    frequencia = 1;
    amplitude = 3;
    offset = 0;
    duracaoMax = 3;
    duracaoMin = 1;

    //Set valores
        //Frequencia
        ui->doubleSpinBox->setValue(frequencia);
        ui->horizontalSlider->setValue(frequencia*100);
        //Amplitude
        ui->doubleSpinBox_2->setValue(amplitude);
        ui->horizontalSlider_2->setValue(amplitude*100);
        //Offset
        ui->doubleSpinBox_3->setValue(offset);
        ui->horizontalSlider_3->setValue(offset*100);
        //Max
        ui->doubleSpinBox_4->setValue(duracaoMax);
        ui->horizontalSlider_5->setValue(duracaoMax*100);
        //Min
        ui->doubleSpinBox_5->setValue(duracaoMin);
        ui->horizontalSlider_6->setValue(duracaoMin*100);

     // Configura wave padrao
     wave = 0;
     //ui->radioButton_11->setChecked(true);
     on_radioButton_11_clicked();

     //Cria Threads e conecta signals com slots
     cThread = new commThread(this);
     connect(cThread,SIGNAL(plotValues(double,double,double,double,double,double, double)),this,SLOT(onPlotValues(double, double,double,double,double,double,double)));


    //OBS: Bom video para Threads

}

supervisorio::~supervisorio()
{
    delete ui;
}

void supervisorio::setupPlot1(QCustomPlot *customPlot)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  QMessageBox::critical(this, "", "Você deve usar a versão > 4.7");
#endif

  plot1Enable[0]=true;//Red Enabled
  plot1Enable[1]=true;//Blue Enabled

  customPlot->addGraph(); // red line
  customPlot->graph(0)->setPen(QPen(Qt::red));
  customPlot->graph(0)->setAntialiasedFill(false);
  customPlot->addGraph(); // blue line
  customPlot->graph(1)->setPen(QPen(Qt::blue));

  customPlot->addGraph(); // red dot
  customPlot->graph(2)->setPen(QPen(Qt::red));
  customPlot->graph(2)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(2)->setScatterStyle(QCPScatterStyle::ssDisc);
  customPlot->addGraph(); // blue dot
  customPlot->graph(3)->setPen(QPen(Qt::blue));
  customPlot->graph(3)->setLineStyle(QCPGraph::lsNone);
  customPlot->graph(3)->setScatterStyle(QCPScatterStyle::ssDisc);

  customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot->xAxis->setAutoTickStep(false);
  //Valores no eixo X por segundo, proporcao utilizada no exemplo 8/4=2s
  customPlot->xAxis->setTickStep(plotRange/4);
  customPlot->axisRect()->setupFullAxesBox();


  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->yAxis2, SLOT(setRange(QCPRange)));

}

void supervisorio::setupPlot2(QCustomPlot *customPlot2)
{
#if QT_VERSION < QT_VERSION_CHECK(4, 7, 0)
  QMessageBox::critical(this, "", "Você deve usar a versão > 4.7");
#endif

  plot2Enable[0]=true;//Red Enabled
  plot2Enable[1]=true;//Blue Enabled
  plot2Enable[2]=true;//Green Enabled
  plot2Enable[3]=true;//Orange Enabled

  customPlot2->addGraph(); // red line
  customPlot2->graph(0)->setPen(QPen(Qt::red));
  customPlot2->graph(0)->setAntialiasedFill(false);
  customPlot2->addGraph(); // blue line
  customPlot2->graph(1)->setPen(QPen(Qt::blue));
  customPlot2->addGraph(); // green line
  customPlot2->graph(2)->setPen(QPen(Qt::green));
  customPlot2->addGraph(); // orange line
  customPlot2->graph(3)->setPen(QPen(qRgb(255,128,0)));

  customPlot2->addGraph(); // red dot
  customPlot2->graph(4)->setPen(QPen(Qt::red));
  customPlot2->graph(4)->setLineStyle(QCPGraph::lsNone);
  customPlot2->graph(4)->setScatterStyle(QCPScatterStyle::ssDisc);
  customPlot2->addGraph(); // blue dot
  customPlot2->graph(5)->setPen(QPen(Qt::blue));
  customPlot2->graph(5)->setLineStyle(QCPGraph::lsNone);
  customPlot2->graph(5)->setScatterStyle(QCPScatterStyle::ssDisc);
  customPlot2->addGraph(); // green dot
  customPlot2->graph(6)->setPen(QPen(Qt::green));
  customPlot2->graph(6)->setLineStyle(QCPGraph::lsNone);
  customPlot2->graph(6)->setScatterStyle(QCPScatterStyle::ssDisc);
  customPlot2->addGraph(); // orange dot
  customPlot2->graph(7)->setPen(QPen(qRgb(255,128,0)));
  customPlot2->graph(7)->setLineStyle(QCPGraph::lsNone);
  customPlot2->graph(7)->setScatterStyle(QCPScatterStyle::ssDisc);

  customPlot2->xAxis->setTickLabelType(QCPAxis::ltDateTime);
  customPlot2->xAxis->setDateTimeFormat("hh:mm:ss");
  customPlot2->xAxis->setAutoTickStep(false);
  //Valores no eixo X por segundo, proporcao utilizada no exemplo 8/4=2s
  customPlot2->xAxis->setTickStep(plotRange/4);
  customPlot2->axisRect()->setupFullAxesBox();

  // make left and bottom axes transfer their ranges to right and top axes:
  connect(customPlot2->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot2->xAxis2, SLOT(setRange(QCPRange)));
  connect(customPlot2->yAxis, SIGNAL(rangeChanged(QCPRange)), customPlot2->yAxis2, SLOT(setRange(QCPRange)));

}

void supervisorio::updatePlot1(double timeStamp, double redPlot, double bluePlot)
{

    //Red
    if(plot1Enable[0]) {
        ui->customPlot->graph(0)->addData(timeStamp, redPlot);
        ui->customPlot->graph(2)->clearData();
        ui->customPlot->graph(2)->addData(timeStamp, redPlot);
        ui->customPlot->graph(0)->removeDataBefore(timeStamp-plotRange);
        ui->customPlot->graph(0)->rescaleValueAxis();
    }

    //Blue
    if(plot1Enable[1]) {
        ui->customPlot->graph(1)->addData(timeStamp, bluePlot);
        ui->customPlot->graph(3)->clearData();
        ui->customPlot->graph(3)->addData(timeStamp, bluePlot);
        ui->customPlot->graph(1)->removeDataBefore(timeStamp-plotRange);
        ui->customPlot->graph(1)->rescaleValueAxis(true);
    }

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(timeStamp+0.25, plotRange, Qt::AlignRight);
    ui->customPlot->replot();

}

void supervisorio::updatePlot2(double timeStamp,double redPlot, double bluePlot, double greenPlot, double orangePlot)
{


  //Red
  if(plot2Enable[0]) {
      ui->customPlot2->graph(0)->addData(timeStamp, redPlot);
      ui->customPlot2->graph(4)->clearData();
      ui->customPlot2->graph(4)->addData(timeStamp, redPlot);
      ui->customPlot2->graph(0)->removeDataBefore(timeStamp-plotRange);
      ui->customPlot2->graph(0)->rescaleValueAxis(true);
  }

  //Blue
  if(plot2Enable[1]){
      ui->customPlot2->graph(1)->addData(timeStamp, bluePlot);
      ui->customPlot2->graph(5)->clearData();
      ui->customPlot2->graph(5)->addData(timeStamp, bluePlot);
      ui->customPlot2->graph(1)->removeDataBefore(timeStamp-plotRange);
      ui->customPlot2->graph(1)->rescaleValueAxis(true);
  }

  //Green
  if(plot2Enable[2]) {
      ui->customPlot2->graph(2)->addData(timeStamp, greenPlot);
      ui->customPlot2->graph(6)->clearData();
      ui->customPlot2->graph(6)->addData(timeStamp, greenPlot);
      ui->customPlot2->graph(2)->removeDataBefore(timeStamp-plotRange);
      ui->customPlot2->graph(2)->rescaleValueAxis(true);
  }

  //Orange
  if(plot2Enable[3]) {
      ui->customPlot2->graph(3)->addData(timeStamp, orangePlot);
      ui->customPlot2->graph(7)->clearData();
      ui->customPlot2->graph(7)->addData(timeStamp, orangePlot);
      ui->customPlot2->graph(3)->removeDataBefore(timeStamp-plotRange);
      ui->customPlot2->graph(3)->rescaleValueAxis(true);
  }


  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot2->xAxis->setRange(timeStamp+0.25, plotRange, Qt::AlignRight);
  ui->customPlot2->replot();

}

void supervisorio::setLayout(bool frequencia, bool amplitude, bool offset, bool duracao) {
    //Frequencia
    ui->comboBox->setEnabled(frequencia);
    ui->doubleSpinBox->setEnabled(frequencia);
    ui->horizontalSlider->setEnabled(frequencia);
    //Amplitude
    ui->label_2->setEnabled(amplitude);
    ui->doubleSpinBox_2->setEnabled(amplitude);
    ui->horizontalSlider_2->setEnabled(amplitude);
    //Offset
    ui->label_3->setEnabled(offset);
    ui->doubleSpinBox_3->setEnabled(offset);
    ui->horizontalSlider_3->setEnabled(offset);
    //Duracao Max Min
    ui->label_12->setEnabled(duracao);
    //Max
    ui->doubleSpinBox_4->setEnabled(duracao);
    ui->horizontalSlider_5->setEnabled(duracao);
    //Min
    ui->doubleSpinBox_5->setEnabled(duracao);
    ui->horizontalSlider_6->setEnabled(duracao);
}

void supervisorio::setTickStep(void) {
    //Valores no eixo X por segundo, proporcao utilizada no exemplo 8/4=2s
    ui->customPlot->xAxis->setTickStep(plotRange/4);
    ui->customPlot2->xAxis->setTickStep(plotRange/4);

}

//==================================================================================================
//==================================================================================================
//END PLOT
//==================================================================================================
//==================================================================================================


//Relações entre objetos do box "Demais conexões"
void supervisorio::on_doubleSpinBox_valueChanged(double arg1)
{
    ui->horizontalSlider->setValue(arg1*100);
}
void supervisorio::on_doubleSpinBox_2_valueChanged(double arg1)
{
    ui->horizontalSlider_2->setValue(arg1*100);
}
void supervisorio::on_doubleSpinBox_3_valueChanged(double arg1)
{
    if(ui->radioButton_9->isChecked()){//malha aberta
        ui->horizontalSlider_3->setValue((arg1+15)*100);
    }
    else{//malha fechada
        ui->horizontalSlider_3->setValue((arg1)*100);
    }
}

void supervisorio::on_doubleSpinBox_4_valueChanged(double arg1)
{//Primeiro doubleSpinBox do aleatório(max)
    if(arg1<(ui->doubleSpinBox_5->value())){
        ui->doubleSpinBox_4->setValue(ui->doubleSpinBox_5->value());
    }

    ui->horizontalSlider_5->setValue(arg1*100);
}
void supervisorio::on_doubleSpinBox_5_valueChanged(double arg1)
{//Segundo doubleSpinBox do aleatório(min)
    ui->horizontalSlider_6->setValue(arg1*100);
    if(arg1>(ui->doubleSpinBox_4->value())){
        ui->doubleSpinBox_5->setValue(ui->doubleSpinBox_4->value());
    }
}
void supervisorio::on_horizontalSlider_sliderReleased()
{
    ui->doubleSpinBox->setValue((double)ui->horizontalSlider->value()/100);
}
void supervisorio::on_horizontalSlider_2_sliderReleased()
{
    ui->doubleSpinBox_2->setValue((double)ui->horizontalSlider_2->value()/100);
}
void supervisorio::on_horizontalSlider_3_sliderReleased()
{
    if(ui->radioButton_9->isChecked()){//malha aberta
        ui->doubleSpinBox_3->setValue(((double)ui->horizontalSlider_3->value()-1500)/100);
    }
    else{//malha fechada
        ui->doubleSpinBox_3->setValue(((double)ui->horizontalSlider_3->value())/100);
    }
}

void supervisorio::on_horizontalSlider_5_sliderReleased()
{//Primeiro slider do aleatório(max)
    if((ui->horizontalSlider_5->value())<(ui->horizontalSlider_6->value())){
        ui->horizontalSlider_5->setValue(ui->horizontalSlider_6->value());
    }

    ui->doubleSpinBox_4->setValue((double)ui->horizontalSlider_5->value()/100);

}
void supervisorio::on_horizontalSlider_6_sliderReleased()
{//Segundo slider do aleatório(min)
    if((ui->horizontalSlider_5->value())<(ui->horizontalSlider_6->value())){
        ui->horizontalSlider_6->setValue(ui->horizontalSlider_5->value());
    }
    ui->doubleSpinBox_5->setValue((double)ui->horizontalSlider_6->value()/100);}

void supervisorio::on_comboBox_currentIndexChanged(int index)
{
    //Muda os valores quando escolhemos período em vez de frequência
    double aux = ui->doubleSpinBox->minimum();
    double aux2 = ui->doubleSpinBox->value();
    ui->doubleSpinBox->setMinimum(1/ui->doubleSpinBox->maximum());
    ui->doubleSpinBox->setMaximum(1/aux);
    ui->doubleSpinBox->setValue(1/aux2);

    ui->horizontalSlider->setMinimum(ui->doubleSpinBox->minimum()*100);
    ui->horizontalSlider->setMaximum(ui->doubleSpinBox->maximum()*100);
    ui->horizontalSlider->setValue(ui->doubleSpinBox->value()*100);
    index++;//Só para não dar warning

}


//OBS: Para adicionar gráficos vá em Widget -> Promote to -> QCustomPLot

//Enable and disable plots
void supervisorio::on_pushButton_2_clicked()
{
    if(plot1Enable[0]==true){
        ui->pushButton_2->setIcon(QIcon(QString::fromUtf8(":/img/Colors/gray.png")));
        plot1Enable[0]=false;
    }
    else{
        ui->pushButton_2->setIcon(QIcon(QString::fromUtf8(":/img/Colors/red.png")));
        plot1Enable[0]=true;
    }
}
void supervisorio::on_pushButton_3_clicked()
{
    if(plot1Enable[1]==true){
        ui->pushButton_3->setIcon(QIcon(QString::fromUtf8(":/img/Colors/gray.png")));
        plot1Enable[1]=false;
    }
    else{
        ui->pushButton_3->setIcon(QIcon(QString::fromUtf8(":/img/Colors/blue.png")));
        plot1Enable[1]=true;
    }
}
void supervisorio::on_pushButton_4_clicked()
{
    if(plot2Enable[0]==true){
        ui->pushButton_4->setIcon(QIcon(QString::fromUtf8(":/img/Colors/gray.png")));
        plot2Enable[0]=false;
    }
    else{
        ui->pushButton_4->setIcon(QIcon(QString::fromUtf8(":/img/Colors/red.png")));
        plot2Enable[0]=true;
    }
}
void supervisorio::on_pushButton_5_clicked()
{
    if(plot2Enable[1]==true){
        ui->pushButton_5->setIcon(QIcon(QString::fromUtf8(":/img/Colors/gray.png")));
        plot2Enable[1]=false;
    }
    else{
        ui->pushButton_5->setIcon(QIcon(QString::fromUtf8(":/img/Colors/blue.png")));
        plot2Enable[1]=true;
    }
}
void supervisorio::on_pushButton_6_clicked()
{
    if(plot2Enable[2]==true){
        ui->pushButton_6->setIcon(QIcon(QString::fromUtf8(":/img/Colors/gray.png")));
        plot2Enable[2]=false;
    }
    else if (ui->radioButton_10->isChecked()){//Malha está fechada
        ui->pushButton_6->setIcon(QIcon(QString::fromUtf8(":/img/Colors/green.png")));
        plot2Enable[2]=true;
    }
}
void supervisorio::on_pushButton_7_clicked()
{
    if(plot2Enable[3]==true){
        ui->pushButton_7->setIcon(QIcon(QString::fromUtf8(":/img/Colors/gray.png")));
        plot2Enable[3]=false;
    }
    else if (ui->radioButton_10->isChecked()){//Malha está fechada
        ui->pushButton_7->setIcon(QIcon(QString::fromUtf8(":/img/Colors/orange.png")));
        plot2Enable[3]=true;
    }
}

//Malha aberta
void supervisorio::on_radioButton_9_clicked()
{
    ui->pushButton_6->setIcon(QIcon(QString::fromUtf8(":/img/Colors/gray.png")));
    plot2Enable[2]=false;
    ui->pushButton_7->setIcon(QIcon(QString::fromUtf8(":/img/Colors/gray.png")));
    plot2Enable[3]=false;
    ui->label_2->setText("Amplitude (V)");
    ui->label_3->setText("Offset (V)");


    ui->horizontalSlider_3->setValue(1500);
    ui->doubleSpinBox_3->setValue(0);
    ui->doubleSpinBox_3->setMaximum(15);
    ui->doubleSpinBox_3->setMinimum(-15);
}

//Malha fechada
void supervisorio::on_radioButton_10_clicked()
{
    ui->pushButton_6->setIcon(QIcon(QString::fromUtf8(":/img/Colors/green.png")));
    plot2Enable[2]=true;
    ui->pushButton_7->setIcon(QIcon(QString::fromUtf8(":/img/Colors/orange.png")));
    plot2Enable[3]=true;
    ui->label_2->setText("Amplitude (cm)");
    ui->label_3->setText("Offset (cm)");

    ui->horizontalSlider_3->setValue(0);
    ui->doubleSpinBox_3->setValue(0);
    ui->doubleSpinBox_3->setMaximum(30);
    ui->doubleSpinBox_3->setMinimum(0);
}

//Degrau layout
void supervisorio::on_radioButton_11_clicked()
{
    setLayout(false, false, true, false);
    nextWave = degrau;
}

//Senoidal layout
void supervisorio::on_radioButton_12_clicked()
{
   setLayout(true, true, true, false);
   nextWave = senoidal;
}

//Quadrada layout
void supervisorio::on_radioButton_13_clicked()
{
    setLayout(true, true, true, false);
    nextWave = quadrada;
}

//Serra layout
void supervisorio::on_radioButton_14_clicked()
{
    setLayout(true, true, true, false);
    nextWave = serra;

}

//Aleatorio layout
void supervisorio::on_radioButton_15_clicked()
{
    setLayout(false, true, true, true);
    nextWave = aleatorio;
}

//Atualiza valores
void supervisorio::on_pushButton_8_clicked()
{
    //Get wave configurations
    frequencia = ui->doubleSpinBox->value();
    amplitude = ui->doubleSpinBox_2->value();
    offset = ui->doubleSpinBox_3->value();
    duracaoMax = ui->doubleSpinBox_4->value();
    duracaoMin= ui->doubleSpinBox_5->value();
    if(ui->comboBox->currentIndex()==1) frequencia=1/frequencia; //Caso tenhamos escolhido período
    wave = nextWave;
    bool malha = ui->radioButton_9->isChecked();
    cThread->setParameters(frequencia, amplitude, offset, duracaoMax, duracaoMin, wave, malha);
}

void supervisorio::onPlotValues(double timeStamp, double sinalCalculado, double sinalSaturado, double nivelTanque1, double nivelTanque2, double setPoint, double erro){

    //Update plots
    supervisorio::updatePlot1(timeStamp,sinalCalculado,sinalSaturado);
    supervisorio::updatePlot2(timeStamp,nivelTanque1,nivelTanque2,setPoint,erro);

    //Update Water Level
    ui->progressBar->setValue(nivelTanque1*100);
    ui->label_5->setText(QString::number(nivelTanque1,'g',2)+" cm");
    ui->progressBar_2->setValue(nivelTanque2*100);
    ui->label_7->setText(QString::number(nivelTanque2,'g',2)+" cm");
}

void supervisorio::on_scaleValue_valueChanged(int value)
{
    plotRange = value;
    setTickStep();
}


void supervisorio::on_connect_clicked(bool checked)
{
    //Inicia Thread de comunicação
    if (checked) {
        ui->connect->setText("Desconectar");
        ui->connectLabel->setText("Conectado");
        cThread->start();
        cThread->setNullParameters();
    } else {
        ui->connect->setText("Conectar");
        ui->connectLabel->setText("Desconectado");
        cThread->setNullParameters();
        //Espera a thread ler os valores
        QThread::msleep (200);
        //Termina a thread
        cThread->terminate();
    }
}
