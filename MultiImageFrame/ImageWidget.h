#pragma once
#include <QWidget>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>  
#include <opencv2/ml/ml.hpp>
#include <iostream>
#include <vector>
using std::vector;
using cv::Point3i;


class ChildWindow;
QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

enum DrawStatus
{
	kChoose,
	kPaste,
	kWarp,
	kNone,
};

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(ChildWindow *relateWindow);
	~ImageWidget(void);

	int ImageWidth();											// Width of image
	int ImageHeight();											// Height of image
	void set_draw_status_to_choose();
	void set_draw_status_to_paste();
	void set_draw_status_to_warp(int index);
	void set_draw_figure(int index);
	QImage* image();
	void set_source_window(ChildWindow* childWindow);
	void getTriangles();
	void FillHole(QVector<QPoint> &mappingPoints, QVector<QPoint> &vertex, QVector<QColor> &color);


protected:
	void paintEvent(QPaintEvent *paintEvent);
	void mousePressEvent(QMouseEvent *mouseEvent);
	void mouseMoveEvent(QMouseEvent *mouseEvent);
	void mouseReleaseEvent(QMouseEvent *mouseEvent);

	public slots:
	// File IO
	void Open(QString filename);								// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal = false, bool vertical = true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin

public:
	QPoint						point_start_;					// Left top point of rectangle region
	QPoint						point_end_;						// Right bottom point of rectangle region
	QVector<QPoint>				points_;
	QVector<QPoint>				origon_points;
	int							index_;
	QVector<QVector<QPoint>>	origon;
	QVector<QVector<QPoint>>	last;

private:
	QImage						*image_;						// image 
	QImage						*image_backup_;
	QImage						*image_temp_;

	// Pointer of child window
	ChildWindow					*source_window_;				// Source child window

	// Signs
	DrawStatus					draw_status_;					// Enum type of draw status
	bool						is_choosing_;
	bool						is_pasting_;
	bool						is_drawing_;
	bool						is_moving_;
	int							warping_state_;
	int							warping_method_;
	int							figure_type_;
};

