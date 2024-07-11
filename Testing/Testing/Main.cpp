// GeoMatchNew.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

//#include "pch.h"
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include "GeoMatch.h"

using namespace std;
using namespace cv;

class CommandParser
{
    char** argList;        //point to hold argument list
    int         numArgs;        // no of arguments in the list
public:
    CommandParser(void);
    CommandParser(int, char**);
    ~CommandParser(void);
    char* GetParameter(const char* key);
};

// default constructor
CommandParser::CommandParser(void)
{
    numArgs = 0;
    argList = NULL;
}

CommandParser::~CommandParser(void)
{
}
//constructor to initialize with number of argument and argument list
CommandParser::CommandParser(int _numArgs, char** _argList)
{
    numArgs = _numArgs;
    argList = _argList;
}

//return argument that corresponds to the key
char* CommandParser::GetParameter(const char* key)
{
    for (int currArg = 0; currArg < numArgs; currArg++)
    {
        if (strcmp(key, argList[currArg]) == 0)
            return argList[currArg + 1];
    }

    return NULL;
}


void WrongUsage()
{
    cout << "\n Edge Based Template Matching Program\n";
    cout << " ------------------------------------\n";
    cout << "\nProgram arguments:\n\n";
    cout << "     -t Template image name (image to be searched)\n\n";
    cout << "     -h High Threshold (High threshold for creating template model)\n\n";
    cout << "     -l Low Threshold (Low threshold for creating template model)\n\n";
    cout << "     -s Search image name (image we are trying to find)\n\n";
    cout << "     -m Minimum score (Minimum score required to proceed with search [0.0 to 1.0])\n\n";
    cout << "     -g greediness (heuristic parameter to terminate search [0.0 to 1.0] )\n\n";

    cout << "Example: GeoMatch -t Template.jpg -h 100 -l 10 -s Search1.jpg -m 0.7 -g 0.5 \n\n";
}

void displayImages(const cv::Mat& templateImage, const cv::Mat& searchImage, bool matchFound)
{
    try {
        if (!templateImage.empty()) {
            if (matchFound) {
                namedWindow("Template with Contours", WINDOW_AUTOSIZE);
                imshow("Template with Contours", templateImage);
            }
            else {
                namedWindow("Template", WINDOW_AUTOSIZE);
                imshow("Template", templateImage);
            }
        }
        else {
            std::cerr << "Template image is empty!" << std::endl;
        }

        if (!searchImage.empty()) {
            namedWindow("Search Image", WINDOW_AUTOSIZE);
            imshow("Search Image", searchImage);
        }
        else {
            std::cerr << "Search image is empty!" << std::endl;
        }
    }
    catch (const cv::Exception& e) {
        std::cerr << "OpenCV error: " << e.what() << std::endl;
    }

    std::cout << "\nPress any key to exit!";
    waitKey(0);
    destroyAllWindows();
}

int main(int argc, char** argv)
{
    void WrongUsage();
    CommandParser cp(argc, argv);

    GeoMatch GM;                // object to implement geometric matching    
    int lowThreshold = 10;      //default value
    int highThreshold = 100;    //default value

    double minScore = 0.7;      //default value
    double greediness = 0.8;        //default value

    double total_time = 0;
    double score = 0;

    Point result;

    //Load Template image 
    char* param;
    param = cp.GetParameter("-t");
    if (param == NULL)
    {
        cout << "ERROR: Template image argument missing";
        WrongUsage();
        return -1;
    }

    Mat templateImage = imread(param, IMREAD_GRAYSCALE);
    if (templateImage.data == NULL)
    {
        cout << "\nERROR: Could not load Template Image.\n" << param;
        return 0;
    }

    param = cp.GetParameter("-s");
    if (param == NULL)
    {
        cout << "ERROR: source image argument missing";
        WrongUsage();
        return -1;
    }

    //Load Search Image
    Mat searchImage = imread(param, IMREAD_GRAYSCALE);
    if (searchImage.data == NULL)
    {
        cout << "\nERROR: Could not load Search Image." << param;
        return 0;
    }

    param = cp.GetParameter("-l"); //get Low threshold
    if (param != NULL)
        lowThreshold = atoi(param);

    param = cp.GetParameter("-h");
    if (param != NULL)
        highThreshold = atoi(param);//get high threshold

    param = cp.GetParameter("-m"); // get minimum score
    if (param != NULL)
        minScore = atof(param);

    param = cp.GetParameter("-g");//get greediness
    if (param != NULL)
        greediness = atof(param);

    //Size templateSize = ; //Size(templateImage.width, templateImage->height);
    Mat grayTemplateImg;  //(templateImage.size(), CV_8U, 1);

    templateImage.copyTo(grayTemplateImg);

    cout << "\n Edge Based Template Matching Program\n";
    cout << " ------------------------------------\n";


    if (!GM.CreateGeoMatchModel(grayTemplateImg, lowThreshold, highThreshold))
    {
        cout << "ERROR: could not create model...";
        return 0;
    }


    cout << " Shape model created.." << "with  Low Threshold = " << lowThreshold << " High Threshold = " << highThreshold << endl;


    //CvSize searchSize = cvSize(searchImage->width, searchImage->height);
    Mat graySearchImg; //= cvCreateImage(searchSize, IPL_DEPTH_8U, 1);

    // Convert color image to gray image. 
    searchImage.copyTo(graySearchImg);

    cout << " Finding Shape Model.." << " Minimum Score = " << minScore << " Greediness = " << greediness << "\n";
    cout << " ------------------------------------\n";

    clock_t start_time1 = clock();
    score = GM.FindGeoMatchModel(graySearchImg, minScore, greediness, &result);
    clock_t finish_time1 = clock();
    total_time = ((double)finish_time1 - (double)start_time1) / CLOCKS_PER_SEC;

    Mat rgb;
    bool matchFound = false;
    if (score > minScore) // if score is at least 0.4
    {
        matchFound = true;
        cout << " Found at [" << result.x << ", " << result.y << "]\n Score = " << score << "\n Searching Time = " << total_time * 1000 << "ms";

        cvtColor(searchImage, rgb, COLOR_GRAY2BGR);
        GM.DrawContours(rgb, result, Scalar(0, 255, 0), 1);
    }
    else {
        cout << " Object Not found";
        searchImage.copyTo(rgb); // Copy original search image to rgb
    }

    //////////////////////////////////

    cout << "\n ------------------------------------\n\n";

    //Display result
    Mat dispTemplate;
    cvtColor(templateImage, dispTemplate, COLOR_GRAY2BGR);
    GM.DrawContours(dispTemplate, CV_RGB(255, 0, 0), 1);

    displayImages(dispTemplate, rgb, matchFound);

    templateImage.release();
    grayTemplateImg.release();
    searchImage.release();
    graySearchImg.release();

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
