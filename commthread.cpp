#include "commthread.h"
#include <QtCore>

commThread::commThread(QObject *parent):
    QThread(parent)
{


}

void commThread::run(){

    for(int i=0;i<1000;i++){
        emit waterLevelRead(i*0.25,i*0.5); //Envia sinal do nível dos tanques para ser lido por um slot do supervisório
        this->msleep(100);
    }
    /* Aqui deve ser lido o tempo a todo instante e, somente quando decorrido 0.1s, a comunicação acontece
     * Esse 'for loop' acima é apenas para demonstrar o funcionamento da comunicação entre thread e supervisório */
}

void commThread::OnOutputVoltageChanged(double voltageTank1, double voltageTank2){
    //Recebe o sinal do supervisório nível de tensão a ser enviado para os tanques

    /* Esse sinal vindo do supervisório chegará a uma taxa bem maior do que a taxa de envio
     * de comunicação para os tanques. Portanto, aqui devemos apenas armazenar os valores
     * recebidos para futuramente enviar-los.
    */
}
