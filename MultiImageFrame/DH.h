#pragma once
#include <BarycentricCoordinates.h>
class DH :public BarycentricCoordinates {
public:
	DH();
	DH(QVector<QPoint> &vertexs);
	double computeWi(QPoint v, int i);

	QVector<QPoint> points_;
	QVector<QVector<double>> matrix_;
	QVector<QPoint> vertex_;
};
