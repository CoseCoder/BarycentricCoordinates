#pragma once
#include <BarycentricCoordinates.h>
class MV :public BarycentricCoordinates {
public:
	MV();
	MV(QVector<QPoint> &vertexs);
	double computeWi(QPoint v, int i);

	QVector<QPoint> points_;
	QVector<QVector<double>> matrix_;
	QVector<QPoint> vertex_;
};