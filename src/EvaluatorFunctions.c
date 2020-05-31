/** @file
 * Evaluator Functions.
 */

/*
 * Copyright (C) 2011 Ramshankar (aka Teknomancer)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*******************************************************************************
 *   Header Files                                                              *
 *******************************************************************************/
#include "Evaluator.h"
#include "EvaluatorFunctions.h"
#include "Errors.h"
#include "GenericDefs.h"
#include "StringOps.h"
#include "InputOutput.h"


/*******************************************************************************
 *   Function Functions!                                                       *
 *******************************************************************************/
static int FnSum(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    for (uint32_t i = 1; i < cTokens; i++)
    {
        apTokens[0]->u.Number.uValue += apTokens[i]->u.Number.uValue;
        apTokens[0]->u.Number.dValue += apTokens[i]->u.Number.dValue;
    }
    return RINF_SUCCESS;
}

static int FnAverage(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    int rc = FnSum(pEval, apTokens, cTokens);
    if (RC_SUCCESS(rc))
    {
        apTokens[0]->u.Number.uValue /= cTokens;
        apTokens[0]->u.Number.dValue /= cTokens;
    }
    return RINF_SUCCESS;
}

static int FnFactorial(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    uint64_t uValue = apTokens[0]->u.Number.uValue;
    uint64_t uFact  = 1;
    while (uValue > 1)
    {
        uFact *= uValue;
        --uValue;
    }
    apTokens[0]->u.Number.uValue = uFact;
    apTokens[0]->u.Number.dValue = uFact;
    return RINF_SUCCESS;
}

static int FnGCD(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    /*
     * Bleh, just write something quickly now. I'll revisit this to make it
     * more efficient later.
     */
    uint64_t uMin = apTokens[0]->u.Number.uValue;
    for (uint32_t i = 1; i < cTokens; i++)
    {
        uint64_t uCur = apTokens[i]->u.Number.uValue;
        if (uMin < uCur)
            uMin = uCur;
    }

    bool fSet = false;
    for (uint64_t u = uMin; u > 0; u--)
    {
        if (uMin % u == 0)
        {
            uint32_t cFactors = 0;
            for (uint64_t k = 0; k < cTokens; k++)
            {
                if (apTokens[k]->u.Number.uValue % u == 0)
                    cFactors ++;
            }
            if (cFactors == cTokens)
            {
                apTokens[0]->u.Number.uValue = u;
                apTokens[0]->u.Number.dValue = u;
                fSet = true;
                break;
            }
        }
    }
    if (!fSet)
    {
        apTokens[0]->u.Number.uValue = 1;
        apTokens[0]->u.Number.dValue = 1;
    }
    return RINF_SUCCESS;
}

static int FnLCM(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    /*
     * Bleh, just write something quickly now. I'll revisit this to make it
     * more efficient later.
     */
    uint64_t uMax = apTokens[0]->u.Number.uValue;
    uint64_t uProduct = 1;
    for (uint32_t i = 0; i < cTokens; i++)
    {
        uint64_t uCur = apTokens[i]->u.Number.uValue;
        uProduct *= uCur;
        if (uMax > uCur)
            uMax = uCur;
    }

    bool fSet = false;
    for (uint64_t u = uMax; u <= uProduct; u += uMax)
    {
        uint32_t cFactors = 0;
        for (uint64_t k = 0; k < cTokens; k++)
        {
            if (u % apTokens[k]->u.Number.uValue == 0)
                cFactors++;
        }
        if (cFactors == cTokens)
        {
            apTokens[0]->u.Number.uValue = u;
            apTokens[0]->u.Number.dValue = u;
            fSet = true;
            break;
        }
    }
    if (!fSet)
    {
        apTokens[0]->u.Number.uValue = 1;
        apTokens[0]->u.Number.dValue = 1;
    }
    return RINF_SUCCESS;
}


static int FnPow(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.dValue = powl(apTokens[0]->u.Number.dValue, apTokens[1]->u.Number.dValue);
    apTokens[0]->u.Number.uValue = (uint64_t)(apTokens[0]->u.Number.dValue + 0.50);
    return RINF_SUCCESS;
}

static int FnSqrt(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.dValue = powl(apTokens[0]->u.Number.dValue, (long double)0.50);
    apTokens[0]->u.Number.uValue = (uint64_t)(apTokens[0]->u.Number.dValue + 0.50);
    return RINF_SUCCESS;
}

static int FnRoot(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.dValue = powl(apTokens[0]->u.Number.dValue, 1 / (long double)apTokens[1]->u.Number.dValue);
    apTokens[0]->u.Number.uValue = (uint64_t)(apTokens[0]->u.Number.dValue + 0.50);
    return RINF_SUCCESS;
}

static int FnIf(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    uint8_t const idxResultToken = apTokens[0]->u.Number.uValue ? 1 : 2;
    apTokens[0]->u.Number.uValue = apTokens[idxResultToken]->u.Number.uValue;
    apTokens[0]->u.Number.dValue = apTokens[idxResultToken]->u.Number.dValue;
    return RINF_SUCCESS;
}


static int FnByteToPage(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    /** @todo Make Functions specify minimum parameter widths like Operators. */
    apTokens[0]->u.Number.uValue = (apTokens[0]->u.Number.uValue + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnByteToKiloByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1K;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnByteToMegaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1M;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnByteToGigaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1G;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnByteToTeraByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1T;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}


static int FnKiloByteToByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1K;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnKiloByteToMegaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnByteToKiloByte(pEval, apTokens, cTokens);
}

static int FnKiloByteToGigaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnByteToMegaByte(pEval, apTokens, cTokens);
}

static int FnKiloByteToTeraByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnByteToGigaByte(pEval, apTokens, cTokens);
}

static int FnKiloByteToPage(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue = (apTokens[0]->u.Number.uValue * _1K + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}


static int FnMegaByteToByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1M;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnMegaByteToKiloByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1K;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnMegaByteToGigaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnByteToKiloByte(pEval, apTokens, cTokens);
}

static int FnMegaByteToTeraByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnByteToMegaByte(pEval, apTokens, cTokens);
}

static int FnMegaByteToPage(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue = (apTokens[0]->u.Number.uValue * _1M + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}


static int FnGigaByteToByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1G;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnGigaByteToKiloByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnMegaByteToByte(pEval, apTokens, cTokens);
}

static int FnGigaByteToMegaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnMegaByteToKiloByte(pEval, apTokens, cTokens);
}

static int FnGigaByteToTeraByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnByteToKiloByte(pEval, apTokens, cTokens);
}

static int FnGigaByteToPage(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue = (apTokens[0]->u.Number.uValue * _1G + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnTeraByteToByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1T;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnTeraByteToKiloByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnGigaByteToByte(pEval, apTokens, cTokens);
}

static int FnTeraByteToMegaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnMegaByteToByte(pEval, apTokens, cTokens);
}

static int FnTeraByteToGigaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return FnKiloByteToByte(pEval, apTokens, cTokens);
}

static int FnTeraByteToPage(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue = (apTokens[0]->u.Number.uValue * _1T + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnPageToByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _MEM_PAGESIZE;
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnPageToKiloByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= (_MEM_PAGESIZE / _1K);
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnPageToMegaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= (_MEM_PAGESIZE / _1M);
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnPageToGigaByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= (_MEM_PAGESIZE / _1G);
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnPageToTeraByte(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= (_MEM_PAGESIZE / _1T);
    apTokens[0]->u.Number.dValue  = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}



static int FnNanosecToMicrosec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1MILLI;
    apTokens[0]->u.Number.dValue /= _1MILLI;
    return RINF_SUCCESS;
}

static int FnNanosecToMillisec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1MICRO;
    apTokens[0]->u.Number.dValue /= _1MICRO;
    return RINF_SUCCESS;
}

static int FnNanosecToSecond(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1NANO;
    apTokens[0]->u.Number.dValue /= _1NANO;
    return RINF_SUCCESS;
}

static int FnNanosecToMinute(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (60 * _1NANO);
    apTokens[0]->u.Number.dValue /= (60 * _1NANO);
    return RINF_SUCCESS;
}

static int FnNanosecToHour(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (3600LL * _1NANO);
    apTokens[0]->u.Number.dValue /= ((long double)3600LL * _1NANO);
    return RINF_SUCCESS;
}

static int FnNanosecToDay(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (86400LL * _1NANO);
    apTokens[0]->u.Number.dValue /= ((long double)86400LL * _1NANO);
    return RINF_SUCCESS;
}

static int FnNanosecToWeek(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (604800LL * _1NANO);
    apTokens[0]->u.Number.dValue /= ((long double)604800LL * _1NANO);
    return RINF_SUCCESS;
}

static int FnNanosecToYear(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (31556926LL * _1NANO);
    apTokens[0]->u.Number.dValue /= ((long double)31556926LL * _1NANO);
    return RINF_SUCCESS;
}

static int FnMicrosecToNanosec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1MILLI;
    apTokens[0]->u.Number.dValue *= _1MILLI;
    return RINF_SUCCESS;
}

static int FnMicrosecToMillisec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1MILLI;
    apTokens[0]->u.Number.dValue /= _1MILLI;
    return RINF_SUCCESS;
}


static int FnMicrosecToSecond(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1MICRO;
    apTokens[0]->u.Number.dValue /= _1MICRO;
    return RINF_SUCCESS;
}


static int FnMicrosecToMinute(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (60 * _1MICRO);
    apTokens[0]->u.Number.dValue /= (60 * _1MICRO);
    return RINF_SUCCESS;
}

static int FnMicrosecToHour(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (3600LL * _1MICRO);
    apTokens[0]->u.Number.dValue /= ((long double)3600LL * _1MICRO);
    return RINF_SUCCESS;
}

static int FnMicrosecToDay(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (86400LL * _1MICRO);
    apTokens[0]->u.Number.dValue /= ((long double)86400LL * _1MICRO);
    return RINF_SUCCESS;
}

static int FnMicrosecToWeek(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (604800LL * _1MICRO);
    apTokens[0]->u.Number.dValue /= ((long double)604800LL * _1MICRO);
    return RINF_SUCCESS;
}

static int FnMicrosecToYear(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (31556926LL * _1MICRO);
    apTokens[0]->u.Number.dValue /= ((long double)31556926LL * _1MICRO);
    return RINF_SUCCESS;
}


static int FnMillisecToNanosec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1MICRO;
    apTokens[0]->u.Number.dValue *= _1MICRO;
    return RINF_SUCCESS;
}

static int FnMillisecToMicrosec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1MILLI;
    apTokens[0]->u.Number.dValue *= _1MILLI;
    return RINF_SUCCESS;
}


static int FnMillisecToSecond(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= _1MILLI;
    apTokens[0]->u.Number.dValue /= _1MILLI;
    return RINF_SUCCESS;
}


static int FnMillisecToMinute(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (60 * _1MILLI);
    apTokens[0]->u.Number.dValue /= (60 * _1MILLI);
    return RINF_SUCCESS;
}

static int FnMillisecToHour(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (3600LL * _1MILLI);
    apTokens[0]->u.Number.dValue /= ((long double)3600LL * _1MILLI);
    return RINF_SUCCESS;
}

static int FnMillisecToDay(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (86400LL * _1MILLI);
    apTokens[0]->u.Number.dValue /= ((long double)86400LL * _1MILLI);
    return RINF_SUCCESS;
}

static int FnMillisecToWeek(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (604800LL * _1MILLI);
    apTokens[0]->u.Number.dValue /= ((long double)604800LL * _1MILLI);
    return RINF_SUCCESS;
}

static int FnMillisecToYear(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= (31556926LL * _1MILLI);
    apTokens[0]->u.Number.dValue /= ((long double)31556926LL * _1MILLI);
    return RINF_SUCCESS;
}


static int FnSecondToNanosec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1NANO;
    apTokens[0]->u.Number.dValue *= _1NANO;
    return RINF_SUCCESS;
}

static int FnSecondToMicrosec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1MICRO;
    apTokens[0]->u.Number.dValue *= _1MICRO;
    return RINF_SUCCESS;
}

static int FnSecondToMillisec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= _1MILLI;
    apTokens[0]->u.Number.dValue *= _1MILLI;
    return RINF_SUCCESS;
}

static int FnSecondToMinute(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= 60LL;
    apTokens[0]->u.Number.dValue /= 60LL;
    return RINF_SUCCESS;
}

static int FnSecondToHour(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= 3600LL;
    apTokens[0]->u.Number.dValue /= (long double)3600LL;
    return RINF_SUCCESS;
}

static int FnSecondToDay(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= 86400LL;
    apTokens[0]->u.Number.dValue /= (long double)86400LL;
    return RINF_SUCCESS;
}

static int FnSecondToWeek(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= 604800LL;
    apTokens[0]->u.Number.dValue /= (long double)604800LL;
    return RINF_SUCCESS;
}

static int FnSecondToYear(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= 31556926LL;
    apTokens[0]->u.Number.dValue /= (long double)31556926LL;
    return RINF_SUCCESS;
}

static int FnMinuteToNanosec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.dValue *= 60 * _1NANO;
    apTokens[0]->u.Number.dValue *= 60 * _1NANO;
    return RINF_SUCCESS;
}

static int FnMinuteToMicrosec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= 60 * _1MICRO;
    apTokens[0]->u.Number.dValue *= 60 * _1MICRO;
    return RINF_SUCCESS;
}

static int FnMinuteToMillisec(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= 60 * _1MILLI;
    apTokens[0]->u.Number.dValue *= 60 * _1MILLI;
    return RINF_SUCCESS;
}

static int FnMinuteToSecond(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue *= 60LL;
    apTokens[0]->u.Number.dValue *= 60LL;
    return RINF_SUCCESS;
}

static int FnMinuteToHour(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= 60LL;
    apTokens[0]->u.Number.dValue /= (long double)60LL;
    return RINF_SUCCESS;
}

static int FnMinuteToDay(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.dValue /= 1440LL;
    apTokens[0]->u.Number.dValue /= (long double)1440LL;
    return RINF_SUCCESS;
}

static int FnMinuteToWeek(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= 10080LL;
    apTokens[0]->u.Number.dValue /= (long double)10080LL;
    return RINF_SUCCESS;
}

static int FnMinuteToYear(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.uValue /= 525948.766;
    apTokens[0]->u.Number.dValue /= (long double)525948.766;
    return RINF_SUCCESS;
}

static int FnCelciusToFahrenheit(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.dValue *= (long double)(9 / 5.0);
    apTokens[0]->u.Number.dValue += 32;
    apTokens[0]->u.Number.uValue  = (uint64_t)apTokens[0]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int FnFahrenheitToCelcius(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    apTokens[0]->u.Number.dValue -= 32;
    apTokens[0]->u.Number.dValue /= (long double)(9 / 5.0);
    apTokens[0]->u.Number.uValue  = (uint64_t)apTokens[0]->u.Number.dValue;
    return RINF_SUCCESS;
}

static int FnSetBit32(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    /* We're fine shifting unsigned, well defined behaviour*/
    if (apTokens[0]->u.Number.uValue <= 31)
    {
        uint32_t const u32Val = 1;
        apTokens[0]->u.Number.uValue = (u32Val << apTokens[0]->u.Number.uValue);
        apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
        return RINF_SUCCESS;
    }
    else
        return RERR_INVALID_COMMAND_PARAMETER;
}

static int FnSetBit64(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    /* We're fine shifting unsigned, well defined behaviour*/
    if (apTokens[0]->u.Number.uValue <= 63)
    {
        uint64_t const u64Val = 1;
        apTokens[0]->u.Number.uValue = (u64Val << apTokens[0]->u.Number.uValue);
        apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
        return RINF_SUCCESS;
    }
    else
        return RERR_INVALID_COMMAND_PARAMETER;
}

static int FnSetBitRange32(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    if (   apTokens[0]->u.Number.uValue > 31
        || apTokens[1]->u.Number.uValue > 31)
        return RERR_INVALID_COMMAND_PARAMETER;

    uint8_t const uFirstBit = R_MIN(apTokens[0]->u.Number.uValue, apTokens[1]->u.Number.uValue);
    uint8_t const uLastBit  = R_MAX(apTokens[0]->u.Number.uValue, apTokens[1]->u.Number.uValue);

    uint32_t u32Val = 0;
    for (unsigned i = uFirstBit; i <= uLastBit; i++)
        u32Val |= ((uint64_t)1U << i);

    apTokens[0]->u.Number.uValue = u32Val;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnSetBitRange64(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    if (   apTokens[0]->u.Number.uValue > 63
        || apTokens[1]->u.Number.uValue > 63)
        return RERR_INVALID_COMMAND_PARAMETER;

    uint8_t const uFirstBit = R_MIN(apTokens[0]->u.Number.uValue, apTokens[1]->u.Number.uValue);
    uint8_t const uLastBit  = R_MAX(apTokens[0]->u.Number.uValue, apTokens[1]->u.Number.uValue);

    uint64_t u64Val = 0;
    for (unsigned i = uFirstBit; i <= uLastBit; i++)
        u64Val |= ((uint64_t)1U << i);

    apTokens[0]->u.Number.uValue = u64Val;
    apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

static int FnMax(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return RERR_NOT_IMPLEMENTED;
}

static int FnMin(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    return RERR_NOT_IMPLEMENTED;
}

static int FnAlign32(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    uint32_t const u32Val   = apTokens[0]->u.Number.uValue;
    uint32_t const u32Align = apTokens[1]->u.Number.uValue;
    if (u32Align % 2 == 0)
    {
        apTokens[0]->u.Number.uValue = ((u32Val + u32Align - 1) & ~(u32Align - 1));
        apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
        return RINF_SUCCESS;
    }
    return RERR_INVALID_COMMAND_PARAMETER;
}

static int FnAlign64(PEVALUATOR pEval, PTOKEN apTokens[], uint32_t cTokens)
{
    uint64_t const u64Val   = apTokens[0]->u.Number.uValue;
    uint64_t const u64Align = apTokens[1]->u.Number.uValue;
    if (u64Align % 2 == 0)
    {
        apTokens[0]->u.Number.uValue = ((u64Val + u64Align - 1) & ~(u64Align - 1));
        apTokens[0]->u.Number.dValue = apTokens[0]->u.Number.uValue;
        return RINF_SUCCESS;
    }
    return RERR_INVALID_COMMAND_PARAMETER;
}


/**
 * g_aFunctions: Table of Functions.
 */
FUNCTION g_aFunctions[] =
{
    /*  Name            Function               fUInt   cMin  cMax    Args    Desc   */
    { "help",           NULL,                  false,    1,  1,          "", "Like you don't know what this does." },
    { "vars",           NULL,                  false,    1,  1,          "", "Displays all defined variables." },
    { "quit",           NULL,                  false,    1,  1,          "", "Exit, stage left." },
    { "bye",            NULL,                  false,    1,  1,          "", "Exit, stage smiling." },

    { "sum",            FnSum,                 false,    1,  MAX_FUNCTION_PARAMETERS, "<num1> [,<num2>...<numN>]", "Sum of the numbers." },
    { "avg",            FnAverage,             false,    1,  MAX_FUNCTION_PARAMETERS, "<num1> [,<num2>...<numN>]", "Average (arithmetic mean) of the numbers." },
    { "if",             FnIf,                  false,    3,  3,          "<cond>,<expr-t>,<expr-f>", "If <cond> evaluates to true, returns <expr-t> otherwise <expr-f>." },
    { "fact",           FnFactorial,           true,     1,  1,          "<num1>", "Factorial." },
    { "gcd",            FnGCD,                 true,     2,  MAX_FUNCTION_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "GCD, Greatest Common Divisor." },
    { "hcf",            FnGCD,                 true,     2,  MAX_FUNCTION_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "HCF, Highest Common Factor." },
    { "lcm",            FnLCM,                 true,     2,  MAX_FUNCTION_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "LCM, Least Common Multiple." },
    { "max",            FnMax,                 true,     2,  MAX_FUNCTION_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "Maximum of the numbers." },
    { "min",            FnMin,                 true,     2,  MAX_FUNCTION_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "Minimum of the numbers." },

    { "pow",            FnPow,                 false,    2,  2, "<num1>, <num2>", "Returns <num1> raised to the power of <num2>." },
    { "root",           FnRoot,                false,    2,  2, "<num1>, <num2>", "Returns the <num2>th root of <num1>." },
    { "sqrt",           FnSqrt,                false,    1,  1, "<num1>", "Returns the square root of <num1>." },

    { "b2kb",           FnByteToKiloByte,      true,     1,  1, "<int1>", "Bytes to kilobytes." },
    { "b2mb",           FnByteToMegaByte,      true,     1,  1, "<int1>", "Bytes to megabytes." },
    { "b2gb",           FnByteToGigaByte,      true,     1,  1, "<int1>", "Bytes to gigabytes." },
    { "b2tb",           FnByteToTeraByte,      true,     1,  1, "<int1>", "Bytes to terabytes." },
    { "b2p",            FnByteToPage,          true,     1,  1, "<int1>", "Bytes to 4K size pages." },

    { "kb2b",           FnKiloByteToByte,      true,     1,  1, "<int1>", "Kilobytes to bytes." },
    { "kb2mb",          FnKiloByteToMegaByte,  true,     1,  1, "<int1>", "Kilobytes to megabytes." },
    { "kb2gb",          FnKiloByteToGigaByte,  true,     1,  1, "<int1>", "Kilobytes to gigabytes." },
    { "kb2tb",          FnKiloByteToTeraByte,  true,     1,  1, "<int1>", "Kilobytes to terabytes." },
    { "kb2p",           FnKiloByteToPage,      true,     1,  1, "<int1>", "Kilobytes to 4K size pages." },

    { "mb2b",           FnMegaByteToByte,      true,     1,  1, "<int1>", "Megabytes to bytes." },
    { "mb2kb",          FnMegaByteToKiloByte,  true,     1,  1, "<int1>", "Megabytes to kilobytes." },
    { "mb2gb",          FnMegaByteToGigaByte,  true,     1,  1, "<int1>", "Megabytes to gigabytes." },
    { "mb2tb",          FnMegaByteToTeraByte,  true,     1,  1, "<int1>", "Megabytes to terabytes." },
    { "mb2p",           FnMegaByteToPage,      true,     1,  1, "<int1>", "Megabytes to 4K size pages." },

    { "gb2b",           FnGigaByteToByte,      true,     1,  1, "<int1>", "Gigabytes to bytes." },
    { "gb2kb",          FnGigaByteToKiloByte,  true,     1,  1, "<int1>", "Gigabytes to kilobytes." },
    { "gb2mb",          FnGigaByteToMegaByte,  true,     1,  1, "<int1>", "Gigabytes to megabytes." },
    { "gb2tb",          FnGigaByteToTeraByte,  true,     1,  1, "<int1>", "Gigabytes to terabytes." },
    { "gb2p",           FnGigaByteToPage,      true,     1,  1, "<int1>", "Gigabytes to 4K size pages." },

    { "tb2b",           FnTeraByteToByte,      true,     1,  1, "<int1>", "Terabytes to bytes." },
    { "tb2kb",          FnTeraByteToKiloByte,  true,     1,  1, "<int1>", "Terabytes to kilobytes." },
    { "tb2mb",          FnTeraByteToMegaByte,  true,     1,  1, "<int1>", "Terabytes to megabytes." },
    { "tb2gb",          FnTeraByteToGigaByte,  true,     1,  1, "<int1>", "Terabytes to gigabytes." },
    { "tb2p",           FnTeraByteToPage,      true,     1,  1, "<int1>", "Terabytes to 4K size pages." },

    { "p2b",            FnPageToByte,          true,     1,  1, "<int1>", "Pages of size 4K to bytes." },
    { "p2kb",           FnPageToKiloByte,      true,     1,  1, "<int1>", "Pages of size 4K to kilobytes." },
    { "p2mb",           FnPageToMegaByte,      true,     1,  1, "<int1>", "Pages of size 4K to megabytes." },
    { "p2gb",           FnPageToGigaByte,      true,     1,  1, "<int1>", "Pages of size 4K to gigabytes." },
    { "p2tb",           FnPageToTeraByte,      true,     1,  1, "<int1>", "Pages of size 4K to terabytes." },

    { "nsec2usec",      FnNanosecToMicrosec,   true,     1,  1, "<num1>", "Nanoseconds to microseconds." },
    { "nsec2msec",      FnNanosecToMillisec,   true,     1,  1, "<num1>", "Nanoseconds to milliseconds." },
    { "nsec2sec",       FnNanosecToSecond,     true,     1,  1, "<num1>", "Nanoseconds to seconds." },
    { "nsec2min",       FnNanosecToMinute,     true,     1,  1, "<num1>", "Nanoseconds to minutes." },
    { "nsec2hr",        FnNanosecToHour,       true,     1,  1, "<num1>", "Nanoseconds to hours." },
    { "nsec2day",       FnNanosecToDay,        true,     1,  1, "<num1>", "Nanoseconds to days." },
    { "nsec2week",      FnNanosecToWeek,       true,     1,  1, "<num1>", "Nanoseconds to weeks." },
    { "nsec2year",      FnNanosecToYear,       true,     1,  1, "<num1>", "Nanoseconds to years." },

    { "usec2nsec",      FnMicrosecToNanosec,   true,     1,  1, "<num1>", "Microseconds to nanoseconds." },
    { "usec2msec",      FnMicrosecToMillisec,  true,     1,  1, "<num1>", "Microseconds to milliseconds." },
    { "usec2sec",       FnMicrosecToSecond,    true,     1,  1, "<num1>", "Microseconds to seconds." },
    { "usec2min",       FnMicrosecToMinute,    true,     1,  1, "<num1>", "Microseconds to minutes." },
    { "usec2hr",        FnMicrosecToHour,      true,     1,  1, "<num1>", "Microseconds to hours." },
    { "usec2day",       FnMicrosecToDay,       true,     1,  1, "<num1>", "Microseconds to days." },
    { "usec2week",      FnMicrosecToWeek,      true,     1,  1, "<num1>", "Microseconds to weeks." },
    { "usec2year",      FnMicrosecToYear,      true,     1,  1, "<num1>", "Microseconds to years." },

    { "msec2nsec",      FnMillisecToNanosec,   true,     1,  1, "<num1>", "Milliseconds to nanoseconds." },
    { "msec2usec",      FnMillisecToMicrosec,  true,     1,  1, "<num1>", "Milliseconds to milliseconds." },
    { "msec2sec",       FnMillisecToSecond,    true,     1,  1, "<num1>", "Milliseconds to seconds." },
    { "msec2min",       FnMillisecToMinute,    true,     1,  1, "<num1>", "Milliseconds to minutes." },
    { "msec2hr",        FnMillisecToHour,      true,     1,  1, "<num1>", "Milliseconds to hours." },
    { "msec2day",       FnMillisecToDay,       true,     1,  1, "<num1>", "Milliseconds to days." },
    { "msec2week",      FnMillisecToWeek,      true,     1,  1, "<num1>", "Milliseconds to weeks." },
    { "msec2year",      FnMillisecToYear,      true,     1,  1, "<num1>", "Milliseconds to years." },

    { "sec2nsec",       FnSecondToNanosec,     true,     1,  1, "<num1>", "Seconds to nanoseconds." },
    { "sec2usec",       FnSecondToMicrosec,    true,     1,  1, "<num1>", "Seconds to microseconds." },
    { "sec2msec",       FnSecondToMillisec,    true,     1,  1, "<num1>", "Seconds to milliseconds." },
    { "sec2min",        FnSecondToMinute,      true,     1,  1, "<num1>", "Seconds to minutes." },
    { "sec2hr",         FnSecondToHour,        true,     1,  1, "<num1>", "Seconds to hours." },
    { "sec2day",        FnSecondToDay,         true,     1,  1, "<num1>", "Seconds to days." },
    { "sec2week",       FnSecondToWeek,        true,     1,  1, "<num1>", "Seconds to weeks." },
    { "sec2year",       FnSecondToYear,        true,     1,  1, "<num1>", "Seconds to years." },

    { "min2nsec",       FnMinuteToNanosec,     true,     1,  1, "<num1>", "Minutes to nanoseconds." },
    { "min2usec",       FnMinuteToMicrosec,    true,     1,  1, "<num1>", "Minutes to microseconds." },
    { "min2msec",       FnMinuteToMillisec,    true,     1,  1, "<num1>", "Minutes to milliseconds." },
    { "min2sec",        FnMinuteToSecond,      true,     1,  1, "<num1>", "Minutes to seconds." },
    { "min2hr",         FnMinuteToHour,        true,     1,  1, "<num1>", "Minutes to hours." },
    { "min2day",        FnMinuteToDay,         true,     1,  1, "<num1>", "Minutes to days." },
    { "min2week",       FnMinuteToWeek,        true,     1,  1, "<num1>", "Minutes to weeks." },
    { "min2year",       FnMinuteToYear,        true,     1,  1, "<num1>", "Minutes to years." },

    { "cel2frh",        FnCelciusToFahrenheit, false,    1,  1, "<num1>", "Celcius to fahrenheit." },
    { "frh2cel",        FnFahrenheitToCelcius, false,    1,  1, "<num1>", "Fahrenheit to celcius." },

    /* VirtualBox style macros/functions. */
    { "RT_BIT",         FnSetBit32,            true,     1,  1, "<bit>", "Sets the specified bit (0-31)." },
    { "RT_BIT_32",      FnSetBit32,            true,     1,  1, "<bit>", "Sets specified bit (0-31). Same as RT_BIT." },
    { "RT_BIT_64",      FnSetBit64,            true,     1,  1, "<bit>", "Sets specified bit (0-63)." },
    { "RT_BITS",        FnSetBitRange32,       true,     1,  2, "<bit_f>,<bit_l>", "Sets the specified bits (0-31) from <bit_f> to <bit_l> both inclusive." },
    { "RT_BITS_32",     FnSetBitRange32,       true,     1,  2, "<bit_f>,<bit_l>", "Sets the specified bits (0-31) from <bit_f> to <bit_l> both inclusive." },
    { "RT_BITS_64",     FnSetBitRange64,       true,     1,  2, "<bit_f>,<bit_l>", "Sets the specified bits (0-63) from <bit_f> to <bit_l> both inclusive." },
    { "RT_ALIGN",       FnAlign32,             true,     2,  2, "<val>, <align>", "Aligns 32-bit <val> to <align>. <align> must be a power of 2." },
    { "RT_ALIGN_32",    FnAlign32,             true,     2,  2, "<val>, <align>", "Aligns 32-bit <val> to <align>. <align> must be a power of 2. Same as RT_ALIGN." },
    { "RT_ALIGN_64",    FnAlign64,             true,     2,  2, "<val>, <align>", "Aligns 64-bit <val> to <align>. <align> must be a power of 2." }
};

/** Total number of Functions in the table. */
const uint32_t g_cFunctions = R_ARRAY_ELEMENTS(g_aFunctions);

