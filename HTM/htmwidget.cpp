#include "htmwidget.h"
#include "QPainter"
#include "math.h"
#include "QtGlobal"
#include "QMouseEvent"

HTMWidget::HTMWidget(QWidget *parent, temporalPooler* tp) : QWidget(parent)
{
	this->tp = tp;
	xSize = tp->xDim;
	ySize = tp->yDim;
	double colSize = NUM_CELLS;
	int colSideSize = sqrt(colSize);
	if(floor(colSideSize) != colSideSize)
	{
		colXSize = floor(colSideSize) + 1;
	}
	else
	{
		colXSize = floor(colSideSize);
	}
	colYSize = floor(colSideSize);

	cellPixelSideSize = 10;
	borderWidth = 2;
	doubleBorderWidth = borderWidth * 2;
	colPixelWidth = colXSize * cellPixelSideSize + doubleBorderWidth;
	colPixelHeight = colYSize * cellPixelSideSize + doubleBorderWidth;
	totalWidth = xSize * (colPixelWidth);
	totalHeight = ySize * (colPixelHeight);
	mouseButtonVal = -1;
	//to avoid excessive repaints
	lastMouseButtonVal = -1;
	lastColIndex = -1;
	inputMode=INPUT_MODE_CLEARING;
	runTPAfterChange=1;

}


void HTMWidget::paintEvent(QPaintEvent *event)
{
	//create a QPainter and pass a pointer to the device.
	//A paint device can be a QWidget, a QPixmap or a QImage
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, false);


	painter.fillRect(0,0, totalWidth, totalHeight, Qt::black);
	painter.setBrush(QBrush(Qt::white, Qt::SolidPattern));
	for(int i = 0; i <= xSize;++i) {
		painter.fillRect(i*(colPixelWidth)-borderWidth,0,doubleBorderWidth,totalHeight, Qt::white);
	}
	for(int j = 0; j <= ySize;++j) {
		painter.fillRect(0,j*(colPixelHeight)-borderWidth,totalWidth,doubleBorderWidth, Qt::white);
	}

	int i,j,k,l;
	int colIndex, cellIndex;
	int tempState;

	for(j=0, colIndex=0;j<ySize;++j)
	{

		for(i=0;i<xSize;++i, ++colIndex)
		{
			if(tp->input[colIndex])
			{
				int xStart = (i*(colPixelWidth));
				int yStart = (j*(colPixelHeight));
				painter.fillRect(xStart, yStart, colPixelWidth, colPixelHeight, Qt::blue);
			}
			for(l=0, cellIndex=0;l<colYSize;++l)
			{
				for(k=0;k<colXSize;++k, ++cellIndex)
				{
					tempState=tp->cols[colIndex].cells[cellIndex].state[tp->tIndex];
					int r = 0;
					int g = 0;
					int b = 0;
					if(tempState&ACTIVE_STATE)
					{
						g=255;
					}
					if(tempState&PREDICTIVE_STATE)
					{
						r=127;
					}
					if(tempState&SEQUENCE_SEGMENT)
					{
						r=255;
					}
					if(tempState&LEARN_STATE)
					{
						b=255;
					}
					int xStart = (i*(colPixelWidth)+borderWidth) + (cellPixelSideSize*k);
					int yStart = (j*(colPixelHeight)+borderWidth) + (cellPixelSideSize*l);
					const QColor clr(r,g,b);
					painter.fillRect(xStart, yStart, cellPixelSideSize, cellPixelSideSize, clr);
				}
			}
		}
	}

}


void HTMWidget::mousePressEvent(QMouseEvent * me)
{
	if(me->button()==Qt::LeftButton)
	{
		mouseButtonVal = 1;
	}
	else if(me->button()==Qt::RightButton)
	{
		mouseButtonVal = 0;
	}

	mouseMoveEvent(me);
}

void HTMWidget::mouseMoveEvent(QMouseEvent *me)
{
	if(mouseButtonVal==-1)
	{
		return;
	}
	QPoint pos = me->pos();
	int xPos = pos.x();
	int yPos = pos.y();
	xPos/=colPixelWidth;
	yPos/=colPixelHeight;
	int colIndex = yPos*xSize + xPos;
	if(lastColIndex!=colIndex || lastMouseButtonVal!=mouseButtonVal)
	{
		if(inputMode==INPUT_MODE_CLEARING)
		{
			tp->input[lastColIndex]=0;
		}
		tp->input[colIndex]=mouseButtonVal;
		if(runTPAfterChange)
		{
			runIteration(tp);
		}
		repaint();
		lastMouseButtonVal = mouseButtonVal;
		lastColIndex = colIndex;
	}

}

void HTMWidget::clearUIState()
{
	lastColIndex = -1;
	lastMouseButtonVal = -1;
}

void HTMWidget::mouseReleaseEvent(QMouseEvent *me)
{
	mouseButtonVal = -1;
}

int HTMWidget::getTotalHeight()
{
	return totalHeight;
}

int HTMWidget::getTotalWidth()
{
	return totalWidth;
}

