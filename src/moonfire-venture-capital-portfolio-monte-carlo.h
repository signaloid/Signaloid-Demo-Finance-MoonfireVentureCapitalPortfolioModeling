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

#pragma once

#include <stdlib.h>

/**
 *	@brief	Calculate and return the last-sample portfolio return, filling
 *		`monteCarloOutputSamples` with one independent portfolio-return
 *		sample per Monte Carlo iteration.
 *
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo iterations to run.
 *	@param	numberOfInvestments		: Number of investments in the portfolio.
 *	@param	alpha				: The Bounded Pareto distribution shape parameter.
 *	@param	xMin				: The Bounded Pareto distribution lower bound parameter.
 *	@param	xMax				: The Bounded Pareto distribution upper bound parameter.
 *	@param	monteCarloOutputSamples		: A pointer to an array where the per-iteration portfolio return samples are stored.
 *	@param	investmentReturns		: Scratch array of size `numberOfInvestments`, reused across iterations.
 *	@return	double				: Returns the last element of `monteCarloOutputSamples`.
 */
double
portfolioReturnMonteCarlo(
	size_t      numberOfMonteCarloIterations,
	size_t      numberOfInvestments,
	double      alpha,
	double      xMin,
	double      xMax,
	double *    monteCarloOutputSamples,
	double *    investmentReturns);

/**
 *	@brief	Calculate the probability of a portfolio loss from an array of
 *		Monte Carlo portfolio-return samples, as one minus the empirical
 *		CDF of the samples evaluated at `totalInvestment`. Sorts
 *		`monteCarloOutputSamples` in place as a side effect.
 *
 *	@param	monteCarloOutputSamples		: Array of Monte Carlo portfolio-return samples; sorted in place.
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo samples.
 *	@param	totalInvestment			: The break-even threshold (aggregate initial investment).
 *	@return	double				: Returns the probability of a portfolio loss, which is a scalar value.
 */
double
computeProbabilityOfLossMonteCarlo(
	double *    monteCarloOutputSamples,
	size_t      numberOfMonteCarloIterations,
	double      totalInvestment);

/**
 *	@brief	Calculate the empirical quantile at the given probability from an
 *		array of Monte Carlo portfolio-return samples, by inverting the
 *		empirical CDF. Sorts `monteCarloOutputSamples` in place as a side
 *		effect.
 *
 *	@param	monteCarloOutputSamples		: Array of Monte Carlo portfolio-return samples; sorted in place.
 *	@param	numberOfMonteCarloIterations	: Number of Monte Carlo samples.
 *	@param	probability			: Probability corresponding to the quantile, i.e. `p = P(X <= q)`.
 *	@return	double				: Returns the empirical quantile, which is a scalar value.
 */
double
computeQuantileMonteCarlo(
	double *    monteCarloOutputSamples,
	size_t      numberOfMonteCarloIterations,
	double      probability);
