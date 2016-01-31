#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QWidget"
#include "htmwidget.h"
extern "C"
{
	#include <tp.h>
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0, temporalPooler *t = NULL);
    ~MainWindow();
protected:
	void keyPressEvent(QKeyEvent *e);

private:
	HTMWidget *htmWidget;
	temporalPooler *tp;
};

#endif // MAINWINDOW_H
