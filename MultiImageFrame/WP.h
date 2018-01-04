#pragma once
#include <BarycentricCoordinates.h>
class WP :public BarycentricCoordinates {
public:
	WP();
	WP(QVector<QPoint> &vertexs);
	double computeWi(QPoint v, int i);

	QVector<QPoint> points_;
	QVector<QVector<double>> matrix_;
	QVector<QPoint> vertex_;
};