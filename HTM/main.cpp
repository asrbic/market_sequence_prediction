#include "mainwindow.h"
#include <QApplication>
#include "stdio.h"
#include "htmwidget.h"
#include "btf.h"
#include <string>


extern "C"
{
	#include <tp.h>
}

using namespace std;

int main(int argc, char *argv[])
{
	//QApplication a(argc, argv);

	srand(7);
	int temporalSize=100;
	int sideSize=16;
	int colPixelSize=20;
	int numCols=sideSize*sideSize;
	int input[numCols];
	memset(input,0,numCols*sizeof(int));
	temporalPooler tp;
	initTP(&tp,sideSize,sideSize,input,1);

	string dir("/home/asrbic/views/market_sequence_prediction/test_data/data/");
	BTF btf(&dir);
	btf.runBackTest();
	//MainWindow w(0,&tp);
	//w.show();
	//return a.exec();
	destroyTP(&tp);

	return 0;
}
