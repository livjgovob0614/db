// TSSTAT.CPP
// Copyright (c) A.Sobolev 2002, 2003, 2004, 2007, 2008, 2010, 2013, 2016, 2018, 2019
//
#include <slib.h>
#include <tv.h>
#pragma hdrstop
// 
// gsl_stats_correlation()
// Calculate Pearson correlation = cov(X, Y) / (sigma_X * sigma_Y)
// This routine efficiently computes the correlation in one pass of the
// data and makes use of the algorithm described in:
// 
// B. P. Welford, "Note on a Method for Calculating Corrected Sums of
// Squares and Products", Technometrics, Vol 4, No 3, 1962.
// 
// This paper derives a numerically stable recurrence to compute a sum of products
// 
// S = sum_{i=1..N} [ (x_i - mu_x) * (y_i - mu_y) ]
// 
// with the relation
// 
// S_n = S_{n-1} + ((n-1)/n) * (x_n - mu_x_{n-1}) * (y_n - mu_y_{n-1})
// 
double gsl_stats_correlation(const double data1[], const size_t stride1, const double data2[], const size_t stride2, const size_t n)
{
	double sum_xsq = 0.0;
	double sum_ysq = 0.0;
	double sum_cross = 0.0;
	// 
	// Compute:
	// sum_xsq = Sum [ (x_i - mu_x)^2 ],
	// sum_ysq = Sum [ (y_i - mu_y)^2 ] and
	// sum_cross = Sum [ (x_i - mu_x) * (y_i - mu_y) ]
	// using the above relation from Welford's paper
	// 
	double mean_x = data1[0 * stride1];
	double mean_y = data2[0 * stride2];
	for(size_t i = 1; i < n; ++i) {
		const double ratio = i / (i + 1.0);
		const double delta_x = data1[i * stride1] - mean_x;
		const double delta_y = data2[i * stride2] - mean_y;
		sum_xsq += delta_x * delta_x * ratio;
		sum_ysq += delta_y * delta_y * ratio;
		sum_cross += delta_x * delta_y * ratio;
		mean_x += delta_x / (i + 1.0);
		mean_y += delta_y / (i + 1.0);
	}
	double r = sum_cross / (sqrt(sum_xsq) * sqrt(sum_ysq));
	return r;
}

struct gsl_block {
	size_t size;
	double * data;
};

struct gsl_vector {
	size_t size;
	size_t stride;
	double * data;
	gsl_block * block;
	int owner;
};

struct gsl_vector_view {
	gsl_vector vector;
};

void gsl_vector_set(gsl_vector * v, const size_t i, double x)
{
#if GSL_RANGE_CHECK
	if(GSL_RANGE_COND(i >= v->size)) {
		GSL_ERROR_VOID ("index out of range", GSL_EINVAL);
	}
#endif
	v->data[i * v->stride] = x;
}

/*
gsl_stats_spearman()
  Compute Spearman rank correlation coefficient

Inputs: data1   - data1 vector
        stride1 - stride of data1
        data2   - data2 vector
        stride2 - stride of data2
        n       - number of elements in data1 and data2
        work    - additional workspace of size 2*n

Return: Spearman rank correlation coefficient
*/
/*double gsl_stats_spearman(const double data1[], const size_t stride1, const double data2[], const size_t stride2, const size_t n, double work[])
{
	size_t i;
	gsl_vector_view ranks1 = gsl_vector_view_array(&work[0], n);
	gsl_vector_view ranks2 = gsl_vector_view_array(&work[n], n);
	double r;
	for(i = 0; i < n; ++i) {
		gsl_vector_set(&ranks1.vector, i, data1[i * stride1]);
		gsl_vector_set(&ranks2.vector, i, data2[i * stride2]);
	}
	// sort data1 and update data2 at same time; compute rank of data1 
	gsl_sort_vector2(&ranks1.vector, &ranks2.vector);
	compute_rank(&ranks1.vector);
	// now sort data2, updating ranks1 appropriately; compute rank of data2 
	gsl_sort_vector2(&ranks2.vector, &ranks1.vector);
	compute_rank(&ranks2.vector);
	// compute correlation of rank vectors in double precision 
	r = gsl_stats_correlation(ranks1.vector.data, ranks1.vector.stride, ranks2.vector.data, ranks2.vector.stride, n);
	return r;
}*/

SLAPI StatBase::StatBase(long flags)
{
	Init(flags);
}

SLAPI StatBase::~StatBase()
{
}

int SLAPI StatBase::Init(long flags /*=0*/)
{
	Flags = flags;
	Count = 0;
	IterCount = 0;
	memzero(Sum, sizeof(Sum));
	Min = SMathConst::Max;
	Max = SMathConst::Min;
	Exp = Var = 0.0;
	Kurtosis = 0.0;
	Skew = 0.0;
	Test_Z = 0.0;
	Test_ChSq = 0.0;
	Series.freeAll();
	return 1;
}

// @prototype
static double Lockes_Z_Test(const RealArray & x);

int SLAPI StatBase::Finish()
{
	if(Count > 0) {
		Exp = Sum[0] / Count;
		// Var = Exp * (Exp + 2.0 * Sum / Count) + SumQ / Count;
		Var = Sum[1] / Count - Exp * (2.0 * Sum[0] / Count - Exp);
		if(Flags & fExt) {
	    	Skew = (Sum[2] / Count) / pow(Sum[1] / Count, 1.5);
	    	Kurtosis = ((Sum[3] / Count) / fpowi(Sum[1] / Count, 2)) - 3.0; /* excess kurtosis */
		}
		if(Flags & fGammaTest) {
			Test_Z = Lockes_Z_Test(Series);
		}
	}
	else {
		Exp = 0.0;
		Var = 0.0;
	}
	return 1;
}

int SLAPI StatBase::Step(double val)
{
	Count++;
	IterCount++;
	Sum[0] += val;
	double sq = val * val;
	Sum[1] += sq;
	SETMAX(Max, val);
	SETMIN(Min, val);
	if(Flags & fExt) {
		Sum[2] += sq * val;
		Sum[3] += sq * sq;
	}
	if(Flags & (fGammaTest|fGaussianTest|fStoreVals)) {
		Series.insert(&val);
	}
	return 1;
}

double SLAPI StatBase::GetSum() const
{
	return Sum[0];
}

double SLAPI StatBase::GetVariance() const
{
	return (Count > 1) ? (Var * ((double)Count) / ((double)(Count - 1))) : 0.0;
}

double SLAPI StatBase::GetStdDev() const
{
	return sqrt(GetVariance());
}

int SLAPI StatBase::GetValue(long idx, double * pVal) const
{
	int    ok = -1;
	if(idx < Series.getCountI()) {
		ASSIGN_PTR(pVal, Series.at(idx));
		ok = 1;
	}
	return ok;
}

int SLAPI StatBase::GetGammaParams(double * pAlpha, double * pBeta) const
{
	double beta = fdivnz(GetVariance(), GetExp());
	double alpha = fdivnz(GetExp(), beta);
	ASSIGN_PTR(pAlpha, alpha);
	ASSIGN_PTR(pBeta, beta);
	return 1;
}
//
//
//
static int real_kendall_tau(const RealArray & x, const RealArray & y, RPairArray & rXy, double *ptau, double *pz)
{
	assert(x.getCount() == y.getCount());
	double tau, nn1, s2, z;
	int    tt1, tx = 0, ty = 0;
	int    Tx = 0, Ty = 0;
	int    Tx2 = 0, Ty2 = 0;
	int    Tx25 = 0, Ty25 = 0;
	int    tie, N0, N1, S;
	uint   i;
	//
	// populate sorter
	//
	uint   j = 0;
	rXy.clear();
	for(i = 0; i < x.getCount(); i++) {
		//if(!na(x[i]) && !na(y[i])) {
			RPair p;
			p.X = x.at(i);
			p.Y = y.at(i);
			rXy.insert(&p);
		//}
	}
	rXy.SortByX(); // sort pairs by x
	//
	// make order counts
	//
	N0 = N1 = 0;
	uint   nn = rXy.getCount();
	for(i = 0; i < nn; i++) {
		const RPair xy_i = rXy[i];
		double y_i = xy_i.Y;
		double x_i = xy_i.X;
		for(j = i+1; j < nn; j++) {
			const RPair & r_xy = rXy[j];
			if(r_xy.Y == y_i)
				Ty++;
			else if(r_xy.X != x_i) {
				if(r_xy.Y > y_i)
					N0++;
				else if(r_xy.Y < y_i)
					N1++;
			}
		}
		if(i > 0) {
			//
			// account for ties in x
			//
			tie = (x_i == rXy[i-1].X);
			if(tie)
				tx++;
			if(tx > 0 && (!tie || i == nn - 1)) {
				tx++;
				tt1 = tx * (tx - 1);
				Tx += tt1;
				Tx2 += tt1 * (tx - 2);
				Tx25 += tt1 * (2 * tx + 5);
				tx = 0;
			}
		}
	}
	if(Ty > 0) {
		//
		// account for ties in y
		//
		Ty = 0;
		rXy.SortByY();
		for(i = 1; i < nn; i++) {
			tie = (rXy[i].Y == rXy[i-1].Y);
			if(tie)
				ty++;
			if(ty > 0 && (!tie || i == nn - 1)) {
				ty++;
				tt1 = ty * (ty - 1);
				Ty += tt1;
				Ty2 += tt1 * (ty - 2);
				Ty25 += tt1 * (2 * ty + 5);
				ty = 0;
			}
		}
	}
	S = N0 - N1;
#if 0
	fprintf(stderr, "N0 = %d, N1 = %d, S = %d\n", N0, N1, S);
	fprintf(stderr, "Tx = %d, Ty = %d\n", Tx, Ty);
#endif
	nn1 = nn * (nn - 1.0);
	//
	// normal approximation as in Shapiro and Chen, Journal of Quality Technology, 2001
	//
	if(Tx == 0 && Ty == 0) {
		tau = 2 * S / nn1;
		s2 = (1.0/18) * nn1 * (2 * nn + 5);
	}
	else {
		double den = (nn1 - Tx) * (nn1 - Ty);
		tau = 2 * S / sqrt(den);
		s2 = (1.0/18) * (nn1 * (2 * nn + 5) - Tx25 - Ty25);
		if(Tx2 != 0 && Ty2 != 0)
			s2 += (1.0/(9*nn1*(nn-2))) * Tx2 * Ty2;
		if(Tx != 0 && Ty != 0)
			s2 += (1.0/(2*nn1)) * Tx * Ty;
	}
	z = (S - 1) / sqrt(s2);
	ASSIGN_PTR(ptau, tau);
	ASSIGN_PTR(pz, z);
	return 1;
}
//
// check for negative values, screen out missing values, and load x into an array for randomization
//
static int locke_shuffle_init(const RealArray & rX, RealArray & rResult)
{
	uint   i, m = 0;
	rResult.clear();
	for(i = 0; i < rX.getCount(); i++) {
		if(rX.at(i) < 0.0)
			return 0;
		else //if(!na(x[i]))
			m++;
	}
	if(m < 4) {
		return 0;
	}
	else {
		for(i = 0; i < rX.getCount(); i++) {
			//if(!na(x[i])) {
				rResult.insert(&rX.at(i));
			//}
		}
		return 1;
	}
}

IMPL_CMPCFUNC(lockes_test_rand, i1, i2)
{
	SRng * p_rng = (SRng *)pExtraData;
	ulong r = p_rng ? p_rng->GetUniformInt(8096) : 0;
	return (r - 4097);
}
//
// lockes_test:
// @x: data series.
//
// Performs Charles Locke's nonparametric test for whether an
// empirical distribution (namely, that of @x over the range @t1 to @t2) is gamma.
// See C. Locke, "A Test for the Composite Hypothesis that a Population has a Gamma Distribution,"
// Commun. Statis.-Theor. Meth. A5(4), 351-364 (1976).  Also see Shapiro and Chen,
// Journal of Quality Technology 33(1), Jan 2001.
// Returns:
//   the z value for test, or #NADBL on error.
//
static double Lockes_Z_Test(const RealArray & x)
{
	double z = 0.0; // NADBL
	double zj;
	uint   m = x.getCount();
	uint   i, t;
	RealArray sx;
	SRng * p_rng = 0;
	if(locke_shuffle_init(x, sx)) {
		m /= 2;
		// dimention = m
		RealArray u;
		RealArray v;
		RPairArray uv;
		double zero = 0.0;
		for(i = 0; i < m; i++) {
			u.insert(&zero);
			v.insert(&zero);
		}
		z = 0.0;
		uint   nrepeat = 100;
		if(x.getCount() > 10000)
			nrepeat = 1;
		else if(x.getCount() > 1000)
			nrepeat = 10;
		//
		// repeat the shuffling of the series NREPEAT times, since the
		// test statistic is sensitive to the ordering under the null
		//
		p_rng = SRng::CreateInstance(SRng::algMT, 0);
		p_rng->Set(getcurtime_());
		for(uint j = 0; j < nrepeat; j++) {
			sx.sort(PTR_CMPCFUNC(lockes_test_rand), p_rng);
			t = 0;
			for(i = 0; i < m; i++) {
				u[i] = sx[t] + sx[t+1];
				v[i] = sx[t] / sx[t+1];
				double temp = sx[t+1] / sx[t];
				SETMAX(v[i], temp);
				t += 2;
			}
			real_kendall_tau(u, v, uv, 0, &zj);
			z += zj;
#if LOCKE_DEBUG
			printf("z[%d] = %g\n", j, zj);
#endif
		}
		z /= (double)nrepeat;
#if LOCKE_DEBUG
		fprintf(stderr, "Kendall's tau: average z = %g\n", z);
#endif
	}
	delete p_rng;
	return z;
}
//
//
//
SLAPI TimSerStat::TimSerStat() : StatBase(0), P_Queue(0), P_AC_Add(0), P_AC_Mul(0)
{
	Init(0);
}

SLAPI TimSerStat::~TimSerStat()
{
	delete P_Queue;
	delete P_AC_Add;
	delete P_AC_Mul;
}

int SLAPI TimSerStat::Init(long flags)
{
	StatBase::Init(flags);
	NumLags = 0;
	ZDELETE(P_Queue);
	ZDELETE(P_AC_Add);
	ZDELETE(P_AC_Mul);
	return 1;
}

int SLAPI TimSerStat::SetNumLags(long numLags)
{
	EXCEPTVAR(SLibError);
	int    ok = 1;
	NumLags = numLags;
	ZDELETE(P_Queue);
	ZDELETE(P_AC_Add);
	ZDELETE(P_AC_Mul);
	if(NumLags > 0) {
		size_t n = static_cast<size_t>(NumLags);
		THROW(P_Queue = new DblQueue(n));
		THROW(P_AC_Add = new double[n]);
		THROW(P_AC_Mul = new double[n]);
		memzero(P_AC_Add, sizeof(double) * n);
		memzero(P_AC_Mul, sizeof(double) * n);
	}
	CATCHZOK
	return ok;
}

int SLAPI TimSerStat::Step(double val, int /*whiteSpace*/)
{
	StatBase::Step(val);
	if(NumLags > 0) {
		const uint c = P_Queue->getNumItems();
		for(uint i = 0; i < c; i++) {
			const  uint   lag_1 = c - i - 1;
			const  double v_i = P_Queue->get(i);
   	        P_AC_Add[lag_1] += (val + v_i);
	   	    P_AC_Mul[lag_1] += (val * v_i);
		}
		P_Queue->push(val);
	}
	return 1;
}

int SLAPI TimSerStat::Finish()
{
	StatBase::Finish();
	if(NumLags > 0) {
		const double n_var = Count * Var;
		const double qexp = Exp * Exp;
		for(uint i = 0; i < P_Queue->getNumItems(); i++)
			P_AC_Add[i] = (qexp * (Count-i-1) + P_AC_Mul[i] - Exp * P_AC_Add[i]) / n_var;
		return 1;
	}
	else
		return -1;
}

uint SLAPI TimSerStat::GetNumLags() const
{
	return P_Queue ? P_Queue->getNumItems() : 0;
}

double SLAPI TimSerStat::GetAutocorrel(uint lag) const
{
	return (lag < GetNumLags()) ? P_AC_Add[lag] : 0L;
}
//
//
//
SLAPI TimSerSpikes::TimSerSpikes() : Sign(0), Count(0), P_Spikes(0)
{
}

SLAPI TimSerSpikes::~TimSerSpikes()
{
	delete P_Spikes;
}

int SLAPI TimSerSpikes::Init(int sign)
{
	Sign = sign;
	Count = 0;
	MEMSZERO(Prev);
	P_Spikes = new SVector(sizeof(Spike)); // @v9.8.12 SArray-->SVector
	return P_Spikes ? 1 : (SLibError = SLERR_NOMEM, 0);
}

int SLAPI TimSerSpikes::Step(long n, double val)
{
	Count++;
	if(Count > 2) {
		if(Sign >= 0)
			if(Prev[1].V >= Prev[0].V && Prev[1].V > val)
				if(!P_Spikes->insert(&Prev[1]))
					return 0;
		if(Sign <= 0)
			if(Prev[1].V <= Prev[0].V && Prev[1].V < val)
				if(!P_Spikes->insert(&Prev[1]))
					return 0;
		Prev[0] = Prev[1];
		Prev[1].V = val;
		Prev[1].N = n;
	}
	else {
		Prev[(size_t)Count-1].V = val;
		Prev[(size_t)Count-1].N = n;
	}
	return 1;
}

long SLAPI TimSerSpikes::GetNumSpikes() const
{
	return SVectorBase::GetCount(P_Spikes);
}

double SLAPI TimSerSpikes::GetSpike(long n, long * pN) const
{
	if(n < GetNumSpikes()) {
		Spike * p_spike = static_cast<Spike *>(P_Spikes->at((uint)n));
		ASSIGN_PTR(pN, p_spike->N);
		return p_spike->V;
	}
	return 0L;
}

struct _PRDINF { // @flat
	long   Distance;
	long   Count;
};

long SLAPI TimSerSpikes::GetMostCommonDistance()
{
	uint   i = 0;
	_PRDINF prd, * p_prd;
	SVector prd_ary(sizeof(_PRDINF)); // @v9.8.12 SArray-->SVector
	for(i = 0; (int)i < GetNumSpikes() - 1; i++) {
		long s, n;
		uint idx = 0;
		GetSpike(i,   &s);
   		GetSpike(i+1, &n);
		prd.Distance = labs(s - n);
		if(prd_ary.lsearch(&prd, &idx, CMPF_LONG)) {
			p_prd = static_cast<_PRDINF *>(prd_ary.at(idx));
			p_prd->Distance = labs(s - n);
			p_prd->Count++;
		}
		else {
			prd.Distance = labs(s - n);
			prd.Count = 1;
			prd_ary.insert(&prd);
		}
	}
	long max_count = 0;
	long distance = 3; // default distance
	for(i = 0; prd_ary.enumItems(&i, (void **)&p_prd);)
		if(p_prd->Count > max_count) {
			distance = p_prd->Distance;
			max_count = p_prd->Count;
		}
	return distance;
}
//
//
//
SLAPI STimeSeries::ValuVec::ValuVec(const char * pSymb, TYPEID typ, int fxPrec) : Typ(typ), SVector(stsize(typ), O_ARRAY), Symb(pSymb), FxPrec(fxPrec), Flags(0)
{
	assert(STimeSeries::VerifyValuVecType(typ));
	assert(!oneof2(Typ, T_FLOAT, T_DOUBLE) || FxPrec == 0);
}

SLAPI STimeSeries::ValuVec::ValuVec() : Typ(T_DOUBLE), SVector(stsize(T_DOUBLE), O_ARRAY), FxPrec(0), Flags(0)
{
	assert(STimeSeries::VerifyValuVecType(Typ));
}

void FASTCALL STimeSeries::ValuVec::ConvertDoubleToInner(double outer, void * pInner) const
{
	switch(Typ) {
		case T_DOUBLE: *(double *)pInner = outer; break;
		case T_FLOAT:  *(float *)pInner = (float)outer; break;
		case T_INT32:
			if(FxPrec != 0)
				*(int32 *)pInner = (int32)R0(outer * fpow10i(FxPrec));
			else
				*(int32 *)pInner = (int32)outer;
			break;
		case T_INT64:
			if(FxPrec != 0)
				*(int64 *)pInner = (int64)R0(outer * fpow10i(FxPrec));
			else
				*(int64 *)pInner = (int64)outer;
			break;
		default:
			assert(0);
	}
}

void FASTCALL STimeSeries::ValuVec::ConvertFloatToInner(float outer, void * pInner) const
{
	switch(Typ) {
		case T_DOUBLE: *static_cast<double *>(pInner) = outer; break;
		case T_FLOAT:  *static_cast<float *>(pInner) = outer; break;
		case T_INT32:  *static_cast<int32 *>(pInner) = (FxPrec != 0) ? (int32)R0(outer * fpow10fi(FxPrec)) : (int32)outer; break;
		case T_INT64:  *static_cast<int64 *>(pInner) = (FxPrec != 0) ? (int64)R0(outer * fpow10fi(FxPrec)) : (int64)outer; break;
		default: assert(0);
	}
}

void FASTCALL STimeSeries::ValuVec::ConvertInt32ToInner(int32 outer, void * pInner) const
{
	switch(Typ) {
		case T_DOUBLE: *static_cast<double *>(pInner) = (double)outer; break;
		case T_FLOAT:  *static_cast<float *>(pInner) = (float)outer; break;
		case T_INT32:  *static_cast<int32 *>(pInner) = (FxPrec != 0) ? (int32)(outer * fpow10i(FxPrec)) : outer; break;
		case T_INT64:  *static_cast<int64 *>(pInner) = (FxPrec != 0) ? (int64)(outer * fpow10i(FxPrec)) : (int64)outer; break;
		default: assert(0);
	}
}

void FASTCALL STimeSeries::ValuVec::ConvertInt64ToInner(int64 outer, void * pInner) const
{
	switch(Typ) {
		case T_DOUBLE: *static_cast<double *>(pInner) = (double)outer; break;
		case T_FLOAT:  *static_cast<float *>(pInner) = (float)outer; break;
		case T_INT32:
			if(FxPrec != 0)
				*static_cast<int32 *>(pInner) = (int32)(outer * fpow10i(FxPrec));
			else
				*static_cast<int32 *>(pInner) = (int32)outer;
			break;
		case T_INT64:
			if(FxPrec != 0)
				*static_cast<int64 *>(pInner) = (int64)(outer * fpow10i(FxPrec));
			else
				*static_cast<int64 *>(pInner) = outer;
			break;
		default:
			assert(0);
	}
}

double FASTCALL STimeSeries::ValuVec::ConvertInnerToDouble(const void * pInner) const
{
	switch(Typ) {
		case T_DOUBLE: return *static_cast<const double *>(pInner);
		case T_FLOAT:  return *static_cast<const float *>(pInner);
		case T_INT32:  return FxPrec ? (((double)(*(int32 *)pInner)) / fpow10i(FxPrec)) : (double)*static_cast<const int32 *>(pInner);
		case T_INT64:  return FxPrec ? (((double)(*(int64 *)pInner)) / fpow10i(FxPrec)) : (double)*static_cast<const int64 *>(pInner);
		default: assert(0); return 0.0;
	}
}

float FASTCALL STimeSeries::ValuVec::ConvertInnerToFloat(const void * pInner) const
{
	switch(Typ) {
		case T_DOUBLE: return (float)*static_cast<const double *>(pInner);
		case T_FLOAT:  return *static_cast<const float *>(pInner);
		case T_INT32:  return FxPrec ? (((float)(*static_cast<const int32 *>(pInner))) / fpow10fi(FxPrec)) : (float)*static_cast<const int32 *>(pInner);
		case T_INT64:  return FxPrec ? (((float)(*static_cast<const int64 *>(pInner))) / fpow10fi(FxPrec)) : (float)*static_cast<const int64 *>(pInner);
		default: assert(0); return 0.0f;
	}
}

int32 FASTCALL STimeSeries::ValuVec::ConvertInnerToInt32(const void * pInner) const
{
	switch(Typ) {
		case T_DOUBLE: return (int32)*static_cast<const double *>(pInner);
		case T_FLOAT:  return (int32)*static_cast<const float *>(pInner);
		case T_INT32:  return FxPrec ? (int32)(((double)(*static_cast<const int32 *>(pInner))) / fpow10i(FxPrec)) : *static_cast<const int32 *>(pInner);
		case T_INT64:  return FxPrec ? (int32)(((double)(*static_cast<const int64 *>(pInner))) / fpow10i(FxPrec)) : (int32)*static_cast<const int64 *>(pInner);
		default: assert(0); return 0;
	}
}

int64 FASTCALL STimeSeries::ValuVec::ConvertInnerToInt64(const void * pInner) const
{
	switch(Typ) {
		case T_DOUBLE: return (int64)*static_cast<const double *>(pInner);
		case T_FLOAT:  return (int64)*static_cast<const float *>(pInner);
		case T_INT32:  return FxPrec ? (int64)(((double)(*static_cast<const int32 *>(pInner))) / fpow10i(FxPrec)) : (int64)*static_cast<const int32 *>(pInner);
		case T_INT64:  return FxPrec ? (int64)(((double)(*static_cast<const int64 *>(pInner))) / fpow10i(FxPrec)) : *static_cast<const int64 *>(pInner);
		default: assert(0); return 0;
	}
}

SLAPI STimeSeries::STimeSeries() : Ver(1), Id(0), State(0)
{
	memzero(Reserve, sizeof(Reserve));
}

SLAPI STimeSeries::STimeSeries(const STimeSeries & rS) 
{
	Copy(rS);
}

SLAPI STimeSeries::~STimeSeries()
{
}

STimeSeries & FASTCALL STimeSeries::operator = (const STimeSeries & rS)
{
	Copy(rS);
	return *this;
}

int FASTCALL STimeSeries::Copy(const STimeSeries & rS)
{
	int    ok = 1;
	Destroy();
	Ver = rS.Ver;
	Id = rS.Id;
	State = rS.State;
	Symb = rS.Symb;
	T = rS.T;
	TSCollection_Copy(VL, rS.VL);
	return ok;
}

void FASTCALL STimeSeries::SetSymb(const char * pSymb)
{
	Symb = pSymb;
}

const char * FASTCALL STimeSeries::GetSymb() const
{
	return Symb;
}

STimeSeries & SLAPI STimeSeries::Z()
{
	for(uint i = 0; i < VL.getCount(); i++) {
		ValuVec * p_vv = VL.at(i);
		CALLPTRMEMB(p_vv, clear());
	}
	T.clear();
	return *this;
}

void SLAPI STimeSeries::Destroy()
{
	VL.freeAll();
	T.freeAll();
}

int SLAPI STimeSeries::SetupBySample(const STimeSeries * pSample)
{
	int    ok = 1;
	Destroy();
	if(pSample) {
		for(uint i = 0; i < pSample->VL.getCount(); i++) {
			const ValuVec * p_s_vec = pSample->VL.at(i);
			ValuVec * p_new_vec = 0;
			if(p_s_vec) {
				THROW(p_new_vec = new ValuVec(p_s_vec->Symb, p_s_vec->Typ, p_s_vec->FxPrec));
			}
			THROW(VL.insert(p_new_vec));
		}
	}
	CATCHZOK
	return ok;
}

uint SLAPI STimeSeries::GetCount() const
{
	return T.getCount();
}

int FASTCALL STimeSeries::GetTime(uint itemIdx, SUniTime * pT) const
{
	int    ok = 1;
	if(itemIdx < T.getCount()) {
		ASSIGN_PTR(pT, T.at(itemIdx));
	}
	else
		ok = 0;
	return ok;
}

int FASTCALL STimeSeries::Swap(uint p1, uint p2)
{
	int    ok = 1;
	if(p1 == p2)
		ok = -1;
	else {
		THROW(T.swap(p1, p2));
		for(uint i = 0; i < VL.getCount(); i++) {
			ValuVec * p_vv = VL.at(i);
			if(p_vv)
				THROW(p_vv->swap(p1, p2));
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::AddItemFromSample(const STimeSeries & rSample, uint samplePos)
{
	int    ok = 1;
	uint   idx = 0;
	assert(rSample.VL.getCount() == VL.getCount());
	THROW(rSample.VL.getCount() == VL.getCount());
	THROW(AddItem(rSample.T.at(samplePos), &idx));
	for(uint i = 0; i < rSample.VL.getCount(); i++) {
		const ValuVec * p_s_vec = rSample.VL.at(i);
		ValuVec * p_vec = VL.at(i);
		assert((p_s_vec && p_vec) || (!p_s_vec && !p_vec));
		THROW((p_s_vec && p_vec) || (!p_s_vec && !p_vec));
		if(p_s_vec) {
			assert(p_s_vec->Typ == p_vec->Typ && p_s_vec->FxPrec == p_vec->FxPrec);
			THROW(p_s_vec->Typ == p_vec->Typ && p_s_vec->FxPrec == p_vec->FxPrec);
			THROW(Helper_SetValue(idx, p_vec, p_s_vec->at(samplePos)));
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::SearchEntry(const SUniTime & rUt, uint startPos, uint * pIdx) const
{
	int    ok = 0;
	for(uint i = startPos; !ok && i < T.getCount(); i++) {
		const SUniTime & r_ut = T.at(i);
		int cr = r_ut.IsEq(rUt);
		if(oneof2(cr, SUniTime::cmprSureTrue, SUniTime::cmprUncertainTrue)) {
			ASSIGN_PTR(pIdx, i);
			ok = cr;
		}
	}
	return ok;
}

int SLAPI STimeSeries::SearchFirstEntrySince(const SUniTime & rSince, uint * pIdx) const
{
	int    ok = 0;
	for(uint i = 0; !ok && i < T.getCount(); i++) {
		const SUniTime & r_ut = T.at(i);
		int    sq;
		int    si = r_ut.Compare(rSince, &sq);
		if(si >= 0 && oneof2(sq, SUniTime::cqSure, SUniTime::cqUncertain)) {
			ASSIGN_PTR(pIdx, i);
			ok = 1;
		}
	}
	return ok;
}

int SLAPI STimeSeries::SearchEntryReverse(const SUniTime & rUt, uint startPos, uint * pIdx) const
{
	int    ok = 0;
	uint   i = startPos;
	if(i) do {
		const SUniTime & r_ut = T.at(--i);
		int cr = r_ut.IsEq(rUt);
		if(oneof2(cr, SUniTime::cmprSureTrue, SUniTime::cmprUncertainTrue)) {
			ASSIGN_PTR(pIdx, i);
			ok = cr;
		}
	} while(!ok && i);
	return ok;
}

static IMPL_CMPFUNC(SUniTime, p1, p2)
{
	int sq;
	return static_cast<const SUniTime *>(p1)->Compare(*static_cast<const SUniTime *>(p2), &sq);
}

int SLAPI STimeSeries::SearchEntryBinary(const SUniTime & rUt, uint * pIdx) const
{
	return T.bsearch(&rUt, pIdx, PTR_CMPFUNC(SUniTime));
}

SLAPI STimeSeries::AppendStat::AppendStat() : AppendCount(0), UpdCount(0), SrcFldsCount(0), 
	IntersectFldsCount(0), TmProfile(0), SpreadSum(0), SpreadCount(0), UpdCountVecIdx(-1)
{
}

int SLAPI STimeSeries::AddItems(const STimeSeries & rSrc, AppendStat * pStat)
{
	int    ok = -1;
	SProfile::Measure pm;
	LAssocArray src_to_this_vl_assoc_list;
	const ValuVec * p_src_spread_vec = 0;
	if(pStat) {
		pStat->SrcFldsCount = rSrc.VL.getCount();
		pStat->AppendCount = 0;
		pStat->UpdCount = 0;
	}
	for(uint i = 0; i < rSrc.VL.getCount(); i++) {
		const ValuVec * p_s_vec = rSrc.VL.at(i);
		if(p_s_vec->Symb.NotEmpty()) {
			uint tidx = 0;
			if(pStat && p_s_vec->Symb.IsEqiAscii("spread"))
				p_src_spread_vec = p_s_vec;
			if(GetValueVecIndex(p_s_vec->Symb, &tidx)) {
				src_to_this_vl_assoc_list.Add(i+1, tidx+1);
			}
		}
	}
	if(pStat)
		pStat->IntersectFldsCount = src_to_this_vl_assoc_list.getCount();
	if(src_to_this_vl_assoc_list.getCount()) {
		SUniTime ut_last;
		SUniTime ut;
		SUniTime fut;
		int    skip_existance_checking = 0;
		if(GetCount())
			GetTime(GetCount()-1, &ut_last);
		else
			skip_existance_checking = 1;
		for(uint j = 0; j < rSrc.GetCount(); j++) {
			if(rSrc.GetTime(j, &ut)) {
				uint   ii = 0;
				int    mod = 0; // 1 - append, 2 - update
				//const int  ser = SearchEntryReverse(ut, T.getCount(), &ii);
				const int  ser = skip_existance_checking ? 0 : SearchEntryBinary(ut, &ii);
				if(ser > 0) {
					GetTime(ii, &fut);
				}
				else {
					THROW(AddItem(ut, &ii));
				}
				for(uint k = 0; k < src_to_this_vl_assoc_list.getCount(); k++) {
					const LAssoc & r_asc = src_to_this_vl_assoc_list.at(k);
					double val = 0.0;
					THROW(rSrc.GetValue(j, r_asc.Key-1, &val));
					const long local_vec_idx = (r_asc.Val-1);
					if(ser > 0) {
						double ex_val = 0.0;
						THROW(GetValue(ii, local_vec_idx, &ex_val));
						if(ex_val != val) {
							THROW(SetValue(ii, local_vec_idx, val));
							mod = 2;
							if(pStat && (pStat->UpdCountVecIdx < 0 || pStat->UpdCountVecIdx == local_vec_idx))
								pStat->UpdCount++;
						}
					}
					else {
						THROW(SetValue(ii, local_vec_idx, val));
						if(p_src_spread_vec) {
							assert(pStat != 0); // ����� p_src_spread_vec ��� �� ������� (see above)
							const void * p_value_buf = p_src_spread_vec->at(j);
							if(p_value_buf) {
								int32 temp_val = p_src_spread_vec->ConvertInnerToInt32(p_value_buf);
								if(temp_val >= 0) {
									pStat->SpreadSum += temp_val;
									pStat->SpreadCount++;
								}
							}
						}
						mod = 1;
					}
				}
				assert(mod == 1 || ser > 0);
				if(pStat) {
					/*if(mod == 2)
						pStat->UpdCount++;
					else*/ if(mod == 1)
						pStat->AppendCount++;
				}
				ok = 1;
			}
		}
	}
	CATCHZOK
	if(pStat)
		pStat->TmProfile = pm.Get();
	return ok;
}

static IMPL_CMPFUNC(STimeSeriesIndex, p1, p2)
{
	int    si = 0;
	const  STimeSeries * p_ts = static_cast<const STimeSeries *>(pExtraData);
	if(p_ts) {
		SUniTime t1;
		SUniTime t2;
		if(p_ts->GetTime(*static_cast<const uint *>(p1), &t1) && p_ts->GetTime(*static_cast<const uint *>(p2), &t2)) {
			int sq;
			si = t1.Compare(t2, &sq);
		}
	}
	return si;
}

int SLAPI STimeSeries::Sort(/*CompFunc fcmp, void * pExtraData*/ /*=0*/)
{
	int    ok = 1;
	const  uint tc = T.getCount();
	if(tc > 1) {
		uint i;
		TSVector <uint> pos_vec;
		for(i = 0; i < VL.getCount(); i++) {
			const ValuVec * p_vv = VL.at(i);
			if(p_vv) {
				THROW(p_vv->getCount() == tc);
			}
		}
		for(i = 0; i < tc; i++)
			pos_vec.insert(&i);
		assert(pos_vec.getCount() == tc);
		pos_vec.sort2(PTR_CMPFUNC(STimeSeriesIndex), this);
		assert(pos_vec.getCount() == tc);
		{
			STimeSeries ts_temp;
			THROW(ts_temp.SetupBySample(this));
			for(i = 0; i < tc; i++) {
				uint pos = pos_vec.at(i);
				THROW(ts_temp.AddItemFromSample(*this, pos));
			}
			//
			Z();
			for(i = 0; i < tc; i++) {
				THROW(AddItemFromSample(ts_temp, i));
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pSCtx)
{
	int    ok = 1;
	THROW(pSCtx->Serialize(dir, Ver, rBuf));
	THROW(pSCtx->Serialize(dir, Id, rBuf));
	THROW(pSCtx->Serialize(dir, Symb, rBuf));
	THROW(pSCtx->SerializeBlock(dir, sizeof(Reserve), Reserve, rBuf, 0));
	THROW(pSCtx->Serialize(dir, &T, rBuf));
	//THROW(TSCollection_Serialize(VL, dir, rBuf, pSCtx));
	//template <class T> int TSCollection_Serialize(TSCollection <T> & rC, int dir, SBuffer & rBuf, SSerializeContext * pSCtx)
	{
		if(dir > 0) {
			uint32 c = VL.getCount();
			THROW(rBuf.Write(&c, sizeof(c)));
			for(uint i = 0; i < c; i++) {
				STimeSeries::ValuVec * p_item = VL.at(i);
				THROW(pSCtx->Serialize(dir, p_item->Typ, rBuf));
				THROW(pSCtx->Serialize(dir, p_item->FxPrec, rBuf));
				THROW(pSCtx->Serialize(dir, p_item->Flags, rBuf));
				THROW(pSCtx->Serialize(dir, p_item->Symb, rBuf));
				THROW(pSCtx->Serialize(dir, dynamic_cast<SVector *>(p_item), rBuf));
				//THROW(p_item->Serialize(dir, rBuf, pSCtx))
			}
		}
		else if(dir < 0) {
			VL.freeAll();
			SString c_symb;
			uint32 c = 0;
			THROW(rBuf.ReadV(&c, sizeof(c)));
			for(uint i = 0; i < c; i++) {
				TYPEID c_typ;
				int16  c_fx_prec;
				uint16 c_flags;
				THROW(pSCtx->Serialize(dir, c_typ, rBuf));
				THROW(pSCtx->Serialize(dir, c_fx_prec, rBuf));
				THROW(pSCtx->Serialize(dir, c_flags, rBuf));
				THROW(pSCtx->Serialize(dir, c_symb, rBuf));
				//THROW(pSCtx->Serialize(dir, dynamic_cast<SVector *>(p_item), rBuf));
				{
					STimeSeries::ValuVec * p_new_item = new STimeSeries::ValuVec(c_symb, c_typ, c_fx_prec);
					//T * p_new_item = rC.CreateNewItem();
					THROW(p_new_item);
					p_new_item->Flags = c_flags;
					THROW(pSCtx->Serialize(dir, dynamic_cast<SVector *>(p_new_item), rBuf));
					//THROW(p_new_item->Serialize(dir, rBuf, pSCtx));
					THROW(VL.insert(p_new_item));
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

STimeSeries::ValuVec * FASTCALL STimeSeries::GetVecBySymb(const char * pSymb, uint * pVecIdx) const
{
	if(!isempty(pSymb)) {
		for(uint i = 0; i < VL.getCount(); i++) {
			ValuVec * p_vec = VL.at(i);
			if(p_vec && p_vec->GetSymb().IsEqiAscii(pSymb)) {
				ASSIGN_PTR(pVecIdx, i);
				return p_vec;
			}
		}
	}
	return (SLS.SetError(SLERR_MATH_TSVECBYSYMBNFOUND, pSymb), 0);
}

STimeSeries::ValuVec * FASTCALL STimeSeries::GetVecByIdx(uint vecIdx) const
{
	return (vecIdx < VL.getCount()) ? VL.at(vecIdx) : 0;
}

int SLAPI STimeSeries::AddValueVec(const char * pSymb, TYPEID typ, int fxPrec, uint * pVecIdx)
{
	int    ok = 1;
	uint   vec_idx = 0;
	THROW(!GetVecBySymb(pSymb, 0));
	THROW(VerifyValuVecType(typ));
	{
		ValuVec * p_new_vec = new ValuVec(pSymb, typ, fxPrec);
		THROW(p_new_vec);
		vec_idx = VL.getCount();
		THROW(VL.insert(p_new_vec));
		ASSIGN_PTR(pVecIdx, vec_idx);
	}
	CATCHZOK
	return ok;
}

uint SLAPI STimeSeries::GetValueVecCount() const
{
	return VL.getCount();
}

int  SLAPI STimeSeries::GetValueVecParam(uint vecIdx, TYPEID * pTyp, SString * pSymb, int * pFxPrec, uint * pFlags) const
{
	int    ok = 0;
	if(vecIdx < VL.getCount()) {
		const STimeSeries::ValuVec * p_vec = VL.at(vecIdx);
		if(p_vec) {
			ASSIGN_PTR(pTyp, p_vec->Typ);
			ASSIGN_PTR(pSymb, p_vec->Symb);
			ASSIGN_PTR(pFxPrec, p_vec->FxPrec);
			ASSIGN_PTR(pFlags, p_vec->Flags);
			ok = 1;
		}
	}
	return ok;
}

int SLAPI STimeSeries::GetValueVecIndex(const char * pSymb, uint * pIdx) const
{
	return BIN(GetVecBySymb(pSymb, pIdx));
}

int SLAPI STimeSeries::AddItem(SUniTime tm, uint * pItemIdx)
{
	if(T.insert(&tm)) {
		ASSIGN_PTR(pItemIdx, T.getCount()-1);
		return 1;
	}
	else
		return 0;
}

int SLAPI STimeSeries::Helper_SetValue(uint itemIdx, STimeSeries::ValuVec * pVec, const void * pValuePtr)
{
	assert(pValuePtr);
	int    ok = 1;
	THROW(pVec);
	THROW(itemIdx < T.getCount());
	{
		const uint org_vec_count = pVec->getCount();
		if(itemIdx < org_vec_count) {
			pVec->atPut(itemIdx, pValuePtr);
		}
		else {
			if(itemIdx > org_vec_count) {
				uint8 zero_value_buf[16];
				memzero(zero_value_buf, sizeof(zero_value_buf));
				for(uint i = 0; i < (itemIdx - org_vec_count); i++) {
					THROW(pVec->insert(zero_value_buf));
				}
			}
			assert(itemIdx == pVec->getCount());
			THROW(pVec->insert(pValuePtr));
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::SetValue(uint itemIdx, uint vecIdx, double value)
{
	uint8 value_buf[16];
	memzero(value_buf, sizeof(value_buf));
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
	if(p_vec) {
		p_vec->ConvertDoubleToInner(value, value_buf);
		return Helper_SetValue(itemIdx, p_vec, value_buf);
	}
	else
		return 0;
}

int SLAPI STimeSeries::SetValue(uint itemIdx, uint vecIdx, float value)
{
	uint8 value_buf[16];
	memzero(value_buf, sizeof(value_buf));
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
	if(p_vec) {
		p_vec->ConvertFloatToInner(value, value_buf);
		return Helper_SetValue(itemIdx, p_vec, value_buf);
	}
	else
		return 0;
}

int SLAPI STimeSeries::SetValue(uint itemIdx, uint vecIdx, int32 value)
{
	uint8 value_buf[16];
	memzero(value_buf, sizeof(value_buf));
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
	if(p_vec) {
		p_vec->ConvertInt32ToInner(value, value_buf);
		return Helper_SetValue(itemIdx, p_vec, value_buf);
	}
	else
		return 0;
}

int SLAPI STimeSeries::SetValue(uint itemIdx, uint vecIdx, int64 value)
{
	uint8 value_buf[16];
	memzero(value_buf, sizeof(value_buf));
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
	if(p_vec) {
		p_vec->ConvertInt64ToInner(value, value_buf);
		return Helper_SetValue(itemIdx, p_vec, value_buf);
	}
	else
		return 0;
}

int SLAPI STimeSeries::GetValue(uint itemIdx, uint vecIdx, double * pValue) const
{
	int    ok = 0;
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
	if(p_vec && itemIdx < p_vec->getCount()) {
		if(pValue) {
			const void * p_value_buf = p_vec->at(itemIdx);
			*pValue = p_vec->ConvertInnerToDouble(p_value_buf);
		}
		ok = 1;
	}
	return ok;
}

int SLAPI STimeSeries::GetValue(uint itemIdx, uint vecIdx, float * pValue) const
{
	int    ok = 0;
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
	if(p_vec && itemIdx < p_vec->getCount()) {
		if(pValue) {
			const void * p_value_buf = p_vec->at(itemIdx);
			*pValue = p_vec->ConvertInnerToFloat(p_value_buf);
		}
		ok = 1;
	}
	return ok;
}

int SLAPI STimeSeries::GetValue(uint itemIdx, uint vecIdx, int32 * pValue) const
{
	int    ok = 0;
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
	if(p_vec && itemIdx < p_vec->getCount()) {
		if(pValue) {
			const void * p_value_buf = p_vec->at(itemIdx);
			*pValue = p_vec->ConvertInnerToInt32(p_value_buf);
		}
		ok = 1;
	}
	return ok;
}

int SLAPI STimeSeries::GetValue(uint itemIdx, uint vecIdx, int64 * pValue) const
{
	int    ok = 0;
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
	if(p_vec && itemIdx < p_vec->getCount()) {
		if(pValue) {
			const void * p_value_buf = p_vec->at(itemIdx);
			*pValue = p_vec->ConvertInnerToInt64(p_value_buf);
		}
		ok = 1;
	}
	return ok;
}

SLAPI STimeSeries::SnapshotEntry::SnapshotEntry() : Idx(0)
{
}

STimeSeries::SnapshotEntry & SLAPI STimeSeries::SnapshotEntry::Z()
{
	Idx = 0;
	Tm.Z();
	Values.clear();
	return *this;
}

int SLAPI STimeSeries::GetSnapshotEntry(uint idx, SnapshotEntry & rEntry) const
{
	int    ok = 1;
	THROW(idx < GetCount());
	rEntry.Idx = idx;
	rEntry.Tm = T.at(idx);
	rEntry.Values.clear();
	for(uint i = 0; i < VL.getCount(); i++) {
		const STimeSeries::ValuVec * p_vec = VL.at(i);
		THROW(p_vec);
		const void * p_value_buf = p_vec->at(idx);
		double value = p_vec->ConvertInnerToDouble(p_value_buf);
		THROW(rEntry.Values.Add(i, value, 0, 0));
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::SetSnapshotEntry(uint idx, const SnapshotEntry & rEntry)
{
	int    ok = 1;
	THROW(idx < GetCount());
	T.at(idx) = rEntry.Tm;
	for(uint i = 0; i < VL.getCount(); i++) {
		uint8 value_buf[16];
		STimeSeries::ValuVec * p_vec = VL.at(i);
		THROW(p_vec);
		memzero(value_buf, sizeof(value_buf));
		const double value = rEntry.Values.Get(i, 0);
		p_vec->ConvertDoubleToInner(value, value_buf);
		THROW(Helper_SetValue(idx, p_vec, value_buf));
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::AppendSnapshotEntry(const SnapshotEntry & rEntry)
{
	int    ok = 1;
	uint   idx = 0;
	THROW(AddItem(rEntry.Tm, &idx));
	for(uint i = 0; i < VL.getCount(); i++) {
		uint8 value_buf[16];
		STimeSeries::ValuVec * p_vec = VL.at(i);
		THROW(p_vec);
		memzero(value_buf, sizeof(value_buf));
		const double value = rEntry.Values.Get(i, 0);
		p_vec->ConvertDoubleToInner(value, value_buf);
		THROW(Helper_SetValue(idx, p_vec, value_buf));
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::GetLVect(uint vecIdx, uint startItemIdx, LVect & rV) const
{
	int    ok = -1;
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
    THROW(p_vec);
	if(rV.size() > 0) {
		const uint last_idx = startItemIdx+rV.size()-1;
		if(last_idx < p_vec->getCount()) {
			for(uint i = startItemIdx, j = 0; i <= last_idx; i++, j++) {
				const void * p_value_buf = p_vec->at(i);
				double value = p_vec->ConvertInnerToDouble(p_value_buf);
				THROW(rV.set(j, value));
			}
			ok = 1;
		}
	}
    CATCHZOK
	return ok;
}

int SLAPI STimeSeries::GetTimeArray(uint startItemIdx, uint idxCount, DateTimeArray & rV) const
{
	rV.clear();
	int   ok = -1;
	if(idxCount) {
		const uint last_idx = (startItemIdx+idxCount-1);
		if(last_idx < T.getCount()) {
			for(uint i = startItemIdx; i <= last_idx; i++) {
				const SUniTime & r_ut = T.at(i);
				LDATETIME dtm;
				r_ut.Get(dtm);
				THROW(rV.add(dtm));
			}
			ok = 1;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::GetRealArray(uint vecIdx, uint startItemIdx, uint idxCount, RealArray & rV) const
{
	rV.clear();
	int    ok = -1;
	STimeSeries::ValuVec * p_vec = GetVecByIdx(vecIdx);
    THROW(p_vec);
	if(idxCount) {
		const uint last_idx = (startItemIdx+idxCount-1);
		if(last_idx < p_vec->getCount()) {
			double buffer[128];
			uint   buffer_ptr = 0;
			for(uint i = startItemIdx; i <= last_idx; i++) {
				const void * p_value_buf = p_vec->at(i);
				double value = p_vec->ConvertInnerToDouble(p_value_buf);
				if(buffer_ptr == SIZEOFARRAY(buffer)) {
					THROW(rV.insertChunk(buffer_ptr, buffer));
					buffer_ptr = 0;					
				}
				assert(buffer_ptr < SIZEOFARRAY(buffer));
				buffer[buffer_ptr++] = value;
			}
			if(buffer_ptr) {
				THROW(rV.insertChunk(buffer_ptr, buffer));
				buffer_ptr = 0;					
			}
			ok = 1;
		}
	}
    CATCHZOK
	return ok;
}

SLAPI STimeSeries::Stat::Stat(long flags) : StatBase(flags), State(0), DeltaAvg(0.0)
{
}

int SLAPI STimeSeries::Analyze(const char * pVecSymb, uint firstIdx, uint count, Stat & rS) const
{
	int    ok = 1;
	uint   vec_idx = 0;
	STimeSeries::ValuVec * p_vec = GetVecBySymb(pVecSymb, &vec_idx);
	THROW(p_vec);
	assert(firstIdx >= 0 && firstIdx < GetCount());
	assert((firstIdx+count) <= GetCount());
	THROW(firstIdx >= 0 && firstIdx < GetCount());
	THROW((firstIdx+count) <= GetCount());
	{
		//const  uint _c = GetCount();
		const uint _c = (firstIdx+count);
		StatBase stat_delta(0);
		rS.State |= Stat::stSorted;
		SUniTime prev_utm;
		SUniTime utm;
		double prev_value = 0.0;
		for(uint i = firstIdx; i < _c; i++) {
			THROW(GetTime(i, &utm));
			if(i > firstIdx) {
				int   sq = 0;
				int   si = utm.Compare(prev_utm, &sq);
				if(si < 0)
					rS.State &= ~Stat::stSorted;
				else if(si == 0 && sq == SUniTime::cmprSureTrue)
					rS.State |= Stat::stHasTmDup;
			}
			{
				const void * p_value_buf = p_vec->at(i);
				double value = p_vec->ConvertInnerToDouble(p_value_buf);
				rS.Step(value);
				if(i > firstIdx) {
					double delta = (value - prev_value) / prev_value;
					stat_delta.Step(fabs(delta));
				}
				prev_value = value;
			}
			prev_utm = utm;
		}
		stat_delta.Finish();
		rS.Finish();
		rS.DeltaAvg = stat_delta.GetExp();
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::Analyze(const char * pVecSymb, Stat & rS) const
{
	return Analyze(pVecSymb, 0, GetCount(), rS);
}

int SLAPI STimeSeries::RemoveItem(uint idx)
{
	int    ok = -1;
	if(idx < GetCount()) {
		for(uint vec_idx = 0; vec_idx < VL.getCount(); vec_idx++) {
			ValuVec * p_vec = VL.at(vec_idx);
			if(p_vec) {
				THROW(p_vec->getCount() == GetCount());
				p_vec->atFree(idx);
			}
		}
		T.atFree(idx);
		ok = 1;
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::Repair(const char * pCriticalVecSymb)
{
	int    ok = -1;
	const  uint _c = GetCount();
	if(_c) {
		LongArray pos_to_remove;
		Sort();
		SUniTime prev_utm;
		SUniTime utm;
		double value = 0.0;
		double prev_value = 0.0;
		STimeSeries::ValuVec * p_vec = 0;
		uint   vec_idx = 0;
		if(!isempty(pCriticalVecSymb)) {
			THROW(p_vec = GetVecBySymb(pCriticalVecSymb, &vec_idx));
		}
		for(uint i = 0; i < _c; i++) {
			THROW(GetTime(i, &utm));
			if(p_vec) {
				const void * p_value_buf = p_vec->at(i);
				value = p_vec->ConvertInnerToDouble(p_value_buf);
			}
			if(i) {
				int   sq = 0;
				int   si = utm.Compare(prev_utm, &sq);
				if(si == 0 && sq == SUniTime::cmprSureTrue) {
					/*if(p_vec) {
						assert(feqeps(value, prev_value, 1e-6));
					}*/
					pos_to_remove.add(i);
				}
			}
			prev_utm = utm;
			prev_value = value;
		}
		{
			uint rp = pos_to_remove.getCount();
			if(rp) do {
				const uint idx_to_remove = static_cast<const uint>(pos_to_remove.get(--rp));
				THROW(RemoveItem(idx_to_remove));
				ok = 1;
			} while(rp);
		}
	}
	CATCHZOK
	return ok;
}

SLAPI STimeSeries::AnalyzeFitParam::AnalyzeFitParam(uint distance, uint firstIdx, uint count) : Distance(distance), FirstIdx(firstIdx), IdxCount(count), Flags(0)
{
}

int SLAPI STimeSeries::AnalyzeFit(const char * pVecSymb, const AnalyzeFitParam & rP, RealArray * pTrendList, RealArray * pSumSqList,
	RealArray * pCov00, RealArray * pCov01, RealArray * pCov11) const
{
	CALLPTRMEMB(pTrendList, clear());
	CALLPTRMEMB(pSumSqList, clear());
	CALLPTRMEMB(pCov00, clear());
	CALLPTRMEMB(pCov01, clear());
	CALLPTRMEMB(pCov11, clear());
	int    ok = 1;
	uint   vec_idx = 0;
	STimeSeries::ValuVec * p_vec = GetVecBySymb(pVecSymb, &vec_idx);
	THROW(p_vec);
	{
		const  uint _c = GetCount();
		if(_c && rP.FirstIdx < _c) {
			const  uint ic = (rP.IdxCount == 0) ? _c : MIN(rP.FirstIdx + rP.IdxCount, _c);
			const  uint distance = rP.Distance;
			RealArray lss_rv_x;
			RealArray lss_rv_y;
			const size_t chunk_size = 128;
			double trend_chunk[chunk_size];
			double sumsq_chunk[chunk_size];
			double cov00_chunk[chunk_size];
			double cov01_chunk[chunk_size];
			double cov11_chunk[chunk_size];
			size_t chunk_p = 0;
			for(uint i = 0; i < distance; i++) {
				THROW(lss_rv_x.add(static_cast<double>(i+1)));
			}
			THROW(lss_rv_y.dim(distance));
			/*if(pTrendList) {
				THROW(pTrendList->add(0.0));
			}
			if(pSumSqList) {
				THROW(pSumSqList->add(0.0));
			}*/
			for(uint j = rP.FirstIdx/*+1*/; j < ic; j++) {
				double trend;
				double sumsq;
				double cov00, cov01, cov11;
				if(j >= distance) {
					LssLin lss;
					int gvr = GetRealArray(vec_idx, j-distance+1, distance, lss_rv_y); // @v10.2.12 @fix (j-distance)-->(j-distance+1)
					assert(gvr > 0);
					assert(lss_rv_x.getCount() == distance);
					assert(lss_rv_y.getCount() == distance);
					lss.Solve(distance, static_cast<const double *>(lss_rv_x.dataPtr()), static_cast<const double *>(lss_rv_y.dataPtr()));
					trend = lss.B;
					sumsq = lss.SumSq;
					cov00 = lss.Cov00;
					cov01 = lss.Cov01;
					cov11 = lss.Cov11;
				}
				else {
					trend = 0.0;
					sumsq = 0.0;
					cov00 = 0.0;
					cov01 = 0.0;
					cov11 = 0.0;
				}
				if(chunk_p >= chunk_size) {
					if(pTrendList) { THROW(pTrendList->insertChunk(chunk_p, trend_chunk)); }
					if(pSumSqList) { THROW(pSumSqList->insertChunk(chunk_p, sumsq_chunk)); }
					if(pCov00) { THROW(pCov00->insertChunk(chunk_p, cov00_chunk)); }
					if(pCov01) { THROW(pCov01->insertChunk(chunk_p, cov01_chunk)); }
					if(pCov11) { THROW(pCov11->insertChunk(chunk_p, cov11_chunk)); }
					chunk_p = 0;
				}
				trend_chunk[chunk_p] = trend;
				sumsq_chunk[chunk_p] = sumsq;
				cov00_chunk[chunk_p] = cov00;
				cov01_chunk[chunk_p] = cov01;
				cov11_chunk[chunk_p] = cov11;
				chunk_p++;
			}
			if(chunk_p) {
				if(pTrendList) { THROW(pTrendList->insertChunk(chunk_p, trend_chunk)); }
				if(pSumSqList) { THROW(pSumSqList->insertChunk(chunk_p, sumsq_chunk)); }
				if(pCov00) { THROW(pCov00->insertChunk(chunk_p, cov00_chunk)); }
				if(pCov01) { THROW(pCov01->insertChunk(chunk_p, cov01_chunk)); }
				if(pCov11) { THROW(pCov11->insertChunk(chunk_p, cov11_chunk)); }
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::Helper_GetFrame(const ValuVec * pVec, uint startIdx, uint count, double diffScale, long normFlags, RealArray & rList) const
{
	rList.clear();
	int    ok = 1;
	const  bool is_diff_scale = (diffScale != 0.0);
	//uint   vec_idx = 0;
	//STimeSeries::ValuVec * p_vec = GetVecBySymb(pVecSymb, &vec_idx);
	if(pVec && startIdx < pVec->getCount()) {
		const uint last_idx = MIN(startIdx+count, pVec->getCount());
		if(normFlags == 0) {
			for(uint i = startIdx; i < last_idx; i++) {
				const void * p_value_buf = pVec->at(i);
				double value = pVec->ConvertInnerToDouble(p_value_buf);
				THROW(rList.insert(&value));
			}
		}
		else {
			double base = 0.0;
			StatBase sb(StatBase::fStoreVals);
			for(uint i = startIdx; i < last_idx; i++) {
				const void * p_value_buf = pVec->at(i);
				double value = pVec->ConvertInnerToDouble(p_value_buf);
				sb.Step(value);
			}
			sb.Finish();
			const RealArray & r_sb_ser = sb.GetSeries();
			assert(r_sb_ser.getCount() == (last_idx-startIdx));
			if(normFlags & nfBaseAvg)
				base = sb.GetExp();
			else {
				base = r_sb_ser.at(0);
			}
			if(base == 0.0 && !(normFlags & nfZero)) {
				// ������� �� ����: ������ ������ ������� - ���������� ������ ������
			}
			else {
				for(uint j = 0; j < r_sb_ser.getCount(); j++) {
					double ex_value = r_sb_ser.at(j);
					double value;
					if(normFlags & nfZero) {
						value = ex_value - base;
						if(is_diff_scale)
							value *= diffScale;
					}
					else {
						value = ex_value / base;
					}
					THROW(rList.insert(&value));
				}
			}
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::GetFrame(const char * pVecSymb, uint startIdx, uint count, long normFlags, RealArray & rList) const
	{ return Helper_GetFrame(GetVecBySymb(pVecSymb, 0), startIdx, count, 0.0, normFlags, rList); }
int SLAPI STimeSeries::GetFrame(const char * pVecSymb, uint startIdx, uint count, double diffScale, long normFlags, RealArray & rList) const
	{ return Helper_GetFrame(GetVecBySymb(pVecSymb, 0), startIdx, count, diffScale, normFlags, rList); }
int SLAPI STimeSeries::GetFrame(uint vecIdx, uint startIdx, uint count, long normFlags, RealArray & rList) const
	{ return Helper_GetFrame(GetVecByIdx(vecIdx), startIdx, count, 0.0, normFlags, rList); }
int SLAPI STimeSeries::GetFrame(uint vecIdx, uint startIdx, uint count, double diffScale, long normFlags, RealArray & rList) const
	{ return Helper_GetFrame(GetVecByIdx(vecIdx), startIdx, count, diffScale, normFlags, rList); }

int SLAPI STimeSeries::GetChunkRecentCount(uint count, STimeSeries & rResult) const
{
	int    ok = -1;
	THROW(rResult.SetupBySample(this));
	if(count) {
		const  uint tsc = GetCount();
		if(tsc >= count) {
			for(uint i = (tsc-count); i < tsc; i++) {
				THROW(rResult.AddItemFromSample(*this, i));
			}
			ok = 1;
		}
	}
	CATCHZOK
	return ok;
}

int SLAPI STimeSeries::GetChunkRecentSince(const SUniTime & rSince, STimeSeries & rResult) const
{
	int    ok = -1;
	uint   first_idx = 0;
	THROW(rResult.SetupBySample(this));
	if(SearchFirstEntrySince(rSince, &first_idx) > 0) {
		const  uint tsc = GetCount();
		for(uint i = first_idx; i < tsc; i++) {
			THROW(rResult.AddItemFromSample(*this, i));
		}
		ok = 1;
	}
	CATCHZOK
	return ok;
}
//
//
//
#if SLTEST_RUNNING // {

SLTEST_R(STimeSeries)
{
	int    ok = 1;
	SString temp_buf;
	SString src_file_name;
	SString test_file_name;
	SLS.QueryPath("testroot", src_file_name);
	src_file_name.SetLastSlash().Cat("data").SetLastSlash().Cat("ts-eurusd.csv");
	SLS.QueryPath("testroot", test_file_name);
	test_file_name.SetLastSlash().Cat("out").SetLastSlash().Cat("ts-eurusd.out");
	SFile f_in(src_file_name, SFile::mRead);
	if(f_in.IsValid()) {
		SString line_buf;
		StringSet ss_in(",");
		STimeSeries ts;

		LDATETIME dtm;
		double open = 0.0;
		double close = 0.0;
		long   tick_vol = 0;
		long   real_vol = 0;
		long   spread = 0;

		uint   vecidx_open = 0;
		uint   vecidx_close = 0;
		uint   vecidx_ticvol = 0;
		uint   vecidx_realvol = 0;
		uint   vecidx_spread = 0;
		//THROW(ts.AddValueVec("open", T_DOUBLE, 0, &vecidx_open));
		THROW(SLTEST_CHECK_NZ(ts.AddValueVec("open", T_INT32, 5, &vecidx_open)));
		//THROW(ts.AddValueVec("close", T_DOUBLE, 0, &vecidx_close));
		THROW(SLTEST_CHECK_NZ(ts.AddValueVec("close", T_INT32, 5, &vecidx_close)));
		THROW(SLTEST_CHECK_NZ(ts.AddValueVec("tick_volume", T_INT32, 0, &vecidx_ticvol)));
		THROW(SLTEST_CHECK_NZ(ts.AddValueVec("real_volume", T_INT32, 0, &vecidx_realvol)));
		THROW(SLTEST_CHECK_NZ(ts.AddValueVec("spread", T_INT32, 0, &vecidx_spread)));
		{
			uint8 sign[8];
			size_t actual_size = 0;
			if(f_in.Read(sign, 4, &actual_size) && actual_size == 4) {
				if(sign[0] == 0xEF && sign[1] == 0xBB && sign[2] == 0xBF)
					f_in.Seek(3);
				else
					f_in.Seek(0);
			}
		}
		while(f_in.ReadLine(line_buf)) {
			line_buf.Chomp().Strip();
			if(line_buf.NotEmpty()) {
				ss_in.setBuf(line_buf);
				dtm.Z();
				open = 0.0;
				close = 0.0;
				tick_vol = 0;
				real_vol = 0;
				spread = 0;
				for(uint ssp = 0, fldn = 0; ss_in.get(&ssp, temp_buf); fldn++) {
					switch(fldn) {
						case 0: strtodate(temp_buf, DATF_YMD, &dtm.d); break;
						case 1: strtotime(temp_buf, TIMF_HMS, &dtm.t); break;
						case 2: open = temp_buf.ToReal(); break;
						case 3: close = temp_buf.ToReal(); break;
						case 4: tick_vol = temp_buf.ToLong(); break;
						case 5: real_vol = temp_buf.ToLong(); break;
						case 6: spread = temp_buf.ToLong(); break;
					}
				}
				if(checkdate(&dtm) && close > 0.0) {
					SUniTime ut;
					ut.Set(dtm, SUniTime::indMin);
					uint   item_idx = 0;
					THROW(SLTEST_CHECK_NZ(ts.AddItem(ut, &item_idx)));
					THROW(SLTEST_CHECK_NZ(ts.SetValue(item_idx, vecidx_open, open)));
					THROW(SLTEST_CHECK_NZ(ts.SetValue(item_idx, vecidx_close, close)));
					THROW(SLTEST_CHECK_NZ(ts.SetValue(item_idx, vecidx_ticvol, tick_vol)));
					THROW(SLTEST_CHECK_NZ(ts.SetValue(item_idx, vecidx_realvol, real_vol)));
					THROW(SLTEST_CHECK_NZ(ts.SetValue(item_idx, vecidx_spread, spread)));
				}
			}
		}
		{
			//
			STimeSeries dts;
			SBuffer sbuf; // serialize buf
			SBuffer cbuf; // compress buf
			SBuffer dbuf; // decompress buf
			SSerializeContext sctx;
			THROW(SLTEST_CHECK_NZ(ts.Serialize(+1, sbuf, &sctx)));
			{
				{
					SCompressor c(SCompressor::tZLib);
					THROW(SLTEST_CHECK_NZ(c.CompressBlock(sbuf.GetBuf(sbuf.GetRdOffs()), sbuf.GetAvailableSize(), cbuf, 0, 0)));
				}
				{
					SCompressor c(SCompressor::tZLib);
					THROW(SLTEST_CHECK_NZ(c.DecompressBlock(cbuf.GetBuf(cbuf.GetRdOffs()), cbuf.GetAvailableSize(), dbuf)));
				}
				SLTEST_CHECK_EQ(sbuf.GetAvailableSize(), dbuf.GetAvailableSize());
				SLTEST_CHECK_Z(memcmp(sbuf.GetBuf(sbuf.GetRdOffs()), dbuf.GetBuf(dbuf.GetRdOffs()), sbuf.GetAvailableSize()));
				THROW(SLTEST_CHECK_NZ(dts.Serialize(-1, dbuf, &sctx)));
			}
			{
				SFile f_out(test_file_name, SFile::mWrite);
				THROW(SLTEST_CHECK_NZ(f_out.IsValid()));
				for(uint i = 0; i < dts.GetCount(); i++) {
					SUniTime ut;
					dts.GetTime(i, &ut);
					ut.Get(dtm);
					THROW(SLTEST_CHECK_NZ(dts.GetValue(i, vecidx_open, &open)));
					THROW(SLTEST_CHECK_NZ(dts.GetValue(i, vecidx_close, &close)));
					THROW(SLTEST_CHECK_NZ(dts.GetValue(i, vecidx_ticvol, &tick_vol)));
					THROW(SLTEST_CHECK_NZ(dts.GetValue(i, vecidx_realvol, &real_vol)));
					THROW(SLTEST_CHECK_NZ(dts.GetValue(i, vecidx_spread, &spread)));
					line_buf.Z().Cat(dtm.d, DATF_ANSI|DATF_CENTURY).Comma().Cat(dtm.t, TIMF_HM).Comma().
						Cat(open, MKSFMTD(0, 5, 0)).Comma().Cat(close, MKSFMTD(0, 5, 0)).Comma().Cat(tick_vol).Comma().Cat(real_vol).Comma().Cat(spread).CR();
					THROW(SLTEST_CHECK_NZ(f_out.WriteLine(line_buf)));
				}
				f_out.Close();
				SLTEST_CHECK_LT(0L, SFile::Compare(src_file_name, test_file_name, 0));
			}
		}
	}
	CATCHZOK
	return ok;
}

#endif // } SLTEST_RUNNING
