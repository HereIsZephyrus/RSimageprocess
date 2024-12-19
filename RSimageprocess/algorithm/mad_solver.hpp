//
//  mad_solver.hpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/18/24.
//

#ifndef mad_solver_hpp
#define mad_solver_hpp
#define NODATA -999
#include <Eigen/Dense>
using namespace Eigen;
class MADSolver{
    using dataMat = std::vector<std::vector<double>>;
public:
    static MADSolver& getSolver(){
        static MADSolver instance;
        return instance;
    }
    MADSolver(const MADSolver&) = delete;
    void operator = (const MADSolver) = delete;
    int bandNum;
    std::vector<double> rho;
    std::vector<VectorXd> A,B;
    std::vector<dataMat> dataVec;
    int showIndex;
    dataMat Z;
    std::vector<std::vector<bool>> changed;
    void calcInitMAD(const MatrixXd& convXX,const MatrixXd& convXY,const MatrixXd& convYY);
    void decreaseIndex(){if (showIndex > 0)  --showIndex;}
    void increaseIndex(){if (showIndex < bandNum)    ++showIndex;}
    void calcChangeSignal();
private:
    MatrixXd calcMatrixPowerNegHalf(const MatrixXd& conv);
    dataMat ChiSquareProb;
    MADSolver(){
        //range N from 1 to 20: 0.1,0.05,0.025,0.01,0.005
        ChiSquareProb = {
            {2.706,3.841,5.024,6.635,7.879},
            {4.605,5.991,7.378,9.210,10.597},
            {6.251,7.815,9.348,11.345,12.838},
            {7.779,9.488,11.143,13.277,14.860},
            {9.236,11.070,12.833,15.086,16.750},
            {10.645,12.592,14.449,16.812,18.548},
            {12.017,14.067,16.013,18.475,20.278},
            {13.362,15.507,17.535,20.090,21.955},
            {14.684,16.919,19.023,21.666,23.589},
            {15.987,18.307,20.483,23.209,25.188},
            {17.275,19.675,21.920,24.725,26.757},
            {18.549,21.026,23.337,26.217,28.300},
            {19.812,22.362,24.736,27.688,29.819},
            {21.064,23.685,26.119,29.141,31.319},
            {22.307,24.996,27.488,30.578,32.801},
            {23.542,26.296,28.845,32.000,34.267},
            {24.769,27.587,30.191,33.409,35.718},
            {25.989,28.869,31.526,34.805,37.156},
            {27.204,30.144,32.852,36.191,38.582},
            {28.412,31.410,34.170,37.566,39.997}};
    }
};
#endif /* mad_solver_hpp */
