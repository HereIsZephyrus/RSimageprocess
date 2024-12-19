//
//  mad_solver.hpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/18/24.
//

#ifndef mad_solver_hpp
#define mad_solver_hpp

#include <Eigen/Dense>
using namespace Eigen;
class MADSolver{
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
    int showIndex;
    void calcInitMAD(const MatrixXd& convXX,const MatrixXd& convXY,const MatrixXd& convYY);
    void decreaseIndex(){if (showIndex > 0)  --showIndex;}
    void increaseIndex(){if (showIndex < bandNum - 1)    ++showIndex;}
private:
    MatrixXd calcMatrixPowerNegHalf(const MatrixXd& conv);
    MADSolver(){}
};
#endif /* mad_solver_hpp */
