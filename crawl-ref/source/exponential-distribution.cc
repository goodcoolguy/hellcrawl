#include "exponential-distribution.h"
#include <map>
#include <utility>
#include <tgmath.h>


double _average_value(double q, int M);
double _small_M_guess(double mu, int M);
double _compute_base(double mu, int M);

double _average_value(double q, int M){
  double qm = pow(q,M);
  return q/(q*qm - 1)*((M+1)*qm - (q*qm - 1)/(q - 1));
}

double _small_M_guess(double mu, int M){
  return ((M - 1) * mu/(1 + mu) + mu/(M - mu)) / M;
}
  
double _compute_base(double mu, int M){
  /* compute base for exponential distribution with mean mu
   * and maximum value M to reasonable precision via secant method */
  
  double q, qq, A, AA, slope;

  if (M < 25 && mu/M < .25)
    q = _small_M_guess(mu, M);
  else
    q = mu/(mu + 1);
  qq = q - .00001; 
  A = _average_value(q, M);
  AA = _average_value(qq, M);

  for (int i = 0; i < 20; i++){ // secant method
    slope = (AA - A)/(qq - q);
    q = qq;
    qq = (mu - AA + qq*slope)/slope;
    A = AA;
    AA = _average_value(qq, M);
    if (abs(mu - AA) < .00001)
      return qq;
  }

  return -1.0; // failed to converge within 20 iterations
}
    
int exponential_quantile(double x, double mu, int M)
{
  /* the quantile function of the discrete exponential distribution
   * on the interval [0, M] with expectation value mu evaluated at x */
  double q = _compute_base(mu, M);
  return floor(log(1 + x*(pow(q, M+1) - 1))/log(q));
}

/*
double _C(double q, int m)
  return (pow(qq,(m+1)) - 1)/(qq - 1);

double _Cmu(double q, int m){
  double qm = pow(q, m-1);
  return m*q*qm/(q - 1) + (q*qm - 1)/(q - 1) 
	  - (q*qm - 1)*q/pow(q - 1,2)*q;
}


double _dC_dq(double q, int m){
  double qm = pow(q, m);
  return (m + 1)*qm/(q - 1) - (q*qm - 1)/pow(q - 1, 2);
}


double _dCmu_dq(double q, int m){
  double qm = pow(q, m-1);
  return ((m - 1)*m*qm/(q - 1) 
	  + 2*m*qm/(q - 1) 
	  - 2*m*q*qm/pow(q - 1, 2)	
	  - 2*(q*qm - 1)/pow(q - 1, 2)
	  + 2*(q*qm - 1)*q/pow(q - 1,3))*q
	  + m*q*qm/(q - 1) 
	  + (q*qm - 1)/(q - 1) - (q*qm - 1)*q/pow(q - 1,2));
}
*/
