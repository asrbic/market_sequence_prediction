#include "mainwindow.h"
#include <QApplication>
extern "C"
{
	#include <tp.h>
}
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	int temporalSize=100;
	int sideSize=16;
	int colPixelSize=20;
	int numCols=sideSize*sideSize;
	int input[numCols];
	memset(input,0,numCols*sizeof(int));
	temporalPooler tp;
	initTP(&tp,sideSize,sideSize,input,1);
	return a.exec();
	destroyTP(&tp);
}
