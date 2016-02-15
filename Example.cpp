// 
// File:   Example.cpp
// Author: leo
//
// Created on 6 de Março de 2008, 20:27
//

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string.h>

#include "quanser.h"



//
//
//
int main(int argc, char** argv) {
    Quanser* q = new Quanser("10.13.97.69", 20072);
    //string leo ;
    q->writeDA(4,1);
    double read = 0;
    read = q->readAD(4);
    cout << read << "\n";
    sleep (1);
    read = q->readAD(4);
    cout << read << "\n";
    sleep (1);
    read = q->readAD(4);
    cout << read << "\n";
    sleep (1);
    read = q->readAD(4);
    cout << read << "\n";
    sleep (1);
    read = q->readAD(4);
    cout << read << "\n";

    return (EXIT_SUCCESS);
}

