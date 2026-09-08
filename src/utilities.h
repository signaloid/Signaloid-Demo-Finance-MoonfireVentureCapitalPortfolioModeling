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
#include <stdbool.h>
#include "common.h"

typedef enum
{
	kOutputVariableIndexPortfolioReturn     = 0,
	kOutputVariableIndexProbabilityOfLoss   = 1,
	kOutputVariableIndexLowQuantile         = 2,
	kOutputVariableIndexHighQuantile        = 3,
	kOutputVariableIndexMax,
} OutputVariableIndex;

typedef enum
{
	kDefaultValuesNumberOfInvestements = 150,
} DefaultValues;

typedef struct
{
	CommonCommandLineArguments  common;
	double                      alpha;
	double                      xMin;
	double                      xMax;
	size_t                      numberOfInvestments;
	double                      lowQuantileProbability;
	double                      highQuantileProbability;
} CommandLineArguments;

/**
 *	@brief	Print out command-line usage.
 */
void
printUsage(void);

/**
 *	@brief	Get command-line arguments.
 *
 *	@param	argc		: Argument count from `main()`.
 *	@param	argv		: Argument vector from `main()`.
 *	@param	arguments	: Pointer to struct to store arguments.
 *	@return			: `kCommonConstantSuccess` if successful, else `kCommonConstantError`.
 */
CommonConstantReturnType
getCommandLineArguments(int argc, char *  argv[], CommandLineArguments *  arguments);

/*
 *	`determineIndexRangeOfSelectedOutputs()`, `printHumanConsumableOutput()`,
 *	`populateJSONVariableStruct()` and `printJSONFormattedOutput()` are now
 *	provided generically by the `common` submodule (see `common.h`), which
 *	operates on `CommonCommandLineArguments *` and plain `size_t` indices
 *	instead of this demo's local `CommandLineArguments *` /
 *	`OutputVariableIndex`. This demo previously carried its own
 *	same-named local versions of these functions with incompatible
 *	signatures, which no longer compiles against the current `common`
 *	submodule; `main()` now calls the generic versions directly instead.
 */
