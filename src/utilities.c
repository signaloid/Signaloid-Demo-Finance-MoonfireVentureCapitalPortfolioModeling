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

#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <inttypes.h>
#include <uxhw.h>
#include "utilities.h"


const double	kDefaultValuesAlpha			= 1.05;
const double	kDefaultValuesXMin			= 0.35;
const double	kDefaultValuesXMax			= 1000.0;
const double	kDefaultValuesLowQuantileProbability	= 0.01;
const double	kDefaultValuesHighQuantileProbability	= 0.99;

void
printUsage(void)
{
	fprintf(stderr, "Example: Moonfire Venture Capital Portfolio Modeling - Signaloid version\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: Valid command-line arguments are:\n");
	fprintf(
		stderr,
		"\t[-o, --output <Path to output CSV file : str>] (Specify the output file.)\n"
		"\t[-S, --select-output <output : int> (Default: 0)] (Compute 0-indexed output.)\n"
		"\t[-M, --multiple-executions <Number of executions : int> (Default: 1)] (Repeated execute kernel for benchmarking.)\n"
		"\t[-T, --time] (Timing mode: Times and prints the timing of the kernel execution.)\n"
		"\t[-b, --benchmarking] (Benchmarking mode: Generate outputs in format for benchmarking.)\n"
		"\t[-j, --json] (Print output in JSON format.)\n"
		"\t[-h, --help] (Display this help message.)\n"
		"\t[-a, --alpha-pareto <Portfolio return bounded Pareto distribution parameter 'alpha': double in (0, inf)> (Default: %"SignaloidParticleModifier".2lf)]\n"
		"\t[-x, --xMin-pareto <Portfolio return bounded Pareto distribution parameter 'xMin': double in (0, xMax]> (Default: %"SignaloidParticleModifier".2lf)]\n"
		"\t[-X, --xMax-pareto <Portfolio return bounded Pareto distribution parameter 'xMax': double in [xMin, inf)> (Default: %"SignaloidParticleModifier".2lf)]\n"
		"\t[-n, --number-of-investments <Number of investments in portfolio: size_t in [1, inf)> (Default: %zu)]\n"
		"\t[-q, --low-quantile-probability <Low quantile probability: double in (0, 1)> (Default: %"SignaloidParticleModifier".2lf)]\n"
		"\t[-Q, --high-quantile-probability <High quantile probability: double in (0, 1)]> (Default: %"SignaloidParticleModifier".2lf)]\n",
		kDefaultValuesAlpha,
		kDefaultValuesXMin,
		kDefaultValuesXMax,
		(size_t)kDefaultValuesNumberOfInvestements,
		kDefaultValuesLowQuantileProbability,
		kDefaultValuesHighQuantileProbability);
	fprintf(stderr, "\n");

	return;
}

/**
 *	@brief	Set the default values for the command-line arguments.
 *
 *	@param	arguments	: command-line arguments pointer.
 *	@return			: `kCommonConstantReturnTypeSuccess` if successful, else `kCommonConstantReturnTypeError`.
 */
static CommonConstantReturnType
setDefaultCommandLineArguments(CommandLineArguments *  arguments)
{
	if (arguments == NULL)
	{
		fprintf(stderr, "Error: Arguments pointer is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Older GCC versions have a bug which gives a spurious warning for
	 *	the C universal zero initializer `{0}`. Any workaround makes the
	 *	code less portable or prevents the common code from adding new
	 *	fields to the `CommonCommandLineArguments` struct. Therefore, we
	 *	surpress this warning.
	 *
	 *	See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119.
	 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"

	*arguments = (CommandLineArguments)
	{
		.common				= (CommonCommandLineArguments) {0},
		.alpha				= kDefaultValuesAlpha,
		.xMin				= kDefaultValuesXMin,
		.xMax				= kDefaultValuesXMax,
		.numberOfInvestments		= kDefaultValuesNumberOfInvestements,
		.lowQuantileProbability		= kDefaultValuesLowQuantileProbability,
		.highQuantileProbability	= kDefaultValuesHighQuantileProbability,
	};
#pragma GCC diagnostic pop

	return kCommonConstantReturnTypeSuccess;
}

CommonConstantReturnType
getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments)
{
	const char *	alphaArg = NULL;
	const char *	xMinArg = NULL;
	const char *	xMaxArg = NULL;
	const char *	numberOfInvestmentsArg = NULL;
	const char *	lowQuantileProbabilityArg = NULL;
	const char *	highQuantileProbabilityArg = NULL;

	if (arguments == NULL)
	{
		fprintf(stderr, "Error: The provided pointer to arguments is NULL.\n");

		return kCommonConstantReturnTypeError;
	}

	if (setDefaultCommandLineArguments(arguments) != kCommonConstantReturnTypeSuccess)
	{
		return kCommonConstantReturnTypeError;
	}

	DemoOption	options[] =
	{
		{ .opt = "a", .optAlternative = "alpha-pareto",			.hasArg = true, .foundArg = &alphaArg,				.foundOpt = NULL },
		{ .opt = "x", .optAlternative = "xMin-pareto",			.hasArg = true, .foundArg = &xMinArg,				.foundOpt = NULL },
		{ .opt = "X", .optAlternative = "xMax-pareto",			.hasArg = true, .foundArg = &xMaxArg,				.foundOpt = NULL },
		{ .opt = "n", .optAlternative = "number-of-investments",	.hasArg = true, .foundArg = &numberOfInvestmentsArg,		.foundOpt = NULL },
		{ .opt = "q", .optAlternative = "low-quantile-probability",	.hasArg = true, .foundArg = &lowQuantileProbabilityArg,		.foundOpt = NULL },
		{ .opt = "Q", .optAlternative = "high-quantile-probability",	.hasArg = true, .foundArg = &highQuantileProbabilityArg,	.foundOpt = NULL },
		{0},
	};

	if (parseArgs(argc, argv, &arguments->common, options) != kCommonConstantReturnTypeSuccess)
	{
		fprintf(stderr, "Parsing command-line arguments failed\n");
		printUsage();

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isHelpEnabled)
	{
		printUsage();

		exit(EXIT_SUCCESS);
	}

	if (arguments->common.isWriteToFileEnabled)
	{
		fprintf(stderr, "Error: This application does not support saving outputs to file.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isInputFromFileEnabled)
	{
		fprintf(stderr, "Error: This application does not support reading inputs from file.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isOutputSelected)
	{
		fprintf(stderr, "Error: Output select option not supported.\n");

		return kCommonConstantReturnTypeError;
	}

	if (arguments->common.isVerbose)
	{
		fprintf(stderr, "Error: Verbose mode not supported.\n");

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Typecheck alpha.
	 */
	if (alphaArg != NULL)
	{
		double	alpha;
		int	ret = parseDoubleChecked(alphaArg, &alpha);

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The alpha pareto distribution parameter(-a) must be a real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		if (alpha < 0)
		{
			fprintf(stderr, "Error: The alpha pareto distribution parameter(-a) must be a positive real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->alpha = alpha;
	}

	/*
	 *	Typecheck xMin.
	 */
	if (xMinArg != NULL)
	{
		double	xMin;
		int	ret = parseDoubleChecked(xMinArg, &xMin);

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The xMin pareto distribution parameter(-x) must be a real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		if (xMin < 0)
		{
			fprintf(stderr, "Error: The xMin pareto distribution parameter(-x) must be a positive real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->xMin = xMin;
	}

	/*
	 *	Typecheck xMax.
	 */
	if (xMaxArg != NULL)
	{
		double	xMax;
		int	ret = parseDoubleChecked(xMaxArg, &xMax);

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The xMax pareto distribution parameter(-X) must be a real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		if (xMax < 0)
		{
			fprintf(stderr, "Error: The xMax pareto distribution parameter(-X) must be a positive real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->xMax = xMax;
	}

	if (arguments->xMax < arguments->xMin)
	{
		fprintf(stderr, "Error: The xMax pareto distribution parameter(-X) cannot be smaller than the xMin pareto distribution paramete(-x).\n");
		printUsage();

		return kCommonConstantReturnTypeError;
	}

	/*
	 *	Typecheck numberOfInvestments.
	 */
	if (numberOfInvestmentsArg != NULL)
	{
		int	numberOfInvestments;
		int	ret = parseIntChecked(numberOfInvestmentsArg, &numberOfInvestments);

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The number of investments parameter(-n) must be an integer number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		if (numberOfInvestments < 1)
		{
			fprintf(stderr, "Error: The number of investments parameter(-n) must be >= 1\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->numberOfInvestments = (size_t) numberOfInvestments;
	}

	/*
	 *	Typecheck lowQuantileProbability.
	 */
	if (lowQuantileProbabilityArg != NULL)
	{
		double	lowQuantileProbability;
		int	ret = parseDoubleChecked(lowQuantileProbabilityArg, &lowQuantileProbability);

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The low quantile probability parameter(-q) must be a real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		if ((lowQuantileProbability <= 0) || (lowQuantileProbability >= 1))
		{
			fprintf(stderr, "Error: The The low quantile probability parameter(-q) must be a value in [0, 1]\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->lowQuantileProbability = lowQuantileProbability;
	}

	/*
	 *	Typecheck highQuantileProbability.
	 */
	if (highQuantileProbabilityArg != NULL)
	{
		double	highQuantileProbability;
		int	ret = parseDoubleChecked(highQuantileProbabilityArg, &highQuantileProbability);

		if (ret != kCommonConstantReturnTypeSuccess)
		{
			fprintf(stderr, "Error: The high quantile probability parameter(-Q) must be a real number.\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		if ((highQuantileProbability <= 0) || (highQuantileProbability >= 1))
		{
			fprintf(stderr, "Error: The high quantile probability parameter(-Q) must be a must be a value between (0, 1)\n");
			printUsage();

			return kCommonConstantReturnTypeError;
		}

		arguments->highQuantileProbability = highQuantileProbability;
	}

	if (arguments->highQuantileProbability < arguments->lowQuantileProbability)
	{
		fprintf(stderr, "Error: The high quantile probability parameter(-Q) cannot be smaller than the low quantile probability parameter(-q).\n");
		printUsage();

		return kCommonConstantReturnTypeError;
	}

	return kCommonConstantReturnTypeSuccess;
}
