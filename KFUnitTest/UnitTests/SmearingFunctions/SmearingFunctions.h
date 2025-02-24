#pragma once

#include <Riostream.h>
#include <TMatrixDSymEigen.h>
#include <TVectorD.h>
#include <TMatrixD.h>
#include <TMath.h>
#include <TRandom.h>

//#include "../ConfigConstants.h"


template<typename T>
bool SmearParameters(size_t num_of_params, T parameters[], T covMatArr[]){
 
 Int_t ret = 0;


  TMatrixDSym A(num_of_params);
  for( Int_t k=0, i=0; i<num_of_params; i++ ){
    for( Int_t j=0; j<=i; j++, k++ ){
      A(i,j) = covMatArr[k];
      A(j,i) = covMatArr[k];
    }
  }

  TMatrixDSymEigen e(A);
  TVectorD D = e.GetEigenValues();
  TMatrixD V = e.GetEigenVectors();
  TMatrixD Vt = V;
  Vt.Transpose(V);

  TVectorD X(num_of_params);
  for( Int_t i=0; i<num_of_params; i++) X(i) = parameters[i];

  X = Vt*X;
  TVectorD X0 = X;
  for( Int_t i=0; i<num_of_params; i++){
    if( D(i) <0 ) ret = 1; 
    D(i) = TMath::Abs(D(i)); // correction of the covariance matrix
    Double_t sigma = TMath::Sqrt(D(i));
    X(i)+= gRandom->Gaus(0, sigma);
  }
  X = V*X;
  X0 = V*X0;

  TMatrixD DD(num_of_params,num_of_params);
  for( Int_t i=0; i<num_of_params; i++ ){
    for( Int_t j=0; j<num_of_params; j++ ) DD(i,j) = 0;
    DD(i,i) = D(i);
  }
  TMatrixD AA = V*DD*Vt;

    for( Int_t k=0, i=0; i<num_of_params; i++ ){
    parameters[i] = X(i);
    for( Int_t j=0; j<=i; j++, k++ ){
      //cout<<AA(i,j) - Cov[k]<<" ";
      covMatArr[k] = AA(i,j);
    }
    }

    return true;
}

bool SmearParticle(KFParticle& part){
    size_t num_of_params = 6;
    size_t covmat_size = num_of_params*(num_of_params+1)/2;

    float params[num_of_params];
    for (int i = 0; i < num_of_params; i++)
        params[i] = part.GetParameter(i);

    float covmat[covmat_size];
    for (int i = 0; i < covmat_size; i++)
        covmat[i] = part.GetCovariance(i);

    bool res = SmearParameters(num_of_params, params, covmat);

    float mass, masserr; part.GetMass(mass, masserr);
    float chi2 = part.GetChi2();
    float charge = (int)part.GetQ();
    int ndf = part.GetNDF();

    #ifdef TESTSUITE
    part.Create(params, covmat, charge, chi2, ndf, mass);
    #else
    part.Create(params, covmat, charge, mass);
    #endif

    return res;
}

bool SmearVertex(KFPVertex& vert){
    size_t num_of_params = 3;
    size_t covmat_size = num_of_params*(num_of_params+1)/2;

    float params[num_of_params];
    vert.GetXYZ(params);

    float covmat[covmat_size];
    vert.GetCovarianceMatrix(covmat);

    bool res = SmearParameters(num_of_params, params, covmat);
    vert.SetXYZ(params);
    return res;
}