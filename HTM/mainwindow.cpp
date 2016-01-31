#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qgridlayout.h"
#include "QWidget"
#include "QKeyEvent"
#include "iostream"

MainWindow::MainWindow(QWidget *parent, temporalPooler* tp) :
	QMainWindow(parent)
{
	this->tp=tp;

	htmWidget = new HTMWidget(0,tp);
	htmWidget->resize(htmWidget->getTotalWidth(),htmWidget->getTotalHeight());
	resize(htmWidget->getTotalWidth(),htmWidget->getTotalHeight());
	setCentralWidget(htmWidget);
	//ui->setupUi(this);
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
	if(e->key()==Qt::Key_C)
	{
		clearTP(tp,0);
		memset(tp->input,0,tp->numCols*sizeof(int));
		htmWidget->clearUIState();
		htmWidget->repaint();
	}
	else if(e->key()==Qt::Key_M)
	{
		htmWidget->inputMode^=1;
	}
	else if(e->key()==Qt::Key_Space)
	{
		runIteration(tp);
		htmWidget->repaint();
	}
	else if(e->key()==Qt::Key_R)
	{
		htmWidget->runTPAfterChange^=1;
	}
}


MainWindow::~MainWindow()
{
	delete(htmWidget);
}
