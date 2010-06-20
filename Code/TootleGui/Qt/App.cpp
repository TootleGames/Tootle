/*
 *  TWxWindow.cpp
 *  TootleGui
 *
 *  Created by Graham Reeves on 14/01/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#if defined(TL_TARGET_MAC)
//	#import <Cocoa/Cocoa.h>	//	access to NSHomeDirectory
#endif

#include "App.h"
#include <TootleCore/TCoreManager.h>
#include <TootleCore/TLCore.h>



MainWindow::MainWindow(QWidget *parent) :
QMainWindow(parent),
ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:

			break;
    }
}


