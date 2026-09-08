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
#include "utilities.h"

/**
 *	@brief	Simulate a single portfolio realization by drawing an independent
 *		Bounded Pareto return for each of `numberOfInvestments` investments
 *		and summing the (equally-weighted) per-investment returns. The
 *		distributional draw uses `UxHwDoubleBoundedparetoDist`: in UxHw mode
 *		this returns the full Bounded Pareto distribution for each
 *		investment, yielding a distributional portfolio return. In Monte
 *		Carlo mode each call returns a single sample, yielding one Monte
 *		Carlo sample portfolio return.
 *
 *	@param	numberOfInvestments	: Number of investments in the portfolio.
 *	@param	alpha			: The Bounded Pareto distribution shape parameter.
 *	@param	xMin			: The Bounded Pareto distribution lower bound parameter.
 *	@param	xMax			: The Bounded Pareto distribution upper bound parameter.
 *	@param	investmentReturns	: Scratch array of size `numberOfInvestments`, filled with the per-investment returns.
 *	@return	double			: Returns the portfolio return (sum of the per-investment returns).
 */
double
portfolioReturnSinglePath(
	size_t      numberOfInvestments,
	double      alpha,
	double      xMin,
	double      xMax,
	double *    investmentReturns);

/**
 *	@brief	UxHw-mode calculation kernel. Computes the selected output(s) using
 *		distributional arithmetic on a single portfolio realization. Writes
 *		per-output results into `outputVariables` and the single
 *		distributional portfolio return / probability / quantile into
 *		`monteCarloOutputSamples[0]`.
 *
 *	@param	arguments		: Command-line arguments.
 *	@param	outputVariables		: Array of size `kOutputVariableIndexMax` to fill.
 *	@param	monteCarloOutputSamples	: Single-element array for the distributional result.
 *	@return	double			: Returns the value of the selected output (or the low quantile when all outputs are selected).
 */
double
calculateOutputUxHw(
	CommandLineArguments *  arguments,
	double *                outputVariables,
	double *                monteCarloOutputSamples);

/**
 *	@brief	Monte Carlo calculation kernel. Runs
 *		`arguments->common.numberOfMonteCarloIterations` independent
 *		portfolio realizations into `monteCarloOutputSamples` and computes
 *		the selected output over the sample array using empirical-CDF /
 *		empirical-quantile math (no UxHw distributional API). Writes
 *		per-output results into `outputVariables`.
 *
 *	@param	arguments		: Command-line arguments.
 *	@param	outputVariables		: Array of size `kOutputVariableIndexMax` to fill.
 *	@param	monteCarloOutputSamples	: Array of `numberOfMonteCarloIterations` doubles, filled with samples.
 *	@return	double			: Returns the value of the selected output (or the low quantile when all outputs are selected).
 */
double
calculateOutputMonteCarlo(
	CommandLineArguments *  arguments,
	double *                outputVariables,
	double *                monteCarloOutputSamples);
