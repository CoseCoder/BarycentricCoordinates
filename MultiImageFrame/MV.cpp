#include "MV.h"

MV::MV()
{
}

MV::MV(QVector<QPoint> &vertexs) {
	vertex_ = vertexs;
	//points_ = points;
	scanLine(vertex_, points_);
	matrix_ = QVector<QVector<double>>(points_.size(), QVector<double>(vertex_.size()));
	computeMatrix(points_, vertex_, matrix_);
}

double MV::computeWi(QPoint v, int i)
{
	int l = i - 1 < 0 ? vertex_.size() - 1 : i - 1,
		r = (i + 1) % vertex_.size();
	double cosX1 = computeCosX(v, vertex_[l], vertex_[i]),
		cosX2 = computeCosX(v, vertex_[i], vertex_[r]);
	double sinX1 = computeSinX(cosX1),
		sinX2 = computeSinX(cosX2);
	double t1 = (1 - cosX1) / sinX1,
		t2 = (1 - cosX2) / sinX2;
	return (t1 + t2) / computeDistance(vertex_[i], v);
}

