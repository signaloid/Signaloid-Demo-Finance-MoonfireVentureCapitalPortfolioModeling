/*
 *	Copyright (c) 2024, Signaloid.
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <uxhw.h>
#include "utilities.h"


static const double	kMoonfireVentureCapitalConstantsTotalInvestment	= 1.0;

/**
 *	@brief	Populates the `invesmentReturns` array with the initial Bounded Pareto
 *		distributions. Reads values from the `arguments`.
 *
 *	@param	arguments		: Pointer to command-line arguments struct.
 *	@param	investmentReturns	: The array of input investment returns.
 */
static void
loadInvestmentReturns(
	CommandLineArguments *	arguments,
	double *		investmentReturns)
{
	double	perInvestmentValue = kMoonfireVentureCapitalConstantsTotalInvestment / arguments->numberOfInvestments;
	
	for (size_t i = 0; i < arguments->numberOfInvestments; i++)
	{
		investmentReturns[i] = UxHwDoubleBoundedparetoDist(
			arguments->alpha,
			arguments->xMin,
			arguments->xMax + arguments->xMin);
		investmentReturns[i] -= arguments->xMin;
		investmentReturns[i] *= perInvestmentValue;
	}

	return;
}

/**
 *	@brief	Calculates the portfolio return by summing the returns of each individual investment.
 *
 *	@param	arguments		: Pointer to command-line arguments struct.
 *	@param	investmentReturns	: The array of investment returns to populate.
 *
 *	@return				: Returns the calculated portfolio return.
 */
static double
calculatePortfolioReturn(
	CommandLineArguments *	arguments,
	double *		investmentReturns)
{
	double	portfolioReturn = 0.0;

	for (size_t i = 0; i < arguments->numberOfInvestments; i++)
	{
		portfolioReturn += investmentReturns[i];
	}

	return portfolioReturn;
}

int
main(int argc, char *  argv[])
{
	CommandLineArguments	arguments = {0};
	double *		investmentReturns;
	double			portfolioReturn;
	double			probabilityOfLoss;
	double			lowQuantile;
	double			highQuantile;
	double *		monteCarloOutputSamples;
	clock_t			start = 0;
	clock_t			end = 0;
	double			cpuTimeInSeconds;
	MeanAndVariance		monteCarloOutputMeanAndVariance = {0};

	/*
	 *	Get command-line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments) != kCommonConstantReturnTypeSuccess)
	{
		return EXIT_FAILURE;
	}

	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputSamples =
			(double *) checkedMalloc(
					arguments.common.numberOfMonteCarloIterations * sizeof(double),
					__FILE__,
					__LINE__);
	}

	/* 
	 *	Allocate `investmentReturns` array.
	 */
	investmentReturns = (double *) checkedMalloc(
					sizeof(double) * arguments.numberOfInvestments,
					__FILE__,
					__LINE__);

	/*
	 *	Start timing if timing is enabled or in benchmarking mode.
	 */
	if ((arguments.common.isTimingEnabled) || (arguments.common.isBenchmarkingMode))
	{
		start = clock();
	}

	for (size_t i = 0; i < arguments.common.numberOfMonteCarloIterations; ++i)
	{
		/*
		 *	Load distributions for investment retruns.
		 */
		loadInvestmentReturns(&arguments, investmentReturns);

		/*
		 *	Calculate the distribution for the total portfolio return and determine statisctical quantities.
		 */
		portfolioReturn = calculatePortfolioReturn(&arguments, investmentReturns);

		/*
		 *	Doesn't calculate quantiles and probability of loss when in benchmarking mode.
		 *	Only calculates portfolio return.
		 */
		if (!arguments.common.isBenchmarkingMode)
		{
			probabilityOfLoss = 1.0 - UxHwDoubleProbabilityGT(portfolioReturn, kMoonfireVentureCapitalConstantsTotalInvestment);

			lowQuantile = UxHwDoubleQuantile(portfolioReturn, arguments.lowQuantileProbability);
			highQuantile = UxHwDoubleQuantile(portfolioReturn, arguments.highQuantileProbability);
		}

		/*
		 *	For Monte Carlo mode, save portfolioReturn.
		 */
		if (arguments.common.isMonteCarloMode)
		{
			monteCarloOutputSamples[i] = portfolioReturn;
		}
	}

	/*
	 *	If not doing Laplace version, then approximate the cost of the third phase of
	 *	Monte Carlo (post-processing), by calculating the mean and variance.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		monteCarloOutputMeanAndVariance = calculateMeanAndVarianceOfDoubleSamples(
							monteCarloOutputSamples,
							arguments.common.numberOfMonteCarloIterations);
		portfolioReturn = monteCarloOutputMeanAndVariance.mean;
	}

	/*
	 *	Stop timing if timing is enabled or in benchmarking mode.
	 */
	if ((arguments.common.isTimingEnabled) || (arguments.common.isBenchmarkingMode))
	{
		end = clock();
		cpuTimeInSeconds = ((double)(end - start)) / CLOCKS_PER_SEC;
	}

	if (arguments.common.isBenchmarkingMode)
	{
		/*
		 *	In benchmarking mode, we print:
		 *		(1) single result (for calculating Wasserstein distance to reference)
		 *		(2) time in microseconds (benchmarking setup expects cpu time in microseconds)
		 */
		printf("%lf %" PRIu64 "\n", portfolioReturn, (uint64_t)(cpuTimeInSeconds*1000000));
	}
	else
	{
		/*
		 *	Print the results in human readable format.
		 */
		if (!arguments.common.isOutputJSONMode)
		{
			printf("The forecast for the total portfolio return with portfolio size %zu is %lf times the initial total investment.\n", arguments.numberOfInvestments, portfolioReturn);

			/*
			 *	Printing probabilities in MonteCarlo Mode, does not make sense, because the values are particles.
			 */
			if (!arguments.common.isMonteCarloMode)
			{
				printf("The probability of loss for this portfolio is %"SignaloidParticleModifier"lf.\n", probabilityOfLoss);
				printf("The %"SignaloidParticleModifier"lf quantile of the total portfolio return is %"SignaloidParticleModifier"lf.\n", arguments.lowQuantileProbability, lowQuantile);
				printf("The %"SignaloidParticleModifier"lf quantile of the total portfolio return is %"SignaloidParticleModifier"lf.\n", arguments.highQuantileProbability, highQuantile);
			}
		}
		/*
		 *	Print the results in JSON format.
		 */
		else
		{
			JSONVariable variables[] = {
				{
					.variableSymbol = "portfolioReturn",
					.variableDescription = "Portfolio return (USD)",
					.values = (JSONVariablePointer) {.asDouble = &portfolioReturn} ,
					.type = kJSONVariableTypeDouble,
					.size = 1,
				}
			};

			printJSONVariables(variables, 1, "Portfolio return.");
		}

		/*
		 *	Print timing result.
		 */
		if ((arguments.common.isTimingEnabled) && (!arguments.common.isOutputJSONMode))
		{
			printf("CPU time used: %lf seconds\n", cpuTimeInSeconds);
		}
	}

	/*
	 *	Free allocated dynamic memory.
	 */
	free(investmentReturns);

	/*
	 *	Save Monte Carlo outputs in an output file and free allocated dynamic memory.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		saveMonteCarloDoubleDataToDataDotOutFile(monteCarloOutputSamples, (uint64_t)(cpuTimeInSeconds*1000000), arguments.common.numberOfMonteCarloIterations);
		free(monteCarloOutputSamples);
	}

	return EXIT_SUCCESS;
}
