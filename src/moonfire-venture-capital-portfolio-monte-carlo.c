/*
 *	Copyright (c) 2024-2026, Signaloid.
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */

#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "moonfire-venture-capital-portfolio-monte-carlo.h"
#include "kernel.h"

/*
 *	Standard ascending double comparison, for sorting a Monte Carlo sample
 *	array before empirical CDF / empirical quantile evaluation.
 */
static int
compareDoublesAscending(const void * a, const void * b)
{
	double  arg1    = *(const double *) a;
	double  arg2    = *(const double *) b;

	if (arg1 > arg2)
	{
		return 1;
	}
	else if (arg1 < arg2)
	{
		return -1;
	}

	return 0;
}

double
portfolioReturnMonteCarlo(
	size_t      numberOfMonteCarloIterations,
	size_t      numberOfInvestments,
	double      alpha,
	double      xMin,
	double      xMax,
	double *    monteCarloOutputSamples,
	double *    investmentReturns)
{
	for (size_t jj = 0; jj < numberOfMonteCarloIterations; jj++)
	{
		monteCarloOutputSamples[jj] = portfolioReturnSinglePath(
			numberOfInvestments,
			alpha,
			xMin,
			xMax,
			investmentReturns
		);
	}

	return monteCarloOutputSamples[numberOfMonteCarloIterations - 1];
}

double
computeProbabilityOfLossMonteCarlo(
	double *    monteCarloOutputSamples,
	size_t      numberOfMonteCarloIterations,
	double      totalInvestment)
{
	double  cumulativeProbability   = 0.0;
	double  incrementalProbability  = 1.0 / (double) numberOfMonteCarloIterations;

	qsort(monteCarloOutputSamples, numberOfMonteCarloIterations, sizeof(double), compareDoublesAscending);

	/*
	 *	The empirical CDF gives equal mass to each of the samples. Since the
	 *	sample is sorted we break once the condition is violated.
	 */
	for (size_t ii = 0; ii < numberOfMonteCarloIterations; ii++)
	{
		if (monteCarloOutputSamples[ii] <= totalInvestment)
		{
			cumulativeProbability += incrementalProbability;
		}
		else
		{
			break;
		}
	}

	return 1.0 - cumulativeProbability;
}

double
computeQuantileMonteCarlo(
	double *    monteCarloOutputSamples,
	size_t      numberOfMonteCarloIterations,
	double      probability)
{
	size_t index;

	/*
	 *	Safeguard against illegal values.
	 */
	if ((probability <= 0.0) || (probability >= 1.0))
	{
		return NAN;
	}

	qsort(monteCarloOutputSamples, numberOfMonteCarloIterations, sizeof(double), compareDoublesAscending);

	/*
	 *	Compute the index corresponding to the quantile by inverting the
	 *	empirical CDF.
	 */
	index = (size_t) ceil((double) numberOfMonteCarloIterations * probability);

	/*
	 *	Safeguarding. Likely superfluous, but you can never be too sure. If
	 *	it is larger than `numberOfMonteCarloIterations`, set it to the
	 *	maximal index.
	 */
	if (index >= numberOfMonteCarloIterations)
	{
		index = numberOfMonteCarloIterations - 1;
	}

	return monteCarloOutputSamples[index];
}
