/*
 * Copyright (C) 2011-2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef Options_h
#define Options_h

#include "GCLogging.h"
#include "JSExportMacros.h"
#include <stdint.h>
#include <stdio.h>
#include <wtf/StdLibExtras.h>

namespace JSC {

// How do JSC VM options work?
// ===========================
// The JSC_OPTIONS() macro below defines a list of all JSC options in use,
// along with their types and default values. The options values are actually
// realized as an array of Options::Entry elements.
//
//     Options::initialize() will initialize the array of options values with
// the defaults specified in JSC_OPTIONS() below. After that, the values can
// be programmatically read and written to using an accessor method with the
// same name as the option. For example, the option "useJIT" can be read and
// set like so:
//
//     bool jitIsOn = Options::useJIT();  // Get the option value.
//     Options::useJIT() = false;         // Sets the option value.
//
//     If you want to tweak any of these values programmatically for testing
// purposes, you can do so in Options::initialize() after the default values
// are set.
//
//     Alternatively, you can override the default values by specifying
// environment variables of the form: JSC_<name of JSC option>.
//
// Note: Options::initialize() tries to ensure some sanity on the option values
// which are set by doing some range checks, and value corrections. These
// checks are done after the option values are set. If you alter the option
// values after the sanity checks (for your own testing), then you're liable to
// ensure that the new values set are sane and reasonable for your own run.

class OptionRange {
private:
    enum RangeState { Uninitialized, InitError, Normal, Inverted };
public:
    OptionRange& operator= (const int& rhs)
    { // Only needed for initialization
        if (!rhs) {
            m_state = Uninitialized;
            m_rangeString = 0;
            m_lowLimit = 0;
            m_highLimit = 0;
        }
        return *this;
    }

    bool init(const char*);
    bool isInRange(unsigned);
    const char* rangeString() const { return (m_state > InitError) ? m_rangeString : "<null>"; }

private:
    RangeState m_state;
    const char* m_rangeString;
    unsigned m_lowLimit;
    unsigned m_highLimit;
};

typedef OptionRange optionRange;
typedef const char* optionString;

#define JSC_OPTIONS(v) \
    v(unsigned, showOptions, 0, "shows JSC options (0 = None, 1 = Overridden only, 2 = All, 3 = Verbose)") \
    \
    v(bool, useLLInt,  true, "allows the LLINT to be used if true") \
    v(bool, useJIT,    true, "allows the baseline JIT to be used if true") \
    v(bool, useDFGJIT, true, "allows the DFG JIT to be used if true") \
    v(bool, useRegExpJIT, true, "allows the RegExp JIT to be used if true") \
    \
    v(bool, reportMustSucceedExecutableAllocations, false, nullptr) \
    \
    v(unsigned, maxPerThreadStackUsage, 4 * MB, nullptr) \
    v(unsigned, reservedZoneSize, 128 * KB, nullptr) \
    v(unsigned, errorModeReservedZoneSize, 64 * KB, nullptr) \
    \
    v(bool, crashIfCantAllocateJITMemory, false, nullptr) \
    \
    v(bool, forceDFGCodeBlockLiveness, false, nullptr) \
    v(bool, forceICFailure, false, nullptr) \
    \
    v(bool, dumpGeneratedBytecodes, false, nullptr) \
    v(bool, dumpBytecodeLivenessResults, false, nullptr) \
    v(bool, validateBytecode, false, nullptr) \
    v(bool, forceDebuggerBytecodeGeneration, false, nullptr) \
    v(bool, forceProfilerBytecodeGeneration, false, nullptr) \
    \
    v(bool, enableFunctionDotArguments, true, nullptr) \
    \
    /* showDisassembly implies showDFGDisassembly. */ \
    v(bool, showDisassembly, false) \
    v(bool, asyncDisassembly, false) \
    v(bool, showDFGDisassembly, false) \
    v(bool, showFTLDisassembly, false) \
    v(bool, showAllDFGNodes, false) \
    v(optionRange, bytecodeRangeToDFGCompile, 0) \
    v(optionString, dfgFunctionWhitelistFile, nullptr) \
    v(bool, dumpBytecodeAtDFGTime, false) \
    v(bool, dumpGraphAtEachPhase, false) \
    v(bool, verboseDFGByteCodeParsing, false) \
    v(bool, verboseCompilation, false) \
    v(bool, verboseFTLCompilation, false) \
    v(bool, logCompilationChanges, false) \
    v(bool, printEachOSRExit, false) \
    v(bool, validateGraph, false) \
    v(bool, validateGraphAtEachPhase, false) \
    v(bool, verboseValidationFailure, false) \
    v(bool, verboseOSR, false) \
    v(bool, verboseFTLOSRExit, false) \
    v(bool, verboseCallLink, false) \
    v(bool, verboseCompilationQueue, false) \
    v(bool, reportCompileTimes, false) \
    v(bool, reportFTLCompileTimes, false) \
    v(bool, verboseCFA, false) \
    v(bool, verboseFTLToJSThunk, false) \
    v(bool, verboseFTLFailure, false) \
    v(bool, alwaysComputeHash, false) \
    v(bool, testTheFTL, false) \
    v(bool, verboseSanitizeStack, false) \
    v(bool, alwaysDoFullCollection, false) \
    v(bool, eagerlyUpdateTopCallFrame, false) \
    \
    v(bool, enableOSREntryToDFG, true) \
    v(bool, enableOSREntryToFTL, true) \
    \
    v(bool, useFTLJIT, true) \
    v(bool, useFTLTBAA, true) \
    v(bool, enableLLVMFastISel, false) \
    v(bool, useLLVMSmallCodeModel, false) \
    v(bool, dumpLLVMIR, false) \
    v(bool, validateFTLOSRExitLiveness, false) \
    v(bool, llvmAlwaysFailsBeforeCompile, false) \
    v(bool, llvmAlwaysFailsBeforeLink, false) \
    v(bool, llvmSimpleOpt, true) \
    v(unsigned, llvmBackendOptimizationLevel, 2) \
    v(unsigned, llvmOptimizationLevel, 2) \
    v(unsigned, llvmSizeLevel, 0) \
    v(unsigned, llvmMaxStackSize, 128 * KB) \
    v(bool, llvmDisallowAVX, true) \
    v(bool, ftlCrashes, false) /* fool-proof way of checking that you ended up in the FTL. ;-) */\
    v(bool, clobberAllRegsInFTLICSlowPath, !ASSERT_DISABLED) \
    v(bool, assumeAllRegsInFTLICAreLive, false) \
    v(bool, enableAccessInlining, true) \
    v(bool, enablePolyvariantDevirtualization, true) \
    v(bool, enablePolymorphicAccessInlining, true) \
    v(bool, enablePolymorphicCallInlining, true) \
    v(unsigned, maxPolymorphicCallVariantListSize, 15) \
    v(unsigned, maxPolymorphicCallVariantListSizeForTopTier, 5) \
    v(unsigned, maxPolymorphicCallVariantsForInlining, 5) \
    v(unsigned, frequentCallThreshold, 2) \
    v(double, minimumCallToKnownRate, 0.51) \
    v(bool, optimizeNativeCalls, false) \
    v(bool, enableObjectAllocationSinking, true) \
    \
    v(bool, enableConcurrentJIT, true) \
    v(unsigned, numberOfDFGCompilerThreads, computeNumberOfWorkerThreads(2, 2) - 1) \
    v(unsigned, numberOfFTLCompilerThreads, computeNumberOfWorkerThreads(8, 2) - 1) \
    v(int32, priorityDeltaOfDFGCompilerThreads, computePriorityDeltaOfWorkerThreads(-1, 0)) \
    v(int32, priorityDeltaOfFTLCompilerThreads, computePriorityDeltaOfWorkerThreads(-2, 0)) \
    \
    v(bool, enableProfiler, false) \
    \
    v(bool, forceUDis86Disassembler, false) \
    v(bool, forceLLVMDisassembler, false) \
    \
    v(bool, enableArchitectureSpecificOptimizations, true) \
    \
    v(bool, breakOnThrow, false) \
    \
    v(unsigned, maximumOptimizationCandidateInstructionCount, 100000) \
    \
    v(unsigned, maximumFunctionForCallInlineCandidateInstructionCount, 180) \
    v(unsigned, maximumFunctionForClosureCallInlineCandidateInstructionCount, 100) \
    v(unsigned, maximumFunctionForConstructInlineCandidateInstructionCount, 100) \
    \
    v(unsigned, maximumFTLCandidateInstructionCount, 20000) \
    \
    /* Depth of inline stack, so 1 = no inlining, 2 = one level, etc. */ \
    v(unsigned, maximumInliningDepth, 5, "maximum allowed inlining depth.  Depth of 1 means no inlining") \
    v(unsigned, maximumInliningRecursion, 2, nullptr) \
    \
    v(unsigned, maximumLLVMInstructionCountForNativeInlining, 80, nullptr) \
    \
    /* Maximum size of a caller for enabling inlining. This is purely to protect us */\
    /* from super long compiles that take a lot of memory. */\
    v(unsigned, maximumInliningCallerSize, 10000, nullptr) \
    \
    v(unsigned, maximumVarargsForInlining, 100, nullptr) \
    \
    v(bool, enablePolyvariantCallInlining, true, nullptr) \
    v(bool, enablePolyvariantByIdInlining, true, nullptr) \
    \
    v(unsigned, maximumBinaryStringSwitchCaseLength, 50, nullptr) \
    v(unsigned, maximumBinaryStringSwitchTotalLength, 2000, nullptr) \
    \
    v(int32, thresholdForJITAfterWarmUp, 500, nullptr) \
    v(int32, thresholdForJITSoon, 100, nullptr) \
    \
    v(int32, thresholdForOptimizeAfterWarmUp, 1000, nullptr) \
    v(int32, thresholdForOptimizeAfterLongWarmUp, 1000, nullptr) \
    v(int32, thresholdForOptimizeSoon, 1000, nullptr) \
    v(int32, executionCounterIncrementForLoop, 1, nullptr) \
    v(int32, executionCounterIncrementForEntry, 15, nullptr) \
    \
    v(int32, thresholdForFTLOptimizeAfterWarmUp, 100000, nullptr) \
    v(int32, thresholdForFTLOptimizeSoon, 1000, nullptr) \
    v(int32, ftlTierUpCounterIncrementForLoop, 1, nullptr) \
    v(int32, ftlTierUpCounterIncrementForReturn, 15, nullptr) \
    v(unsigned, ftlOSREntryFailureCountForReoptimization, 15, nullptr) \
    v(unsigned, ftlOSREntryRetryThreshold, 100, nullptr) \
    \
    v(int32, evalThresholdMultiplier, 10, nullptr) \
    v(unsigned, maximumEvalCacheableSourceLength, 256, nullptr) \
    \
    v(bool, randomizeExecutionCountsBetweenCheckpoints, false, nullptr) \
    v(int32, maximumExecutionCountsBetweenCheckpointsForBaseline, 1000, nullptr) \
    v(int32, maximumExecutionCountsBetweenCheckpointsForUpperTiers, 50000, nullptr) \
    \
    v(unsigned, likelyToTakeSlowCaseMinimumCount, 20, nullptr) \
    v(unsigned, couldTakeSlowCaseMinimumCount, 10, nullptr) \
    \
    v(unsigned, osrExitCountForReoptimization, 100, nullptr) \
    v(unsigned, osrExitCountForReoptimizationFromLoop, 5, nullptr) \
    \
    v(unsigned, reoptimizationRetryCounterMax, 0, nullptr)  \
    \
    v(unsigned, minimumOptimizationDelay, 1, nullptr) \
    v(unsigned, maximumOptimizationDelay, 5, nullptr) \
    v(double, desiredProfileLivenessRate, 0.75, nullptr) \
    v(double, desiredProfileFullnessRate, 0.35, nullptr) \
    \
    v(double, doubleVoteRatioForDoubleFormat, 2, nullptr) \
    v(double, structureCheckVoteRatioForHoisting, 1, nullptr) \
    v(double, checkArrayVoteRatioForHoisting, 1, nullptr) \
    \
    v(unsigned, minimumNumberOfScansBetweenRebalance, 100, nullptr) \
    v(unsigned, numberOfGCMarkers, computeNumberOfGCMarkers(7), nullptr) \
    v(unsigned, opaqueRootMergeThreshold, 1000, nullptr) \
    v(double, minHeapUtilization, 0.8, nullptr) \
    v(double, minCopiedBlockUtilization, 0.9, nullptr) \
    v(double, minMarkedBlockUtilization, 0.9, nullptr) \
    v(unsigned, slowPathAllocsBetweenGCs, 0, "force a GC on every Nth slow path alloc, where N is specified by this option") \
    \
    v(double, percentCPUPerMBForFullTimer, 0.0003125, nullptr) \
    v(double, percentCPUPerMBForEdenTimer, 0.0025, nullptr) \
    v(double, collectionTimerMaxPercentCPU, 0.05, nullptr) \
    \
    v(bool, forceWeakRandomSeed, false, nullptr) \
    v(unsigned, forcedWeakRandomSeed, 0, nullptr) \
    \
    v(bool, useZombieMode, false, "debugging option to scribble over dead objects with 0xdeadbeef") \
    v(bool, objectsAreImmortal, false, "debugging option to keep all objects alive forever") \
    v(bool, showObjectStatistics, false, nullptr) \
    \
    v(gcLogLevel, logGC, GCLogging::None, "debugging option to log GC activity (0 = None, 1 = Basic, 2 = Verbose)") \
    v(bool, disableGC, false, nullptr) \
    v(unsigned, gcMaxHeapSize, 0, nullptr) \
    v(bool, recordGCPauseTimes, false, nullptr) \
    v(bool, logHeapStatisticsAtExit, false, nullptr) \
    v(bool, enableTypeProfiler, false, nullptr) \
    v(bool, enableControlFlowProfiler, false, nullptr) \
    \
    v(bool, verifyHeap, false, nullptr) \
    v(unsigned, numberOfGCCyclesToRecordForVerification, 3, nullptr) \
    \
    v(bool, enableExceptionFuzz, false, nullptr) \
    v(unsigned, fireExceptionFuzzAt, 0, nullptr) \
    \
    v(bool, enableExecutableAllocationFuzz, false, nullptr) \
    v(unsigned, fireExecutableAllocationFuzzAt, 0, nullptr) \
    v(unsigned, fireExecutableAllocationFuzzAtOrAfter, 0, nullptr) \
    v(bool, verboseExecutableAllocationFuzz, false, nullptr)

class Options {
public:
    enum class DumpLevel {
        None = 0,
        Overridden,
        All,
        Verbose
    };
    
    // This typedef is to allow us to eliminate the '_' in the field name in
    // union inside Entry. This is needed to keep the style checker happy.
    typedef int32_t int32;

    // Declare the option IDs:
    enum OptionID {
#define FOR_EACH_OPTION(type_, name_, defaultValue_, description_) \
        name_##ID,
        JSC_OPTIONS(FOR_EACH_OPTION)
#undef FOR_EACH_OPTION
        numberOfOptions
    };

    enum class Type {
        boolType,
        unsignedType,
        doubleType,
        int32Type,
        optionRangeType,
        optionStringType,
        gcLogLevelType,
    };

    static void initialize();

    // Parses a single command line option in the format "<optionName>=<value>"
    // (no spaces allowed) and set the specified option if appropriate.
    JS_EXPORT_PRIVATE static bool setOption(const char* arg);
    JS_EXPORT_PRIVATE static void dumpAllOptions(DumpLevel, const char* title = nullptr, FILE* stream = stdout);
    static void dumpOption(DumpLevel, OptionID, FILE* stream = stdout, const char* header = "", const char* footer = "");

    // Declare accessors for each option:
#define FOR_EACH_OPTION(type_, name_, defaultValue_, description_) \
    ALWAYS_INLINE static type_& name_() { return s_options[name_##ID].type_##Val; } \
    ALWAYS_INLINE static type_& name_##Default() { return s_defaultOptions[name_##ID].type_##Val; }

    JSC_OPTIONS(FOR_EACH_OPTION)
#undef FOR_EACH_OPTION

private:
    // For storing for an option value:
    union Entry {
        bool boolVal;
        unsigned unsignedVal;
        double doubleVal;
        int32 int32Val;
        OptionRange optionRangeVal;
        const char* optionStringVal;
        GCLogging::Level gcLogLevelVal;
    };

    // For storing constant meta data about each option:
    struct EntryInfo {
        const char* name;
        const char* description;
        Type type;
    };

    Options();

    // Declare the singleton instance of the options store:
    JS_EXPORTDATA static Entry s_options[numberOfOptions];
    static Entry s_defaultOptions[numberOfOptions];
    static const EntryInfo s_optionsInfo[numberOfOptions];

    friend class Option;
};

class Option {
public:
    Option(Options::OptionID id)
        : m_id(id)
        , m_entry(Options::s_options[m_id])
    {
    }
    
    void dump(FILE*) const;
    bool operator==(const Option& other) const;
    bool operator!=(const Option& other) const { return !(*this == other); }
    
    const char* name() const;
    const char* description() const;
    Options::Type type() const;
    bool isOverridden() const;
    const Option defaultOption() const;
    
    bool& boolVal();
    unsigned& unsignedVal();
    double& doubleVal();
    int32_t& int32Val();
    OptionRange optionRangeVal();
    const char* optionStringVal();
    GCLogging::Level& gcLogLevelVal();
    
private:
    // Only used for constructing default Options.
    Option(Options::OptionID id, Options::Entry& entry)
        : m_id(id)
        , m_entry(entry)
    {
    }
    
    Options::OptionID m_id;
    Options::Entry& m_entry;
};

inline const char* Option::name() const
{
    return Options::s_optionsInfo[m_id].name;
}

inline const char* Option::description() const
{
    return Options::s_optionsInfo[m_id].description;
}

inline Options::Type Option::type() const
{
    return Options::s_optionsInfo[m_id].type;
}

inline bool Option::isOverridden() const
{
    return *this != defaultOption();
}

inline const Option Option::defaultOption() const
{
    return Option(m_id, Options::s_defaultOptions[m_id]);
}

inline bool& Option::boolVal()
{
    return m_entry.boolVal;
}

inline unsigned& Option::unsignedVal()
{
    return m_entry.unsignedVal;
}

inline double& Option::doubleVal()
{
    return m_entry.doubleVal;
}

inline int32_t& Option::int32Val()
{
    return m_entry.int32Val;
}

inline OptionRange Option::optionRangeVal()
{
    return m_entry.optionRangeVal;
}

inline const char* Option::optionStringVal()
{
    return m_entry.optionStringVal;
}

inline GCLogging::Level& Option::gcLogLevelVal()
{
    return m_entry.gcLogLevelVal;
}

} // namespace JSC

#endif // Options_h
