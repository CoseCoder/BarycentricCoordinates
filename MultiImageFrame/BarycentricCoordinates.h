#pragma once
#include<QVector>
#include <QPoint>
#include<QPolygon>
class BarycentricCoordinates
{
public:
	BarycentricCoordinates() {}

	void scanLine(QVector<QPoint>& vertex, QVector<QPoint> &points) {
		QPolygon polygon(vertex);
		QRect rect = polygon.boundingRect();

		int minY = rect.y();
		int maxY = minY + rect.height();

		for (int y = minY; y < maxY; y++) {
			for (int x = rect.x(); x < rect.x() + rect.width(); x++)
			{
				QPoint t(x, y);
				if (polygon.containsPoint(t, Qt::WindingFill))
					points.append(t);
			}
		}
	}

	double computeCosX(const QPoint &p0, const QPoint &p1, const QPoint &p2) const {
		QVector<QPoint> triangle = { p0,p1,p2 };
		double cosX = (computeSquareOfDistance(triangle[0], triangle[1]) + computeSquareOfDistance(triangle[0], triangle[2]) - computeSquareOfDistance(triangle[1], triangle[2])) / (2.0*computeDistance(triangle[0], triangle[1])*computeDistance(triangle[0], triangle[2]));
		return cosX;
	}

	double computeSinX(double cosX) const {
		return sqrt(1 - cosX*cosX);
	}

	double computeSinX(const QPoint &p0, const QPoint &p1, const QPoint &p2) const {
		double cosX = computeCosX(p0, p1, p2);
		return computeSinX(cosX);
	}

	double computeTanX(const QPoint &p0, const QPoint &p1, const QPoint &p2) const {
		double cosX = computeCosX(p0, p1, p2);
		return computeSinX(cosX) / cosX;
	}

	double computeCotX(const QPoint &p0, const QPoint &p1, const QPoint &p2) const {
		double cosX = computeCosX(p0, p1, p2);
		return cosX / computeSinX(cosX);
	}

	double computeSquareOfDistance(const QPoint &p1, const QPoint &p2) const {
		return (p1.x() - p2.x())*(p1.x() - p2.x()) + (p1.y() - p2.y())*(p1.y() - p2.y());
	}

	double computeDistance(const QPoint &p1, const QPoint &p2) const {
		return sqrt(computeSquareOfDistance(p1, p2));
	}

	virtual double computeWi(QPoint v, int i) = 0;

	void computeW(QPoint v, int i, QVector<QPoint> &vertex, QVector<QVector<double>> &matrix) {
		double sum = 0.0;

		for (int j = 0; j < vertex.size(); j++) {
			matrix[i][j] = computeWi(v, j);
			sum += matrix[i][j];
		}

		for (int j = 0; j < vertex.size(); j++) {
			matrix[i][j] /= sum;
		}
	}

	void computeMatrix(QVector<QPoint> &points, QVector<QPoint> &vertex, QVector<QVector<double>> &matrix) {
		for (int i = 0; i < points.size(); i++)
			computeW(points[i], i, vertex, matrix);
	}

	QPoint mappingPoint(QVector<QPoint> &newVertex, QVector<QVector<double>> &matrix, int k) {
		double x = 0.0, y = 0.0;
		for (int i = 0; i < newVertex.size(); i++) {
			x += matrix[k][i] * newVertex[i].x();
			y += matrix[k][i] * newVertex[i].y();
		}
		return QPoint(x, y);
	}

	QVector<QPoint> points_;
	QVector<QPoint> vertex_;
	QVector<QVector<double>> matrix_;
};