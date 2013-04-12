/**
 * @author Yuncheng Li (raingomm[AT]gmail.com)
 * @version 2013/04/12
 */


#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>

using namespace std;
using namespace cv;

struct params {
    int thresh, l1, l2, disk0;
    double ratio;

    params()
    {
        thresh = 50;
        l1 = 10;
        l2 = 10;
        disk0 = 5;
        ratio = 1.5;
    }

    void write(FileStorage &fs) const
    {
        fs << "{" << "brightness_thresh" << thresh
            << "width_thresh" << l1
            << "height_thresh" << l2
            << "thick_thresh" << disk0
            << "area_ratio" << ratio
            << "}";
    }

    void read(const FileNode &node)
    {
        thresh = (int)node["brightness_thresh"];
        l1 = (int)node["width_thresh"];
        l2 = (int)node["height_thresh"];
        disk0 = (int)node["thick_thresh"];
        ratio = (double)node["area_ratio"];
    }
};

inline void write(FileStorage& fs, const std::string&, const params& x)
{
    x.write(fs);
}

inline void read(const FileNode& node, params &x, const params & default_value = params())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

Mat filtering(const Mat &_bgrImg, const params &param)
{
    Mat se1 = getStructuringElement( MORPH_RECT, Size( param.l1, 1 ), Point( param.l1 >> 1, 0) );
    Mat se2 = getStructuringElement( MORPH_RECT, Size( 1, param.l2), Point(0, param.l2 >> 1) );
    Mat se3 = getStructuringElement( MORPH_RECT, Size( param.disk0, param.disk0), Point(param.disk0 >> 1, param.disk0 >> 1) );

    vector<Mat> bgrSplit;
    split(_bgrImg, bgrSplit);
    Mat mask = Mat::ones(_bgrImg.size(), CV_8UC1);

    for (int i = 0; i < 3; i++) {
        Mat threshed;
        threshold(bgrSplit[i], threshed, param.thresh, 1,THRESH_BINARY_INV);
        bitwise_and(threshed, mask, mask);
    }

    Mat mask1;
    Mat mask2;
    Mat mask3;

    morphologyEx(mask, mask1, MORPH_OPEN, se1);
    morphologyEx(mask, mask2, MORPH_OPEN, se2);
    morphologyEx(mask, mask3, MORPH_OPEN, se3);

    //imwrite("mask1.bmp", mask1 * 255);
    //imwrite("mask2.bmp", mask2 * 255);
    //imwrite("mask3.bmp", mask3 * 255);

    bitwise_or(mask1, mask2, mask);
    bitwise_not(mask3, mask3);
    bitwise_and(mask, mask3, mask);

    return mask;
}

static double upright(Point pt1, Point pt2)
{
    double dx = pt1.x - pt2.x;
    double dy = pt1.y - pt2.y;

    double maxd = std::min(fabs(dx), fabs(dy));

    return maxd / sqrt(dx*dx + dy*dy);
}

static double angle( Point pt1, Point pt2, Point pt0 )
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


Rect detectRect(Mat &map, const params &param)
{
    vector<vector<Point> > contours;
    findContours(map, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    Rect bestRect;
    double bestRatio = std::numeric_limits<double>::max();

    for (size_t i = 0; i < contours.size(); ++i)
    {
        Rect tmp = boundingRect(contours[i]);

        double area1 = tmp.width * tmp.height;

        vector<Point> cvxhull;
        convexHull(contours[i], cvxhull);
        double area2 = contourArea(cvxhull);

        double ratio = area1 / (area2 + 1e-10);

        if (bestRatio > ratio)
        {
            bestRect = tmp;
            bestRatio = ratio;
        }
    }

    if (bestRatio < param.ratio)
        return bestRect;
    else
        return Rect();
}

int main(int argc, const char *argv[])
{

    if (argc < 2)
    {
        cerr << argv[0] << " img-filename" << endl;
        return 0;
    }

    Mat img = imread(argv[1]);

    if (img.empty())
    {
        cerr << argv[1] << " is not a valid image file" << endl;
        return 0;
    }

    params m_param;
    FileStorage fs("config.xml", FileStorage::READ);
    fs["params"] >> m_param;

    Mat I = filtering(img, m_param);

    Rect rect = detectRect(I, m_param);

    if (rect.width != 0 && rect.height != 0)
        cout << rect.x << ' ' << rect.y << ' ' << rect.width << ' ' << rect.height << endl;

    return 0;
}
