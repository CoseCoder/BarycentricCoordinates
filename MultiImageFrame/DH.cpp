#include "DH.h"

DH::DH()
{
}

DH::DH(QVector<QPoint> &vertexs) {
	vertex_ = vertexs;
	//points_ = points;
	scanLine(vertex_, points_);
	matrix_ = QVector<QVector<double>>(points_.size(), QVector<double>(vertex_.size()));
	computeMatrix(points_, vertex_, matrix_);
}
double DH::computeWi(QPoint v, int i)
{
	int l = i - 1 < 0 ? vertex_.size() - 1 : i - 1,
		r = (i + 1) % vertex_.size();
	return computeCotX(vertex_[l], vertex_[i], v) + computeCotX(vertex_[r], vertex_[i], v);
}
