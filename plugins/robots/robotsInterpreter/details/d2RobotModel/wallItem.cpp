#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <QtWidgets/QStyleOptionGraphicsItem>

#include "wallItem.h"

#include "../../../../../qrkernel/settingsManager.h"
#include "d2ModelScene.h"
#include <math.h>

using namespace qReal::interpreters::robots;
using namespace details::d2Model;
using namespace graphicsUtils;

WallItem::WallItem(QPointF const &begin, QPointF const &end)
	: LineItem(begin, end)
	, mDragged(false)
	, mImage(":/icons/2d_wall.png")
	, mOldX1(0)
	, mOldY1(0)
{
	setPrivateData();
	setAcceptDrops(true);
}

void WallItem::VK_setWallPath(int stroke)
{

	QPainterPath wallPath;
	wallPath.moveTo(VK_mP[0]);
	wallPath.lineTo(VK_mP[1]);
	wallPath.lineTo(VK_mP[2]);
	wallPath.lineTo(VK_mP[3]);
	wallPath.lineTo(VK_mP[0]);
	VK_mWallPath = wallPath;
}



void WallItem::setPrivateData()
{
	setZValue(1);
	mPen.setWidth(10);
	mPen.setStyle(Qt::NoPen);
	mBrush.setStyle(Qt::SolidPattern);
	mBrush.setTextureImage(mImage);
	mSerializeName = "wall";

}

QPointF WallItem::begin()
{
	return QPointF(mX1, mY1) + scenePos();

}

QPointF WallItem::end()
{
	return QPointF(mX2, mY2) + scenePos();

}



void WallItem::drawItem(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->drawPath(shape());
	VK_setLines();
	VK_setWallPath();
}

void WallItem::drawExtractionForItem(QPainter *painter)
{
	if (!isSelected()) {
		return;
	}

	painter->setPen(QPen(Qt::green));
	mLineImpl.drawExtractionForItem(painter, mX1, mY1, mX2, mY2, drift);
	mLineImpl.drawFieldForResizeItem(painter, resizeDrift, mX1, mY1, mX2, mY2);

}

void WallItem::mousePressEvent(QGraphicsSceneMouseEvent * event)
{
	AbstractItem::mousePressEvent(event);
	mDragged = (flags() & ItemIsMovable) || mOverlappedWithRobot;
	mOldX1 = qAbs(mX1 - event->scenePos().x());
	mOldY1 = qAbs(mY1 - event->scenePos().y());

}

void WallItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	QPointF const oldPos = pos();
	if (SettingsManager::value("2dShowGrid").toBool() && mDragged && ((flags() & ItemIsMovable) || mOverlappedWithRobot)){
		QPointF const pos = event->scenePos();
		int const indexGrid = SettingsManager::value("2dGridCellSize").toInt();
		qreal const deltaX = (mX1 - mX2);
		qreal const deltaY = (mY1 - mY2);
		mX1 = pos.x() - mOldX1;
		mY1 = pos.y() - mOldY1;
		reshapeBeginWithGrid(indexGrid);
		setDraggedEndWithGrid(deltaX, deltaY);
		mCellNumbX1 = mX1/indexGrid;
		mCellNumbY1 = mY1/indexGrid;
		mCellNumbX2 = mX2/indexGrid;
		mCellNumbY2 = mY2/indexGrid;
	} else if (mDragged) {
		QGraphicsItem::mouseMoveEvent(event);
	}
	// Items under cursor cannot be dragged when adding new item,
	// but it mustn`t confuse the case when item is unmovable
	// because overapped with robot
	if (mDragged && ((flags() & ItemIsMovable) || mOverlappedWithRobot)) {
		emit wallDragged(this, realShape(), oldPos);
	}
	event->accept();

}

bool WallItem::isDragged()
{
	return mDragged;

}

void WallItem::mouseReleaseEvent(QGraphicsSceneMouseEvent * event)
{
	QGraphicsItem::mouseReleaseEvent(event);
	mDragged = false;

}

QDomElement WallItem::serialize(QDomDocument &document, QPoint const &topLeftPicture)
{
	QDomElement wallNode = document.createElement(mSerializeName);
	wallNode.setAttribute("begin", QString::number(mX1 + scenePos().x() - topLeftPicture.x())
			+ ":" + QString::number(mY1 + scenePos().y() - topLeftPicture.y()));
	wallNode.setAttribute("end", QString::number(mX2 + scenePos().x() - topLeftPicture.x())
			+ ":" + QString::number(mY2 + scenePos().y() - topLeftPicture.y()));
	return wallNode;

}

void WallItem::deserializePenBrush(QDomElement const &element)
{
	Q_UNUSED(element)
	setPrivateData();

}

void WallItem::onOverlappedWithRobot(bool overlapped)
{
	mOverlappedWithRobot = overlapped;

}

void WallItem::VK_setLines()
{
	VK_linesList.clear();

	qreal x1 = begin().rx();// mX1;// + scenePos().rx();
	qreal x2 = end().rx();// mX2;// + scenePos().rx();
	qreal y1 = begin().ry();// mY1;// + scenePos().ry();
	qreal y2 = end().ry();// mY2;// + scenePos().ry();

	qreal deltx = x2 - x1;
	qreal delty = y2 - y1;
	qreal len = sqrt(deltx*deltx + delty*delty);
	deltx /= len;
	delty /= len;
	deltx *= 5;
	delty *= 5;

	QVector2D norm (y1 - y2, x2 - x1);
	norm = norm.normalized();
	norm.operator *=(mPen.widthF()/2); //= norm*mPen.widthF();

	VK_mP[0] = QPointF(x1 - deltx + norm.x(), y1 - delty + norm.y());
    VK_mP[1] = QPointF(x1 - deltx - norm.x(), y1 - delty - norm.y());
    VK_mP[2] = QPointF(x2 + deltx - norm.x(), y2 + delty - norm.y());
    VK_mP[3] = QPointF(x2 + deltx + norm.x(), y2 + delty + norm.y());

	//VK_mP[0] += scenePos();
	//VK_mP[1] += scenePos();
	//VK_mP[2] += scenePos();
	//VK_mP[3] += scenePos();

	VK_linesList.push_back(QLineF(VK_mP[0],VK_mP[1]));
    VK_linesList.push_back(QLineF(VK_mP[1],VK_mP[2]));
    VK_linesList.push_back(QLineF(VK_mP[2],VK_mP[3]));
    VK_linesList.push_back(QLineF(VK_mP[3],VK_mP[0]));
}
