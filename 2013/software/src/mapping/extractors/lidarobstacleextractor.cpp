#include "lidarobstacleextractor.h"
#include <algorithm>
#include <iostream>
#include "mapping/obstacles/linearobstacle.h"
#include "mapping/obstacles/circularobstacle.h"
#include <eigen3/Eigen/Geometry>

namespace IGVC
{
namespace mapping
{
namespace extractors
{

LidarObstacleExtractor::LidarObstacleExtractor(Lidar *device)
    : LonNewLidarData(this)
{
    if(device)
    {
        device->onNewData += &LonNewLidarData;
        _device = device;
    }
    jumpDistThreshold = 0.1;
    linearityThreshold = 0.75;
    sizeThreshold = 10;
}

void LidarObstacleExtractor::disconnect()
{
    if(_device)
    {
        _device->onNewData -= &LonNewLidarData;
        _device = 0;
    }
}

void LidarObstacleExtractor::onNewLidarData(LidarState data)
{
    using namespace std;

    vector<Obstacle *> obstacles;

    vector<Obstacle*> filtered = filterCircularObstacles(filterByLinearRegression(filterBySize(clusterByJumps(clusterByValidity(&data)))));
    obstacles.insert(obstacles.end(), filtered.begin(), filtered.end());

    onNewData(obstacles);
}

std::vector<PointArrayObstacle*> LidarObstacleExtractor::clusterByValidity(LidarState *data)
{
    using namespace std;

    vector<PointArrayObstacle*> clusters;

    bool inCluster = false;

    PointArrayObstacle *cluster;

    for(int i = 0; i < 1024; i++)
    {
        LidarPoint &pt = data->points[i];

        float x = cos(pt.angle)*pt.distance;
        float y = sin(pt.angle)*pt.distance;

        if(pt.valid)
        {
            if(!inCluster)
            {
                clusters.push_back(new PointArrayObstacle());
                cluster = clusters.at(clusters.size()-1);
                cluster->addPoint(Vector2f(x,y));
                inCluster = true;
            } else {
                cluster->addPoint(Vector2f(x,y));
            }
        } else {
            if(inCluster)
            {
                inCluster = false;
            }
        }
    }

    return clusters;
}

std::vector<PointArrayObstacle*> LidarObstacleExtractor::clusterByJumps(std::vector<PointArrayObstacle*> data)
{
    using namespace std;
    vector<PointArrayObstacle*> clusters;

    bool inCluster = false;

    PointArrayObstacle *newcluster;

    for(vector<PointArrayObstacle*>::iterator iter = data.begin(); iter < data.end(); iter++)
    {
        PointArrayObstacle* cluster = (*iter);
        Vector2f* points = cluster->getPoints();
        for(int i = 0; i < cluster->getNumPoints()-1; i++)
        {
            if(!inCluster)
            {
                clusters.push_back(new PointArrayObstacle());
                newcluster = clusters.at(clusters.size()-1);
                newcluster->addPoint(points[i]);
                inCluster = true;
            } else {
                //float dist = points[i].distanceTo(points[i+1]);
                float dist = (points[i+1] - points[i]).norm();
                newcluster->addPoint(points[i]);
                if(dist >= jumpDistThreshold)
                {
                    // Jump detected, break cluster
                    inCluster = false;
                }
            }
        }
        if(inCluster)
            newcluster->addPoint(points[cluster->getNumPoints()-1]);
        inCluster = false;
    }
    return clusters;
}

std::vector<PointArrayObstacle*> LidarObstacleExtractor::filterBySize(std::vector<PointArrayObstacle*> data)
{
    using namespace std;
    vector<PointArrayObstacle*> clusters;

    for(vector<PointArrayObstacle*>::iterator iter = data.begin(); iter < data.end(); iter++)
    {
        PointArrayObstacle* cluster = (*iter);
        if(cluster->getNumPoints() > sizeThreshold)
        {
            clusters.push_back(cluster);
        }
    }

    return clusters;
}

float LidarObstacleExtractor::calculateLinearCorrelationCoefficient(Vector2f *points, int numPoints)
{
    // Compute averages
    float avgX = 0;
    float avgY = 0;
    for(int i = 0; i < numPoints; i++)
    {
        avgX += points[i][0];
        avgY += points[i][1];
    }
    avgX /= numPoints;
    avgY /= numPoints;

    // Compute variances and covariances of X and Y
    float SSxx = 0;
    float SSyy = 0;
    float SSxy = 0;
    for(int i = 0; i < numPoints; i++)
    {
        float xi = points[i][0];
        float yi = points[i][1];
        SSxx += (xi - avgX)*(xi - avgX);
        SSyy += (yi - avgY)*(yi - avgY);
        SSxy += (xi - avgX)*(yi - avgY);
    }
    return ( SSxy * SSxy ) / ( SSxx * SSyy );
}

Vector2f LidarObstacleExtractor::closestPointToLineFromPoint(Vector2f &lineCoefs, Vector2f &p)
{
    Vector2f A = Vector2f(0,lineCoefs[0]); // Point on line @ x = 0
    Vector2f B = Vector2f(1,lineCoefs[1] + lineCoefs[0]); // Point on line @ x = 1
    Vector2f AP = p - A;
    Vector2f AB = B - A;
    float t = AP.dot(AB) / AB.squaredNorm();
    return A + AB*t;
}

std::vector<Obstacle*> LidarObstacleExtractor::filterByLinearRegression(std::vector<PointArrayObstacle*> data)
{
    using namespace std;

    std::vector<Obstacle*> obstacles;

    for(vector<PointArrayObstacle*>::iterator iter = data.begin(); iter != data.end(); iter++)
    {
        PointArrayObstacle* cluster = (*iter);
        Vector2f *points = cluster->getPoints();

        // Filter non-linear obstacles via correlation coefficient
        float rsquared = calculateLinearCorrelationCoefficient(points, cluster->getNumPoints());
        if(rsquared >= linearityThreshold)
        {
            MatrixXf A(cluster->getNumPoints(), 2);
            for(int i = 0; i < cluster->getNumPoints(); i++)
            {
                A(i,0) = 1;
                A(i,1) = points[i][0]; // X-Values
            }
            MatrixXf b(cluster->getNumPoints(),1);
            for(int i = 0; i < cluster->getNumPoints(); i++)
            {
                b(i,0) = points[i][1]; // Y-Values
            }

            /*
             * Solve Az=b to estimate y=mx+c where m=z[1] and c = z[0]
             */
            Vector2f z = A.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);

            Vector2f start = closestPointToLineFromPoint(z, points[0]);
            Vector2f end = closestPointToLineFromPoint(z, points[cluster->getNumPoints()-1]);

            obstacles.push_back(new LinearObstacle(start, end));
        } else {
            obstacles.push_back(cluster);
        }
    }

    return obstacles;
}

std::vector<Obstacle*> LidarObstacleExtractor::filterCircularObstacles(std::vector<Obstacle*> data)
{
    using namespace std;
    vector<Obstacle*> filtered;

    for(vector<Obstacle*>::iterator iter = data.begin(); iter != data.end(); iter++)
    {
        Obstacle* obst = (*iter);
        PointArrayObstacle* raw = dynamic_cast<PointArrayObstacle*>(obst);
        if(raw != 0)
        {
            // This obstacle has yet to be classified, check for circularity

            Vector2f center(0.0f, 0.0f);
            Vector2f *points = raw->getPoints();
            int q = 0;
            for(int ind = 0; ind < raw->getNumPoints()-2; ind++)
            {
                Vector2f Pi = points[ind], Pj = points[ind + 1], Pk = points[ind + 2];
                Vector2f circumcenter;
                float delta = (Pk[0]-Pj[0])*(Pj[1]-Pi[1])-(Pj[0]-Pi[0])*(Pk[1]-Pj[1]);
                if(delta != 0) // If this triplet does not form a line
                {
//                    circumcenter[0] = ( (Pk[1]-Pj[1])*Pi.squaredNorm()+(Pi[1]-Pk[1])*Pj.squaredNorm()+(Pj[1]-Pi[1])*Pk.squaredNorm() ) / delta;
//                    circumcenter[1] = - ( (Pk[0]-Pj[0])*Pi.squaredNorm()+(Pi[0]-Pk[0])*Pj.squaredNorm()+(Pj[0]-Pi[0])*Pk.squaredNorm() ) / delta;

                    Vector2f PiPj = Pj - Pi;
                    Vector2f V1 = Vector2f(PiPj[1], - PiPj[0]).normalized();
                    Vector2f O1 = ( Pi + Pj ) / 2.0;

                    Vector2f PjPk = Pk - Pj;
                    Vector2f V2 = Vector2f(PjPk[1], - PjPk[0]).normalized();
                    Vector2f O2 = ( Pj + Pk ) / 2.0;

                    MatrixXf A(2,2);
                    for(int i = 0; i < 2; i++)
                    {
                        A(i,0) = V1[i];
                        A(i,1) = V2[i];
                    }
                    Vector2f b = O2 - O1;

                    Vector2f z = A.jacobiSvd(ComputeThinU | ComputeThinV).solve(b);

                    circumcenter = O1 + z[0] * V1;

                    center += circumcenter;
                    q++;
                }
            }
            if(q != 0) // If at least one triplet was not a line...
                center /= q;

            float rhat = 0;
            float rmin = (points[0]-center).norm();
            float rmax = (points[0]-center).norm();
            for(int i = 0; i < raw->getNumPoints(); i++)
            {
                float r = (points[i] - center).norm();
                rhat += r;
                rmin = min(r, rmin);
                rmax = max(r, rmax);
            }
            rhat /= raw->getNumPoints();

            float rdeviance = rmax - rmin;

            if(rdeviance < 0.1)
                filtered.push_back(new CircularObstacle(center, rhat, 32));
            else
                filtered.push_back(obst);

        } else {
            // This obstacle has already been classified, ignore it
            filtered.push_back(obst);
        }
    }

    return filtered;
}

}
}
}