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

    // Setup plots
    setupPlot1(ui->customPlot);
    setupPlot2(ui->customPlot2);

    // Setup a timer for updating the screen
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(screenUpdateSlot()));
    dataTimer.start(16); // 16ms= ~ 60fps

    //Inicialização para o plot randômico
    lastTimeStamp=0;
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
     wave = senoidal;
     ui->radioButton_12->setChecked(true);

     //Cria e linca Threads
     cThread = new commThread(this);
     connect(cThread,SIGNAL(waterLevelRead(double,double)),this,SLOT(OnWaterLevelRead(double,double)));


    //OBS: Bom video para Threads

}

supervisorio::~supervisorio()
{
    delete ui;
}


void supervisorio::screenUpdateSlot(){//Runs every time that timer times out

    // Get timeStamp
    double timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

    //Calculates new points
    double nivelTanque1 = qSin(timeStamp)*5+5;
    double nivelTanque2 = qCos(timeStamp)*5+5;

    switch(wave)
    {
    case degrau:
        sinalCalculado = amplitude + offset;
        break;
    case senoidal:
        sinalCalculado = qSin(timeStamp*3.14159265359*frequencia)*amplitude+offset;
        break;
    case quadrada:
        sinalCalculado = qSin(timeStamp*3.14159265359*frequencia)*amplitude+offset;
        if(sinalCalculado>0)sinalCalculado = amplitude;
        else sinalCalculado = -amplitude;
        break;
    case serra:
        sinalCalculado = (fmod((timeStamp*3.14159265359*frequencia), (2*3.14159265359))/(2*3.14159265359))*amplitude*2-amplitude+offset;
        break;
    case aleatorio:
        if((timeStamp-lastTimeStamp)>timeToNextRandomNumber){
            sinalCalculado = (double)rand()/RAND_MAX * amplitude * 2 - amplitude + offset;
            lastTimeStamp=timeStamp;
            timeToNextRandomNumber= ((double)rand()/RAND_MAX) * (duracaoMax-duracaoMin) + duracaoMin;
            if (timeToNextRandomNumber>duracaoMax)timeToNextRandomNumber=duracaoMax;//Isso não deveria acontecer
        }
        break;
    default:
        qDebug() << "ERRO: Nenhuma onda selecionada!";
    }

    //Calculates other points
    double sinalSaturado = supervisorio::lockSignal(sinalCalculado,nivelTanque1);
    double setPoint = qSin(timeStamp*0.5+1);
    double erro = nivelTanque1-setPoint;


    //Update plots
    supervisorio::updatePlot1(timeStamp,sinalCalculado,sinalSaturado);
    supervisorio::updatePlot2(timeStamp,nivelTanque1,nivelTanque2,setPoint,erro);

    //Update Water Level
    ui->progressBar->setValue(nivelTanque1*100);
    ui->label_5->setText(QString::number(nivelTanque1,'g',2)+" cm");
    ui->progressBar_2->setValue(nivelTanque2*100);
    ui->label_7->setText(QString::number(nivelTanque2,'g',2)+" cm");


}

double supervisorio::lockSignal(double sinalCalculado, double nivelTanque1){

    double sinalSaturado=sinalCalculado;

    //Trava 1
    if(sinalCalculado>4) sinalSaturado=4;
    else if(sinalCalculado<-4) sinalSaturado=-4;

    //Trava 2
    if(nivelTanque1<3 && sinalCalculado<0) sinalSaturado=0;

    //Trava 3
    if(nivelTanque1>28 && sinalCalculado>3.25) sinalSaturado=3.25;

    //Trava 4
    if(nivelTanque1>29 && sinalCalculado>0) sinalSaturado=0;

    return sinalSaturado;
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
  customPlot->xAxis->setTickStep(2);
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
  customPlot2->xAxis->setTickStep(2);
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
        ui->customPlot->graph(0)->removeDataBefore(timeStamp-8);
        ui->customPlot->graph(0)->rescaleValueAxis();
    }

    //Blue
    if(plot1Enable[1]) {
        ui->customPlot->graph(1)->addData(timeStamp, bluePlot);
        ui->customPlot->graph(3)->clearData();
        ui->customPlot->graph(3)->addData(timeStamp, bluePlot);
        ui->customPlot->graph(1)->removeDataBefore(timeStamp-8);
        ui->customPlot->graph(1)->rescaleValueAxis(true);
    }

    // make key axis range scroll with the data (at a constant range size of 8):
    ui->customPlot->xAxis->setRange(timeStamp+0.25, 8, Qt::AlignRight);
    ui->customPlot->replot();

}

void supervisorio::updatePlot2(double timeStamp,double redPlot, double bluePlot, double greenPlot, double orangePlot)
{


  //Red
  if(plot2Enable[0]) {
      ui->customPlot2->graph(0)->addData(timeStamp, redPlot);
      ui->customPlot2->graph(4)->clearData();
      ui->customPlot2->graph(4)->addData(timeStamp, redPlot);
      ui->customPlot2->graph(0)->removeDataBefore(timeStamp-8);
      ui->customPlot2->graph(0)->rescaleValueAxis(true);
  }

  //Blue
  if(plot2Enable[1]){
      ui->customPlot2->graph(1)->addData(timeStamp, bluePlot);
      ui->customPlot2->graph(5)->clearData();
      ui->customPlot2->graph(5)->addData(timeStamp, bluePlot);
      ui->customPlot2->graph(1)->removeDataBefore(timeStamp-8);
      ui->customPlot2->graph(1)->rescaleValueAxis(true);
  }

  //Green
  if(plot2Enable[2]) {
      ui->customPlot2->graph(2)->addData(timeStamp, greenPlot);
      ui->customPlot2->graph(6)->clearData();
      ui->customPlot2->graph(6)->addData(timeStamp, greenPlot);
      ui->customPlot2->graph(2)->removeDataBefore(timeStamp-8);
      ui->customPlot2->graph(2)->rescaleValueAxis(true);
  }

  //Orange
  if(plot2Enable[3]) {
      ui->customPlot2->graph(3)->addData(timeStamp, orangePlot);
      ui->customPlot2->graph(7)->clearData();
      ui->customPlot2->graph(7)->addData(timeStamp, orangePlot);
      ui->customPlot2->graph(3)->removeDataBefore(timeStamp-8);
      ui->customPlot2->graph(3)->rescaleValueAxis(true);
  }


  // make key axis range scroll with the data (at a constant range size of 8):
  ui->customPlot2->xAxis->setRange(timeStamp+0.25, 8, Qt::AlignRight);
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
    ui->horizontalSlider_3->setValue((arg1+10)*100);
}
void supervisorio::on_spinBox_valueChanged(int arg1)
{
    //ui->horizontalSlider_4->setValue(arg1);
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
    ui->doubleSpinBox_3->setValue(((double)ui->horizontalSlider_3->value()-1000)/100);
}
void supervisorio::on_horizontalSlider_4_sliderReleased()
{
    //ui->spinBox->setValue(ui->horizontalSlider_4->value());
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
}

//Degrau layout
void supervisorio::on_radioButton_11_clicked()
{
    setLayout(false, true, false, false);
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
}

void supervisorio::OnWaterLevelRead(double waterLevelTank1, double waterLevelTank2){
    ui->label_11->setText(QString::number(waterLevelTank1));
}

void supervisorio::on_pushButton_clicked()
{
   //Inicia Thread de comunicação
   cThread->start();
}
