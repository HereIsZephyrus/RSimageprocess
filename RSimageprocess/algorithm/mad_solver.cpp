//
//  mad_solver.cpp
//  RSimageprocess
//
//  Created by ChanningTong on 12/18/24.
//

#include "mad_solver.hpp"

void MADSolver::calcInitMAD(const MatrixXd& convXX,const MatrixXd& convXY,const MatrixXd& convYY){
    MatrixXd revConvXX = calcMatrixPowerNegHalf(convXX),revConvYY = calcMatrixPowerNegHalf(convYY);
    MatrixXd M = revConvXX * convXY * revConvYY;
    JacobiSVD<MatrixXd> svd(M, ComputeThinU | ComputeThinV);
    VectorXd singularValues = svd.singularValues();
    MatrixXd U = svd.matrixU();
    MatrixXd V = svd.matrixV();
    std::vector<int> indices(singularValues.size());
    for (int i = 0; i < singularValues.size(); ++i)
        indices[i] = i;
    std::sort(indices.begin(), indices.end(),
              [&singularValues](int i1, int i2) {
                  return singularValues(i1) > singularValues(i2);
              });
    //MatrixXd testMatrix = convYY.inverse() * convXY;
    rho.clear();
    A.clear();  B.clear();
    dataVec.clear();
    for (int i = 0; i < singularValues.size(); ++i) {
        rho.push_back(singularValues(indices[i]));
        VectorXd a = revConvXX * U.col(indices[i]), b = revConvYY * V.col(indices[i]);
        A.push_back(a);
        B.push_back(b);
        //std::cout<<"<a>:"<<(testMatrix * a /singularValues(indices[i])).transpose() <<std::endl;
        //std::cout<<"<b>:"<<b.transpose()<<std::endl;
    }
}
MatrixXd MADSolver::calcMatrixPowerNegHalf(const MatrixXd& conv) {
    SelfAdjointEigenSolver<MatrixXd> eigenSolver(conv);
    if (eigenSolver.info() != Success)
        throw std::runtime_error("Eigenvalue decomposition failed.");
    VectorXd eigenValues = eigenSolver.eigenvalues();
    MatrixXd eigenVectors = eigenSolver.eigenvectors();
    VectorXd eigenValuesInverseSqrt = eigenValues.array().sqrt().inverse();
    MatrixXd D = eigenValuesInverseSqrt.asDiagonal();
    return eigenVectors * D * eigenVectors.transpose();
}
