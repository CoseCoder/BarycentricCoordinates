#include "WP.h"

WP::WP()
{

}

WP::WP(QVector<QPoint> &vertexs) {
	vertex_ = vertexs;
	//points_ = points;
	scanLine(vertex_, points_);
	matrix_ = QVector<QVector<double>>(points_.size(), QVector<double>(vertex_.size()));
	computeMatrix(points_, vertex_, matrix_);
}


double WP::computeWi(QPoint v, int i)
{
	int l = i - 1 < 0 ? vertex_.size() - 1 : i - 1,
		r = (i + 1) % vertex_.size();

	double t1 = computeCotX(vertex_[i], vertex_[l], v),
		t2 = computeCotX(vertex_[i], vertex_[r], v);

	return (t1 + t2) / computeSquareOfDistance(vertex_[i], v);
}

