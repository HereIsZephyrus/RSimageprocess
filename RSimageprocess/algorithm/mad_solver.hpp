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
    double maxSingularValue;
    VectorXd leftSingularVector;
    VectorXd rightSingularVector;
private:
    
    MADSolver(){}
};
#endif /* mad_solver_hpp */
