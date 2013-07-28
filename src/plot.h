#pragma once

#include <string>
#include <iostream>
#include <vector>
#include "config.h"
using namespace std;

#ifdef _WIN32
	const string gnuPlot = "C:\\opt\\gnuplot\\bin\\gnuplot.exe -persist" ;
#elif  __linux__
	const string gnuPlot = "/usr/bin/gnuplot" ;
#elif  __APPLE__
	//const string gnuPlot = "/Applications/Gnuplot.app/Contents/Resources/bin/gnuplot" ; //Old gnuplot path (v4.0) on my Mac, does not support drawing a polygin, on aqua.
	const string gnuPlot = "/usr/local/bin/gnuplot"; //current gnuPlot path, v4.6 on x11
#else
	const string gnuPlot = "/usr/local/bin/gnuplot";
#error Undefined operating system/Unknown gnuplot path
#endif

//


class Plotter{
    int dim;
    string gnuPlotPath ;
    FILE *gnuplotPipe;
    bool closed;
   
    //EmptyPlots are called in the constructor
    void emptyPlot(string title, double xmin, double xmax, double ymin, double ymax, double zmin, double zmax, int angleX, int angleY, string xlabel, string ylabel,string zlabel);
    void emptyPlot(string title, double xmin, double xmax, double ymin, double ymax );
    
public:
    //creates empty 2D or 3D plot, the path to gnuPlot must be provided
    Plotter(string path, Configuration*);
    ~Plotter();
    void execute(string str);
    
    void drawArray(vector< vector<double> > trace, int, int);
    void drawLine(double iFromX, double iFromY, double iToX, double iToY);
    void drawLine(double iFromX, double iFromY, double iFromZ, double iToX, double iToY, double iToZ);
    void saveToPdf(string path);
    void waitForKey();
    void close();
};

