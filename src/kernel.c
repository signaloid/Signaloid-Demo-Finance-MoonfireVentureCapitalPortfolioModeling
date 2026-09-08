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
#include <stdbool.h>
#include <stdlib.h>
#include <uxhw.h>
#include "kernel.h"
#include "moonfire-venture-capital-portfolio-uxhw.h"
#include "moonfire-venture-capital-portfolio-monte-carlo.h"
#include "utilities.h"
#include "common.h"

/*
 *	Aggregate initial investment: `loadInvestmentReturns()`-derived per-
 *	investment returns are normalized so that a portfolio return of `1.0`
 *	corresponds to getting back exactly the amount invested.
 */
static const double kMoonfireVentureCapitalConstantsTotalInvestment = 1.0;

double
portfolioReturnSinglePath(
	size_t      numberOfInvestments,
	double      alpha,
	double      xMin,
	double      xMax,
	double *    investmentReturns)
{
	double  perInvestmentValue  = kMoonfireVentureCapitalConstantsTotalInvestment / (double) numberOfInvestments;
	double  portfolioReturn     = 0.0;

	for (size_t ii = 0; ii < numberOfInvestments; ii++)
	{
		/*
		 *	Bounded Pareto return for this investment.
		 */
		investmentReturns[ii]   = UxHwDoubleBoundedparetoDist(alpha, xMin, xMax + xMin);
		investmentReturns[ii]   -= xMin;
		investmentReturns[ii]   *= perInvestmentValue;
		portfolioReturn         += investmentReturns[ii];
	}

	return portfolioReturn;
}

double
calculateOutputUxHw(
	CommandLineArguments *  arguments,
	double *                outputVariables,
	double *                monteCarloOutputSamples)
{
	double      result = 0.0;
	double      portfolioReturn;
	double *    investmentReturns;
	bool        calculateAllOutputs;

	calculateAllOutputs = (arguments->common.outputSelect == kOutputVariableIndexMax);

	investmentReturns = (double *) checkedMalloc(
		sizeof(double) * arguments->numberOfInvestments,
		__FILE__,
		__LINE__
	);

	portfolioReturn = portfolioReturnSinglePath(
		arguments->numberOfInvestments,
		arguments->alpha,
		arguments->xMin,
		arguments->xMax,
		investmentReturns
	);

	free(investmentReturns);

	monteCarloOutputSamples[0] = portfolioReturn;

	/*
	 *	`portfolioReturn` is always recorded in
	 *	`outputVariables[kOutputVariableIndexPortfolioReturn]` regardless
	 *	of which output is selected, since it is the base value all other
	 *	outputs derive from.
	 */
	outputVariables[kOutputVariableIndexPortfolioReturn] = portfolioReturn;

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexPortfolioReturn)
	{
		result = portfolioReturn;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexProbabilityOfLoss)
	{
		double probabilityOfLoss = computeProbabilityOfLossUxHw(portfolioReturn, kMoonfireVentureCapitalConstantsTotalInvestment);

		if (arguments->common.outputSelect == kOutputVariableIndexProbabilityOfLoss)
		{
			monteCarloOutputSamples[0] = probabilityOfLoss;
		}

		result = outputVariables[kOutputVariableIndexProbabilityOfLoss] = probabilityOfLoss;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexLowQuantile)
	{
		double lowQuantile = computeQuantileUxHw(portfolioReturn, arguments->lowQuantileProbability);

		if (arguments->common.outputSelect == kOutputVariableIndexLowQuantile)
		{
			monteCarloOutputSamples[0] = lowQuantile;
		}

		result = outputVariables[kOutputVariableIndexLowQuantile] = lowQuantile;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexHighQuantile)
	{
		double highQuantile = computeQuantileUxHw(portfolioReturn, arguments->highQuantileProbability);

		if (arguments->common.outputSelect == kOutputVariableIndexHighQuantile)
		{
			monteCarloOutputSamples[0] = highQuantile;
		}

		result = outputVariables[kOutputVariableIndexHighQuantile] = highQuantile;
	}

	return result;
}

double
calculateOutputMonteCarlo(
	CommandLineArguments *  arguments,
	double *                outputVariables,
	double *                monteCarloOutputSamples)
{
	double          result = 0.0;
	double *        investmentReturns;
	size_t          numberOfMonteCarloIterations;
	bool            calculateAllOutputs;
	MeanAndVariance portfolioReturnMeanAndVariance;

	calculateAllOutputs             = (arguments->common.outputSelect == kOutputVariableIndexMax);
	numberOfMonteCarloIterations    = arguments->common.numberOfMonteCarloIterations;

	investmentReturns = (double *) checkedMalloc(
		sizeof(double) * arguments->numberOfInvestments,
		__FILE__,
		__LINE__
	);

	portfolioReturnMonteCarlo(
		numberOfMonteCarloIterations,
		arguments->numberOfInvestments,
		arguments->alpha,
		arguments->xMin,
		arguments->xMax,
		monteCarloOutputSamples,
		investmentReturns
	);

	free(investmentReturns);


	portfolioReturnMeanAndVariance = calculateMeanAndVarianceOfDoubleSamples(monteCarloOutputSamples, numberOfMonteCarloIterations);
	outputVariables[kOutputVariableIndexPortfolioReturn] = portfolioReturnMeanAndVariance.mean;

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexPortfolioReturn)
	{
		result = outputVariables[kOutputVariableIndexPortfolioReturn];
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexProbabilityOfLoss)
	{
		double probabilityOfLoss = computeProbabilityOfLossMonteCarlo(
			monteCarloOutputSamples,
			numberOfMonteCarloIterations,
			kMoonfireVentureCapitalConstantsTotalInvestment
		);

		if (arguments->common.outputSelect == kOutputVariableIndexProbabilityOfLoss)
		{
			monteCarloOutputSamples[0] = probabilityOfLoss;
		}

		result = outputVariables[kOutputVariableIndexProbabilityOfLoss] = probabilityOfLoss;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexLowQuantile)
	{
		double lowQuantile = computeQuantileMonteCarlo(
			monteCarloOutputSamples,
			numberOfMonteCarloIterations,
			arguments->lowQuantileProbability
		);

		if (arguments->common.outputSelect == kOutputVariableIndexLowQuantile)
		{
			monteCarloOutputSamples[0] = lowQuantile;
		}

		result = outputVariables[kOutputVariableIndexLowQuantile] = lowQuantile;
	}

	if (calculateAllOutputs || arguments->common.outputSelect == kOutputVariableIndexHighQuantile)
	{
		double highQuantile = computeQuantileMonteCarlo(
			monteCarloOutputSamples,
			numberOfMonteCarloIterations,
			arguments->highQuantileProbability
		);

		if (arguments->common.outputSelect == kOutputVariableIndexHighQuantile)
		{
			monteCarloOutputSamples[0] = highQuantile;
		}

		result = outputVariables[kOutputVariableIndexHighQuantile] = highQuantile;
	}

	return result;
}
