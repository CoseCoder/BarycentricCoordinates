#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>
#include "ChildWindow.h"
#include "WP.h"
#include "MV.h"
#include "DH.h"
using namespace cv;
using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow *relateWindow)
{
	image_ = new QImage();
	image_backup_ = new QImage();
	image_temp_ = new QImage();

	draw_status_ = kNone;
	is_choosing_ = false;
	is_pasting_ = false;
	is_moving_ = false;
	is_drawing_ = false;
	warping_state_ = 0;
	warping_method_ = 0;
	figure_type_ = 0;
	index_ = -1;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);

	source_window_ = NULL;
	setMouseTracking(true);
}

ImageWidget::~ImageWidget(void)
{
	delete image_, image_backup_, image_temp_;
}

int ImageWidget::ImageWidth()
{
	return image_->width();
}

int ImageWidget::ImageHeight()
{
	return image_->height();
}

void ImageWidget::set_draw_status_to_choose()
{
	draw_status_ = kChoose;
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
}

void ImageWidget::set_draw_status_to_warp(int index)
{
	draw_status_ = kWarp;
	warping_method_ = index;
}

void ImageWidget::set_draw_figure(int index)
{
	figure_type_ = index;
}

double  computeSquareOfDistance(const QPoint &p1, const QPoint &p2) {
	return (p1.x() - p2.x())*(p1.x() - p2.x()) + (p1.y() - p2.y())*(p1.y() - p2.y());
}


void scanLine(QVector<QPoint>& vertex, QVector<QPoint> &points, int width) {
	QPolygon polygon(vertex);

	int maxY = 0, minY = INT_MAX;

	for (int i = 0; i < vertex.size(); i++)
	{
		if (vertex[i].y() > maxY)
			maxY = vertex[i].y();
		if (vertex[i].y() < minY)
			minY = vertex[i].y();
	}

	for (int y = minY; y <= maxY; y++) {
		for (int x = 0; x < width; x++)
		{
			QPoint t(x, y);
			if (polygon.containsPoint(t, Qt::OddEvenFill))
				points.append(t);
		}
	}
}

bool isInsidePoly(const QPointF &iPoint, const QPolygonF &iMyPoly)
{
	int   i, j = iMyPoly.size() - 1;
	int x = iPoint.x(), y = iPoint.y();
	bool  oddNodes = false;

	for (i = 0; i < iMyPoly.size(); i++) {
		if ((iMyPoly[i].y() < y && iMyPoly[j].y() >= y
			|| iMyPoly[j].y() < y && iMyPoly[i].y() >= y)
			&& (iMyPoly[i].x() <= x || iMyPoly[j].x() <= x)) {
			if (iMyPoly[i].x() + (y - iMyPoly[i].y()) / (iMyPoly[j].y() - iMyPoly[i].y())*(iMyPoly[j].x() - iMyPoly[i].x()) < x) {
				oddNodes = !oddNodes;
			}
		}
		j = i;
	}
	return oddNodes;

}

bool isOnLine(const QPointF &iPoint, const QPolygonF &iMyPoly) {
	double x = iPoint.x(), y = iPoint.y();
	double k1, k2;
	int size = iMyPoly.size();
	for (int i = 0; i < size; i++) {
		if (iMyPoly[i].x() == x&&iMyPoly[i].y() == y)
			return true;
		k1 = (y - iMyPoly[i].y()) / (x - iMyPoly[i].x());
		k2 = (y - iMyPoly[(i + 1) % size].y()) / (x - iMyPoly[(i + 1) % size].x());
		if (k1 == k2 && ((x<iMyPoly[i].x() && x>iMyPoly[(i + 1) % size].x()) || (x > iMyPoly[i].x() && x < iMyPoly[(i + 1) % size].x())))
			return true;
	}
	return false;
}

QImage* ImageWidget::image()
{
	return image_;
}

void ImageWidget::set_source_window(ChildWindow* childWindow)
{
	source_window_ = childWindow;
}

void ImageWidget::getTriangles()
{
	last.clear();

	QPolygon tpolygon(origon_points);
	QRect qrect = tpolygon.boundingRect();
	Rect rect(qrect.x(), qrect.y(), qrect.width(), qrect.height());
	Subdiv2D subdiv2d(rect);

	Point2f fp;
	for (int i = 0; i < origon_points.size(); i++) {
		fp = Point2f(origon_points[i].x(), origon_points[i].y());
		subdiv2d.insert(fp);
	}

	vector<cv::Vec6f> triangleList;
	subdiv2d.getTriangleList(triangleList);

	bool in;
	QVector<QPoint> t;
	for (int i = 0; i < triangleList.size(); i++) {
		in = true;
		t.clear();
		for (int j = 0; j < 3; j++) {
			QPoint p(triangleList[i][2 * j], triangleList[i][2 * j + 1]);
			t.append(p);
			if (!(isInsidePoly(p, tpolygon) || isOnLine(p, tpolygon))) {
				in = false;
				break;
			}
		}
		if (in)
			origon.append(t);
	}
	last = origon;
}

void ImageWidget::FillHole(QVector<QPoint> &mappingPoints, QVector<QPoint> &vertex, QVector<QColor> &color)
{
	BarycentricCoordinates *p;
	QVector<QPoint> Points;
	p->scanLine(vertex, Points);
	int output;
	int x[8] = { 0,1,1,1,0,-1,-1,-1 };
	int y[8] = { -1,-1,0,1,1,1,0,-1 };
	QPoint qp;
	QColor qc;
	for (int i = 0; i < Points.size(); i++) {
		if (!mappingPoints.contains(Points[i])) {

			int tr = 0, tg = 0, tb = 0;		//周围8个点的颜色信息(用于求red,green,blue的和)
			bool in = false;

			for (int p = 0; p < 8; p++)		//分别计算周围8个点的红,绿,蓝值之和与周围已被映射到的点的红,绿,蓝值之和
			{
				qp = QPoint(Points[i].x() + x[p], Points[i].y() + y[p]);
				qc = image_->pixelColor(qp);
				if (mappingPoints.contains(qp))
				{
					image_->setPixelColor(Points[i], image_->pixelColor(qp));
					in = true;
					break;
				}
				tr += qc.red();
				tg += qc.green();
				tb += qc.blue();
			}
			/*output = kNearestP::kClosest(mappingPoints, Points[i], 1);
			image_->setPixelColor(Points[i], color[output]);*/

			//若周围有已被映射到的点,则确定该点的颜色为周围已被映射到的点的颜色平均值,否则为周围8个点的颜色平均值
			if (!in)
				image_->setPixelColor(Points[i], QColor(tr / 8, tg / 8, tb / 8));
		}
	}
}

void ImageWidget::paintEvent(QPaintEvent *paintEvent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect(0, 0, image_->width(), image_->height());
	painter.drawImage(rect, *image_);

	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setPen(Qt::red);


	if (figure_type_ == 1)
		painter.drawPolygon(points_);
	else if (figure_type_ == 2 && is_drawing_)
		painter.drawPolyline(points_);
	else if (figure_type_ == 2 && !is_drawing_)
		painter.drawPolygon(points_);


	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent *mouseEvent)
{
	if (Qt::LeftButton == mouseEvent->button())
	{
		switch (draw_status_)
		{
		case kChoose:
			is_choosing_ = true;
			point_start_ = point_end_ = mouseEvent->pos();
			break;

		case kPaste:
		{
			if (source_window_ == NULL)
				break;

			is_pasting_ = true;

			// Start point in object image
			int xpos = mouseEvent->pos().rx();
			int ypos = mouseEvent->pos().ry();

			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->point_start_.rx();
			int ysourcepos = source_window_->imagewidget_->point_start_.ry();

			// Width and Height of rectangle region
			int w = source_window_->imagewidget_->point_end_.rx()
				- source_window_->imagewidget_->point_start_.rx() + 1;
			int h = source_window_->imagewidget_->point_end_.ry()
				- source_window_->imagewidget_->point_start_.ry() + 1;

			if (w < 0) {
				xsourcepos += w;
				w = -w;
			}
			if (h < 0) {
				ysourcepos += h;
				h = -h;
			}

			// Paste
			if ((xpos + w < image_->width()) && (ypos + h < image_->height()))
			{
				// Restore image
			//	*(image_) = *(image_backup_);
				// Paste
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						image_->setPixel(xpos + i, ypos + j, source_window_->imagewidget_->image()->pixel(xsourcepos + i, ysourcepos + j));
					}
				}
			}
			break;
		}

		case kWarp: {
			if (source_window_ == NULL)
				break;

			int x = mouseEvent->pos().x(), y = mouseEvent->pos().y();
			QPoint p(x, y);
			if (!is_drawing_ && warping_state_ == 1 && index_ != -1) {
				is_moving_ = true;
				warping_state_ = 2;
				return;
			}

			if (!is_choosing_)
			{
				if (1 == figure_type_) {
					points_.clear();
					is_choosing_ = true;
					point_start_ = point_end_ = mouseEvent->pos();
					points_.append(point_start_);
					points_.append(QPoint(mouseEvent->pos()));
					points_.append(QPoint(mouseEvent->pos()));
					points_.append(point_end_);
				}
				else if (2 == figure_type_) {
					if (!is_drawing_)
						points_.clear();
					is_choosing_ = true;
					points_.append(mouseEvent->pos());
					is_drawing_ = true;
				}

				warping_state_ = 1;
			}

			break;
		}

		default:
			break;
		}
	}
	else if (Qt::RightButton == mouseEvent->button()) {
		if (draw_status_ == kWarp&&is_drawing_) {
			is_choosing_ = true;
			is_drawing_ = false;
			origon_points = points_;
			getTriangles();
		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent *mouseEvent)
{
	switch (draw_status_)
	{
	case kChoose:
		// Store point position for rectangle region
		if (is_choosing_)
		{
			point_end_ = mouseEvent->pos();
		}
		break;

	case kPaste:
		// Paste rectangle region to object image
		if (is_pasting_)
		{
			// Start point in object image
			int xpos = mouseEvent->pos().rx();
			int ypos = mouseEvent->pos().ry();

			// Start point in source image
			int xsourcepos = source_window_->imagewidget_->point_start_.rx();
			int ysourcepos = source_window_->imagewidget_->point_start_.ry();

			// Width and Height of rectangle region
			int w = source_window_->imagewidget_->point_end_.rx()
				- source_window_->imagewidget_->point_start_.rx() + 1;
			int h = source_window_->imagewidget_->point_end_.ry()
				- source_window_->imagewidget_->point_start_.ry() + 1;

			if (w < 0) {
				xsourcepos += w;
				w = -w;
			}
			if (h < 0) {
				ysourcepos += h;
				h = -h;
			}

			// Paste
			if (((xpos > 0) || (ypos > 0)) && ((xpos + w < image_->width()) || (ypos + h < image_->height())))
			{
				// Restore image 
				*(image_) = *(image_temp_);

				if (xpos <= 0)
					xpos = 1;
				else if (ypos <= 0)
					ypos = 1;

				if (xpos + w >= image_->width())
					xpos = image_->width() - w - 1;
				else if (ypos + h >= image_->height())
					ypos = height() - h - 1;

				// Paste
				for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						image_->setPixel(xpos + i, ypos + j, source_window_->imagewidget_->image()->pixel(xsourcepos + i, ysourcepos + j));
					}
				}
			}
		}
		break;

	case kWarp: {
		int x = mouseEvent->pos().x(), y = mouseEvent->pos().y();
		QPoint p(x, y);
		if (!is_choosing_) {
			//setCursor(Qt::ArrowCursor);
			for (int i = 0; i < points_.size(); i++) {
				if (computeSquareOfDistance(p, points_[i]) < 37) {
					//setCursor(Qt::CrossCursor);
					index_ = i;
					break;
				}
			}
		}

		if (2 == warping_state_&&is_moving_ && !is_drawing_)
		{
			*image_ = *image_temp_;
			for (int i = 0; i < last.size(); i++) {
				for (int j = 0; j < 3; j++)
					if (last[i][j] == points_[index_])
						last[i][j] = mouseEvent->pos();
			}
			points_[index_] = mouseEvent->pos();
			QVector<QPoint> mappingPoints;
			QVector<QColor> color;

			if (1 == warping_method_) {
				for (int i = 0; i < last.size(); i++) {
					mappingPoints.clear();
					color.clear();
					WP wp(origon[i]);
					for (int j = 0; j < wp.points_.size(); j++) {
						QPoint t = wp.mappingPoint(last[i], wp.matrix_, j);
						QColor c = image_temp_->pixelColor(wp.points_[j]);
						image_->setPixelColor(t, c);
						mappingPoints.push_back(t);
						color.push_back(c);
					}
					FillHole(mappingPoints, last[i], color);
				}
			}
			else if (2 == warping_method_) {
				for (int i = 0; i < last.size(); i++) {
					mappingPoints.clear();
					color.clear();
					MV mv(origon[i]);
					for (int j = 0; j < mv.points_.size(); j++) {
						QPoint t = mv.mappingPoint(last[i], mv.matrix_, j);
						QColor c = image_temp_->pixelColor(mv.points_[j]);
						image_->setPixelColor(t, c);
						mappingPoints.push_back(t);
						color.push_back(c);
					}
					FillHole(mappingPoints, last[i], color);
				}
			}
			else if (3 == warping_method_) {
				for (int i = 0; i < last.size(); i++) {
					mappingPoints.clear();
					color.clear();
					DH dh(origon[i]);
					for (int j = 0; j < dh.points_.size(); j++) {
						QPoint t = dh.mappingPoint(last[i], dh.matrix_, j);
						QColor c = image_temp_->pixelColor(dh.points_[j]);
						image_->setPixelColor(t, c);
						mappingPoints.push_back(t);
						color.push_back(c);
					}
					FillHole(mappingPoints, last[i], color);
				}
			}
			//FillHole(mappingPoints, , color);
		}
		else if (is_choosing_&&figure_type_ == 1) {
			points_[2] = mouseEvent->pos();
			points_[3] = QPoint(points_[2].x(), points_[0].y());
			points_[1] = QPoint(points_[0].x(), points_[2].y());
		}
		break;
	}

	default:
		break;
	}

	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
	switch (draw_status_)
	{
	case kChoose:
		if (is_choosing_)
		{
			point_end_ = mouseEvent->pos();
			is_choosing_ = false;
			break;
		}

	case kPaste:
		if (is_pasting_)
		{
			is_pasting_ = false;
			break;
		}

	case kWarp:
		if (is_choosing_&&figure_type_ == 1)
		{
			points_[2] = mouseEvent->pos();
			points_[3] = QPoint(points_[2].x(), points_[0].y());
			points_[1] = QPoint(points_[0].x(), points_[2].y());
			is_choosing_ = false;
			origon_points = points_;
			getTriangles();
		}
		else if (!is_choosing_&&figure_type_ == 1) {
			index_ = -1;
		}
		else if (is_choosing_ && !is_drawing_&&figure_type_ == 2) {
			is_choosing_ = false;
			index_ = -1;
		}
		else if (is_choosing_ &&figure_type_ == 2) {
			is_choosing_ = false;
			index_ = -1;
		}
		if (warping_state_ == 2) {
			index_ = -1;
			warping_state_ = 1;
			is_moving_ = false;
		}
		//*(image_temp_) = *(image_);
		update();
		return;
		/*else {
			is_warping_ = false;
		}*/

	default:
		break;
	}
	draw_status_ = kNone;
	*(image_temp_) = *(image_);
	update();
}

void ImageWidget::Open(QString fileName)
{
	// Load file
	if (!fileName.isEmpty())
	{
		image_->load(fileName);
		*(image_backup_) = *(image_);
		*(image_temp_) = *(image_);
	}

	//	setFixedSize(image_->width(), image_->height());
	//	relate_window_->setWindowFlags(Qt::Dialog);
	//	relate_window_->setFixedSize(QSize(image_->width(), image_->height()));
	//	relate_window_->setWindowFlags(Qt::SubWindow);

		//image_->invertPixels(QImage::InvertRgb);
		//*(image_) = image_->mirrored(true, true);
		//*(image_) = image_->rgbSwapped();
	cout << "image size: " << image_->width() << ' ' << image_->height() << endl;

	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (fileName.isNull())
	{
		return;
	}

	image_->save(fileName);
}

void ImageWidget::Invert()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}

	// equivalent member function of class QImage
	// image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(image_));
	int width = image_->width();
	int height = image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, height - 1 - j));
				}
			}
		}
		else
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
				}
			}
		}

	}
	else
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(image_) = image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
			image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
		}
	}

	update();
}

void ImageWidget::Restore()
{
	*(image_) = *(image_backup_);
	*(image_temp_) = *(image_backup_);
	point_start_ = point_end_ = QPoint(0, 0);
	draw_status_ = kNone;
	is_choosing_ = false;
	is_drawing_ = false;
	is_moving_ = false;
	warping_method_ = 0;
	warping_state_ = 0;
	points_.clear();
	origon.clear();
	last.clear();
	update();
}
