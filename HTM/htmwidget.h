#ifndef HTMWIDGET_H
#define HTMWIDGET_H

#define INPUT_MODE_CLEARING 1
#define INPUT_MODE_NON_CLEARING 0
#include <QWidget>
extern "C"
{
	#include <tp.h>
}
class HTMWidget : public QWidget
{
	Q_OBJECT
public:
	explicit HTMWidget(QWidget *parent = 0, temporalPooler* tp=NULL);
	int inputMode;
	int runTPAfterChange;
	void clearUIState();
	int getTotalWidth();
	int getTotalHeight();
protected:
	void paintEvent(QPaintEvent *event);
	void mousePressEvent(QMouseEvent * me);
	void mouseReleaseEvent(QMouseEvent * me);
	void mouseMoveEvent(QMouseEvent *me);

private:
	temporalPooler* tp;
	int xSize;
	int ySize;
	int colXSize;
	int colYSize;
	int cellPixelSideSize;
	int borderWidth;
	int doubleBorderWidth;
	int colPixelWidth;
	int colPixelHeight;
	int totalWidth;
	int totalHeight;
	int mouseButtonVal;
	int lastColIndex;
	int lastMouseButtonVal;
signals:

public slots:
};

#endif // HTMWIDGET_H
