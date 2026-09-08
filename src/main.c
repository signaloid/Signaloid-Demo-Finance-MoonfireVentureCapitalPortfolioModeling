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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <uxhw.h>
#include "utilities.h"
#include "kernel.h"
#include "common.h"

int
main(int argc, char *  argv[])
{
	CommandLineArguments        arguments = { 0 };
	double                      output;
	double *                    monteCarloOutputSamples = NULL;
	clock_t                     start   = 0;
	clock_t                     end     = 0;
	double                      cpuTimeInSeconds;
	double                      outputVariables[kOutputVariableIndexMax];
	const char *                applicationDescription = "Moonfire Venture Capital Portfolio Modeling";
	const char *                outputVariableNames[kOutputVariableIndexMax] = {
		[kOutputVariableIndexPortfolioReturn]   = "portfolioReturn",
		[kOutputVariableIndexProbabilityOfLoss] = "probabilityOfLoss",
		[kOutputVariableIndexLowQuantile]       = "lowQuantile",
		[kOutputVariableIndexHighQuantile]      = "highQuantile",
	};
	const char *                outputVariableDescriptions[kOutputVariableIndexMax] = {
		[kOutputVariableIndexPortfolioReturn]   = "Investment returns of the portfolio",
		[kOutputVariableIndexProbabilityOfLoss] = "Probability of portfolio loss",
		[kOutputVariableIndexLowQuantile]       = "Low quantile specified by CLA",
		[kOutputVariableIndexHighQuantile]      = "High quantile specified by CLA",
	};
	kOutputVariableTypeIndex    outputVariableTypes[kOutputVariableIndexMax] = {
		[kOutputVariableIndexPortfolioReturn]   = kOutputVariableTypeDistribution,
		[kOutputVariableIndexProbabilityOfLoss] = kOutputVariableTypeScalar,
		[kOutputVariableIndexLowQuantile]       = kOutputVariableTypeScalar,
		[kOutputVariableIndexHighQuantile]      = kOutputVariableTypeScalar,
	};
	MeanAndVariance             meanAndVariance;

	/*
	 *	Get command-line arguments.
	 */
	if (getCommandLineArguments(argc, argv, &arguments) != kCommonConstantReturnTypeSuccess)
	{
		return EXIT_FAILURE;
	}

	monteCarloOutputSamples =
		(double *) checkedMalloc(
			arguments.common.numberOfMonteCarloIterations * sizeof(double),
			__FILE__,
			__LINE__
		);

	/*
	 *	Start timing if timing is enabled.
	 */
	if (arguments.common.isTimingEnabled)
	{
		start = clock();
	}

	bool isSelectedOutputScalar = (arguments.common.outputSelect != kOutputVariableIndexMax) &&
	                              (outputVariableTypes[arguments.common.outputSelect] == kOutputVariableTypeScalar);

	if (arguments.common.isMonteCarloMode)
	{
		output = calculateOutputMonteCarlo(&arguments, outputVariables, monteCarloOutputSamples);

		/*
		 *	If not doing UxHw version, then approximate the cost of the third phase of
		 *	Monte Carlo (post-processing), by calculating the mean and variance.
		 */
		if (!isSelectedOutputScalar)
		{
			meanAndVariance = calculateMeanAndVarianceOfDoubleSamples(monteCarloOutputSamples, arguments.common.numberOfMonteCarloIterations);
			output          = outputVariables[arguments.common.outputSelect] = meanAndVariance.mean;
		}
	}
	else
	{
		output = calculateOutputUxHw(&arguments, outputVariables, monteCarloOutputSamples);
	}

	/*
	 *	Stop timing if timing is enabled.
	 */
	if (arguments.common.isTimingEnabled)
	{
		end                 = clock();
		cpuTimeInSeconds    = ((double) (end - start)) / CLOCKS_PER_SEC;
	}

	CommonCommandLineArguments printArguments = arguments.common;

	if (arguments.common.isMonteCarloMode && isSelectedOutputScalar)
	{
		printArguments.isMonteCarloMode             = false;
		printArguments.numberOfMonteCarloIterations = 1;
	}

	/*
	 *	Print the results in JSON format.
	 */
	if (arguments.common.isOutputJSONMode)
	{
		printJSONFormattedOutput(
			&printArguments,
			monteCarloOutputSamples,
			outputVariables,
			outputVariableDescriptions,
			kOutputVariableIndexMax,
			applicationDescription
		);
	}
	/*
	 *	Print the results in human readable format.
	 */
	else
	{
		printHumanConsumableOutput(
			&printArguments,
			kOutputVariableIndexMax,
			outputVariables,
			outputVariableNames,
			outputVariableDescriptions,
			monteCarloOutputSamples
		);
	}

	/*
	 *	Print timing result.
	 */
	if ((arguments.common.isTimingEnabled) && (!arguments.common.isOutputJSONMode))
	{
		printf("CPU time used: %" SignaloidParticleModifier "lf seconds\n", cpuTimeInSeconds);
	}

	/*
	 *	Save Monte carlo outputs in an output file.
	 */
	if (arguments.common.isMonteCarloMode)
	{
		size_t samplesToSave = isSelectedOutputScalar
		                ? 1
		                : arguments.common.numberOfMonteCarloIterations;

		saveMonteCarloDoubleDataToDataDotOutFile(
			monteCarloOutputSamples,
			(uint64_t) (cpuTimeInSeconds * 1000000),
			samplesToSave
		);
	}

	/*
	 *	Free dynamically-allocated memory.
	 */
	free(monteCarloOutputSamples);

	return EXIT_SUCCESS;
}
