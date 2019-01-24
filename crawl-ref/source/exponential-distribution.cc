#include "exponential-distribution.h"
#include <map>
#include <utility>
#include <tgmath.h>
#include <assert.h>

double _average_value(double q, int M);
double _q_guess(double mu, int M);
double _compute_base(double mu, int M);

double _average_value(double q, int M){
  double qm = pow(q,M);
  return q/(q*qm - 1)*((M+1)*qm - (q*qm - 1)/(q - 1));
}

double _q_guess(double mu, int M){
  /* We get a pretty good approximation to q by fitting a model a +
   * b/(X - b) + c*X where X = mu/M and the parameters depend on
   * M. This dependence is fairly complex so we use simpler
   * approximations. This could definitely be improved, but it is good
   * enough to achieve our tolerances in two iterations of the secant
   * method in most cases with a worst case of four iterations. 
   * All these numbers come out of regressions, not to be 
   * manually tweaked. */
  
  double a,b,c;
  double X = mu/M;
  double t = 1.0/M;
  
  if (M == 2)
    a = .9803;
  else if (M <= 26)
    a = -0.00530023909773214*M + 1.083828677482155;
  else
    a = -2.693725268549617/(M + 10.645455791588759) 
      + 1.016451397501317;

  if (M > 24)
    b = -0.7725332041927504*t - .002350037570408218;
  else
    b = -2.0990446376887535*t + .06147736645323743;
  
  if (M <= 17)
    c = 1./(.3624005064143364*M - 0.16115906227234122);
  else if (M > 17 && M < 23)
    c = 1./(-0.1625569933337456*M + 8.24116610462735);
  else
    c = 1./(.17545286619001513*M - .16115906227234122);

  return a + b/(X - b) + c*X;
}

double _compute_base(double mu, int M){
  /* compute base for exponential distribution with mean mu
   * and maximum value M to reasonable precision via secant method */
  
  double q, qq, A, AA, slope;

  assert(M > 0);
  assert(mu > 0.);
  assert(mu < M);

  if (M == 1)
    return mu/(1 - mu);
  if (mu > .5*M)
    return 1.0/_compute_base(-mu + M, M);

  q = _q_guess(mu, M);
  qq = q - .00001; 
  A = _average_value(q, M);
  AA = _average_value(qq, M);

  for (int i = 0; i < 20; i++){ // secant method
    slope = (AA - A)/(qq - q);
    q = qq;
    qq = (mu - AA + qq*slope)/slope;
    A = AA;
    AA = _average_value(qq, M);
    if (abs(mu - AA) < .001)
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
