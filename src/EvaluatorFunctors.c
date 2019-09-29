/** @file
 * Evaluator Functors.
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
#include "EvaluatorFunctors.h"
#include "Errors.h"
#include "GenericDefs.h"
#include "StringOps.h"
#include "InputOutput.h"


/*******************************************************************************
 *   Functor Functors!                                                         *
 *******************************************************************************/
int FnSum(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    for (uint32_t i = 1; i < cAtoms; i++)
    {
        apAtoms[0]->u.Number.uValue += apAtoms[i]->u.Number.uValue;
        apAtoms[0]->u.Number.dValue += apAtoms[i]->u.Number.dValue;
    }
    return RINF_SUCCESS;
}

int FnAverage(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    int rc = FnSum(pEval, apAtoms, cAtoms);
    if (RC_SUCCESS(rc))
    {
        apAtoms[0]->u.Number.uValue /= cAtoms;
        apAtoms[0]->u.Number.dValue /= cAtoms;
    }
    return RINF_SUCCESS;
}

int FnFactorial(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    UINTEGER uValue = apAtoms[0]->u.Number.uValue;
    UINTEGER uFact  = 1;
    while (uValue > 1)
    {
        uFact *= uValue;
        --uValue;
    }
    apAtoms[0]->u.Number.uValue = uFact;
    apAtoms[0]->u.Number.dValue = uFact;
    return RINF_SUCCESS;
}

int FnGCD(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    /*
     * Bleh, just write something quickly now. I'll revisit this to make it
     * more efficient later.
     */
    UINTEGER uMin = apAtoms[0]->u.Number.uValue;
    for (uint32_t i = 1; i < cAtoms; i++)
    {
        UINTEGER uCur = apAtoms[i]->u.Number.uValue;
        if (uMin < uCur)
            uMin = uCur;
    }

    bool fSet = false;
    for (UINTEGER u = uMin; u > 0; u--)
    {
        if (uMin % u == 0)
        {
            uint32_t cFactors = 0;
            for (UINTEGER k = 0; k < cAtoms; k++)
            {
                if (apAtoms[k]->u.Number.uValue % u == 0)
                    cFactors ++;
            }
            if (cFactors == cAtoms)
            {
                apAtoms[0]->u.Number.uValue = u;
                apAtoms[0]->u.Number.dValue = u;
                break;
            }
        }
    }
    if (!fSet)
    {
        apAtoms[0]->u.Number.uValue = 1;
        apAtoms[0]->u.Number.dValue = 1;
    }
    return RINF_SUCCESS;
}

int FnLCM(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    /*
     * Bleh, just write something quickly now. I'll revisit this to make it
     * more efficient later.
     */
    UINTEGER uMax = apAtoms[0]->u.Number.uValue;
    UINTEGER uProduct = 1;
    for (uint32_t i = 0; i < cAtoms; i++)
    {
        UINTEGER uCur = apAtoms[i]->u.Number.uValue;
        uProduct *= uCur;
        if (uMax > uCur)
            uMax = uCur;
    }

    bool fSet = false;
    for (UINTEGER u = uMax; u <= uProduct; u += uMax)
    {
        uint32_t cFactors = 0;
        for (UINTEGER k = 0; k < cAtoms; k++)
        {
            if (u % apAtoms[k]->u.Number.uValue == 0)
                cFactors++;
        }
        if (cFactors == cAtoms)
        {
            apAtoms[0]->u.Number.uValue = u;
            apAtoms[0]->u.Number.dValue = u;
            fSet = true;
            break;
        }
    }
    if (!fSet)
    {
        apAtoms[0]->u.Number.uValue = 1;
        apAtoms[0]->u.Number.dValue = 1;
    }
    return RINF_SUCCESS;
}


int FnPow(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.dValue = FPOWFLOAT(apAtoms[0]->u.Number.dValue, apAtoms[1]->u.Number.dValue);
    apAtoms[0]->u.Number.uValue = (UINTEGER)(apAtoms[0]->u.Number.dValue + 0.50);
    return RINF_SUCCESS;
}

int FnSqrt(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.dValue = FPOWFLOAT(apAtoms[0]->u.Number.dValue, (FLOAT)0.50);
    apAtoms[0]->u.Number.uValue = (UINTEGER)(apAtoms[0]->u.Number.dValue + 0.50);
    return RINF_SUCCESS;
}

int FnRoot(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.dValue = FPOWFLOAT(apAtoms[0]->u.Number.dValue, 1 / (FLOAT)apAtoms[1]->u.Number.dValue);
    apAtoms[0]->u.Number.uValue = (UINTEGER)(apAtoms[0]->u.Number.dValue + 0.50);
    return RINF_SUCCESS;
}

int FnIf(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    uint8_t const idxResultAtom = apAtoms[0]->u.Number.uValue ? 1 : 2;
    apAtoms[0]->u.Number.uValue = apAtoms[idxResultAtom]->u.Number.uValue;
    apAtoms[0]->u.Number.dValue = apAtoms[idxResultAtom]->u.Number.dValue;
    return RINF_SUCCESS;
}


int FnByteToPage(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    /* -XXX- @todo make functors specify minimum parameter widths like operators */
    apAtoms[0]->u.Number.uValue = (apAtoms[0]->u.Number.uValue + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apAtoms[0]->u.Number.dValue = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnByteToKiloByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1K;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnByteToMegaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1M;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnByteToGigaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1G;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnByteToTeraByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1T;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}


int FnKiloByteToByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1K;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnKiloByteToMegaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnByteToKiloByte(pEval, apAtoms, cAtoms);
}

int FnKiloByteToGigaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnByteToMegaByte(pEval, apAtoms, cAtoms);
}

int FnKiloByteToTeraByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnByteToGigaByte(pEval, apAtoms, cAtoms);
}

int FnKiloByteToPage(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue = (apAtoms[0]->u.Number.uValue * _1K + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apAtoms[0]->u.Number.dValue = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}


int FnMegaByteToByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1M;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnMegaByteToKiloByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1K;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnMegaByteToGigaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnByteToKiloByte(pEval, apAtoms, cAtoms);
}

int FnMegaByteToTeraByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnByteToMegaByte(pEval, apAtoms, cAtoms);
}

int FnMegaByteToPage(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue = (apAtoms[0]->u.Number.uValue * _1M + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apAtoms[0]->u.Number.dValue = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}


int FnGigaByteToByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1G;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnGigaByteToKiloByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnMegaByteToByte(pEval, apAtoms, cAtoms);
}

int FnGigaByteToMegaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnMegaByteToKiloByte(pEval, apAtoms, cAtoms);
}

int FnGigaByteToTeraByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnByteToKiloByte(pEval, apAtoms, cAtoms);
}

int FnGigaByteToPage(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue = (apAtoms[0]->u.Number.uValue * _1G + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apAtoms[0]->u.Number.dValue = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnTeraByteToByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1T;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnTeraByteToKiloByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnGigaByteToByte(pEval, apAtoms, cAtoms);
}

int FnTeraByteToMegaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnMegaByteToByte(pEval, apAtoms, cAtoms);
}

int FnTeraByteToGigaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return FnKiloByteToByte(pEval, apAtoms, cAtoms);
}

int FnTeraByteToPage(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue = (apAtoms[0]->u.Number.uValue * _1T + _MEM_PAGEOFFSET) >> _MEM_PAGESHIFT;
    apAtoms[0]->u.Number.dValue = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnPageToByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _MEM_PAGESIZE;
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnPageToKiloByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= (_MEM_PAGESIZE / _1K);
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnPageToMegaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= (_MEM_PAGESIZE / _1M);
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnPageToGigaByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= (_MEM_PAGESIZE / _1G);
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnPageToTeraByte(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= (_MEM_PAGESIZE / _1T);
    apAtoms[0]->u.Number.dValue  = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}



int FnNanosecToMicrosec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1MILLI;
    apAtoms[0]->u.Number.dValue /= _1MILLI;
    return RINF_SUCCESS;
}

int FnNanosecToMillisec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1MICRO;
    apAtoms[0]->u.Number.dValue /= _1MICRO;
    return RINF_SUCCESS;
}

int FnNanosecToSecond(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1NANO;
    apAtoms[0]->u.Number.dValue /= _1NANO;
    return RINF_SUCCESS;
}

int FnNanosecToMinute(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (60 * _1NANO);
    apAtoms[0]->u.Number.dValue /= (60 * _1NANO);
    return RINF_SUCCESS;
}

int FnNanosecToHour(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (3600LL * _1NANO);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)3600LL * _1NANO);
    return RINF_SUCCESS;
}

int FnNanosecToDay(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (86400LL * _1NANO);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)86400LL * _1NANO);
    return RINF_SUCCESS;
}

int FnNanosecToWeek(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (604800LL * _1NANO);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)604800LL * _1NANO);
    return RINF_SUCCESS;
}

int FnNanosecToYear(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (31556926LL * _1NANO);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)31556926LL * _1NANO);
    return RINF_SUCCESS;
}

int FnMicrosecToNanosec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1MILLI;
    apAtoms[0]->u.Number.dValue *= _1MILLI;
    return RINF_SUCCESS;
}

int FnMicrosecToMillisec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1MILLI;
    apAtoms[0]->u.Number.dValue /= _1MILLI;
    return RINF_SUCCESS;
}


int FnMicrosecToSecond(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1MICRO;
    apAtoms[0]->u.Number.dValue /= _1MICRO;
    return RINF_SUCCESS;
}


int FnMicrosecToMinute(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (60 * _1MICRO);
    apAtoms[0]->u.Number.dValue /= (60 * _1MICRO);
    return RINF_SUCCESS;
}

int FnMicrosecToHour(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (3600LL * _1MICRO);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)3600LL * _1MICRO);
    return RINF_SUCCESS;
}

int FnMicrosecToDay(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (86400LL * _1MICRO);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)86400LL * _1MICRO);
    return RINF_SUCCESS;
}

int FnMicrosecToWeek(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (604800LL * _1MICRO);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)604800LL * _1MICRO);
    return RINF_SUCCESS;
}

int FnMicrosecToYear(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (31556926LL * _1MICRO);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)31556926LL * _1MICRO);
    return RINF_SUCCESS;
}


int FnMillisecToNanosec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1MICRO;
    apAtoms[0]->u.Number.dValue *= _1MICRO;
    return RINF_SUCCESS;
}

int FnMillisecToMicrosec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1MILLI;
    apAtoms[0]->u.Number.dValue *= _1MILLI;
    return RINF_SUCCESS;
}


int FnMillisecToSecond(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= _1MILLI;
    apAtoms[0]->u.Number.dValue /= _1MILLI;
    return RINF_SUCCESS;
}


int FnMillisecToMinute(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (60 * _1MILLI);
    apAtoms[0]->u.Number.dValue /= (60 * _1MILLI);
    return RINF_SUCCESS;
}

int FnMillisecToHour(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (3600LL * _1MILLI);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)3600LL * _1MILLI);
    return RINF_SUCCESS;
}

int FnMillisecToDay(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (86400LL * _1MILLI);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)86400LL * _1MILLI);
    return RINF_SUCCESS;
}

int FnMillisecToWeek(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (604800LL * _1MILLI);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)604800LL * _1MILLI);
    return RINF_SUCCESS;
}

int FnMillisecToYear(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= (31556926LL * _1MILLI);
    apAtoms[0]->u.Number.dValue /= ((FLOAT)31556926LL * _1MILLI);
    return RINF_SUCCESS;
}


int FnSecondToNanosec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1NANO;
    apAtoms[0]->u.Number.dValue *= _1NANO;
    return RINF_SUCCESS;
}

int FnSecondToMicrosec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1MICRO;
    apAtoms[0]->u.Number.dValue *= _1MICRO;
    return RINF_SUCCESS;
}

int FnSecondToMillisec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= _1MILLI;
    apAtoms[0]->u.Number.dValue *= _1MILLI;
    return RINF_SUCCESS;
}

int FnSecondToMinute(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= 60LL;
    apAtoms[0]->u.Number.dValue /= 60LL;
    return RINF_SUCCESS;
}

int FnSecondToHour(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= 3600LL;
    apAtoms[0]->u.Number.dValue /= (FLOAT)3600LL;
    return RINF_SUCCESS;
}

int FnSecondToDay(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= 86400LL;
    apAtoms[0]->u.Number.dValue /= (FLOAT)86400LL;
    return RINF_SUCCESS;
}

int FnSecondToWeek(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= 604800LL;
    apAtoms[0]->u.Number.dValue /= (FLOAT)604800LL;
    return RINF_SUCCESS;
}

int FnSecondToYear(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= 31556926LL;
    apAtoms[0]->u.Number.dValue /= (FLOAT)31556926LL;
    return RINF_SUCCESS;
}

int FnMinuteToNanosec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.dValue *= 60 * _1NANO;
    apAtoms[0]->u.Number.dValue *= 60 * _1NANO;
    return RINF_SUCCESS;
}

int FnMinuteToMicrosec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= 60 * _1MICRO;
    apAtoms[0]->u.Number.dValue *= 60 * _1MICRO;
    return RINF_SUCCESS;
}

int FnMinuteToMillisec(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= 60 * _1MILLI;
    apAtoms[0]->u.Number.dValue *= 60 * _1MILLI;
    return RINF_SUCCESS;
}

int FnMinuteToSecond(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue *= 60LL;
    apAtoms[0]->u.Number.dValue *= 60LL;
    return RINF_SUCCESS;
}

int FnMinuteToHour(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= 60LL;
    apAtoms[0]->u.Number.dValue /= (FLOAT)60LL;
    return RINF_SUCCESS;
}

int FnMinuteToDay(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.dValue /= 1440LL;
    apAtoms[0]->u.Number.dValue /= (FLOAT)1440LL;
    return RINF_SUCCESS;
}

int FnMinuteToWeek(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= 10080LL;
    apAtoms[0]->u.Number.dValue /= (FLOAT)10080LL;
    return RINF_SUCCESS;
}

int FnMinuteToYear(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.uValue /= 525948.766;
    apAtoms[0]->u.Number.dValue /= (FLOAT)525948.766;
    return RINF_SUCCESS;
}

int FnCelciusToFahrenheit(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.dValue *= (FLOAT)(9 / 5.0);
    apAtoms[0]->u.Number.dValue += 32;
    apAtoms[0]->u.Number.uValue  = (UINTEGER)apAtoms[0]->u.Number.dValue;
    return RINF_SUCCESS;
}

int FnFahrenheitToCelcius(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    apAtoms[0]->u.Number.dValue -= 32;
    apAtoms[0]->u.Number.dValue /= (FLOAT)(9 / 5.0);
    apAtoms[0]->u.Number.uValue  = (UINTEGER)apAtoms[0]->u.Number.dValue;
    return RINF_SUCCESS;
}

int FnSetBit(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    /* We're fine shifting unsigned, well defined behaviour*/
    apAtoms[0]->u.Number.uValue = (1U << apAtoms[0]->u.Number.uValue);
    apAtoms[0]->u.Number.dValue = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}

int FnMax(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return RERR_NOT_IMPLEMENTED;
}

int FnMin(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    return RERR_NOT_IMPLEMENTED;
}

int FnAlign(PEVALUATOR pEval, PATOM apAtoms[], uint32_t cAtoms)
{
    U64INTEGER const uVal   = apAtoms[0]->u.Number.uValue;
    U64INTEGER const uAlign = apAtoms[1]->u.Number.uValue;
    apAtoms[0]->u.Number.uValue = ((uVal + uAlign - 1) & ~(uAlign - 1));
    apAtoms[0]->u.Number.dValue = apAtoms[0]->u.Number.uValue;
    return RINF_SUCCESS;
}


/**
 * g_aFunctors: Table of functors.
 */
FUNCTOR g_aFunctors[] =
{
    /*  Name            Function               fUInt   cMin  cMax    Args    Desc   */
    { "help",           NULL,                  false,    1,  1,          "", "Like you don't know what this does." },
    { "vars",           NULL,                  false,    1,  1,          "", "Displays all defined variables." },
    { "quit",           NULL,                  false,    1,  1,          "", "Exit, stage left." },
    { "bye",            NULL,                  false,    1,  1,          "", "Exit, stage smiling." },

    { "sum",            FnSum,                 false,    1,  MAX_FUNCTOR_PARAMETERS, "<num1> [,<num2>...<numN>]", "Sum of the numbers." },
    { "avg",            FnAverage,             false,    1,  MAX_FUNCTOR_PARAMETERS, "<num1> [,<num2>...<numN>]", "Average (arithmetic mean) of the numbers." },
    { "if",             FnIf,                  false,    3,  3,          "<cond>,<expr-t>,<expr-f>", "If <cond> evaluates to true, returns <expr-t> otherwise <expr-f>." },
    { "fact",           FnFactorial,           true,     1,  1,          "<num1>", "Factorial." },
    { "gcd",            FnGCD,                 true,     2,  MAX_FUNCTOR_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "GCD, Greatest Common Divisor." },
    { "hcf",            FnGCD,                 true,     2,  MAX_FUNCTOR_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "HCF, Highest Common Factor." },
    { "lcm",            FnLCM,                 true,     2,  MAX_FUNCTOR_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "LCM, Least Common Multiple." },
    { "max",            FnMax,                 true,     2,  MAX_FUNCTOR_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "Maximum of the numbers." },
    { "min",            FnMin,                 true,     2,  MAX_FUNCTOR_PARAMETERS, "<num1>, <num2> [,<num3>...<numN>]", "Minimum of the numbers." },

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
    { "RT_BIT",         FnSetBit,              true,     1,  1, "<bit>", "Sets specified bit number for integer 0." },
    { "RT_BIT_32",      FnSetBit,              true,     1,  1, "<bit>", "Sets specified bit number for integer 0. Same as RT_BIT." },
    { "RT_BIT_64",      FnSetBit,              true,     1,  1, "<bit>", "Sets specified bit number for integer 0. Same as RT_BIT." },
    { "RT_ALIGN",       FnAlign,               true,     2,  2, "<val>, <align>", "Aligns <val> to boundary of <align>. <align> must be a power of 2." },
    { "RT_ALIGN_32",    FnAlign,               true,     2,  2, "<val>, <align>", "Same as RT_ALIGN." },
    { "RT_ALIGN_64",    FnAlign,               true,     2,  2, "<val>, <align>", "Same as RT_ALIGN." }
};

/** Total number of functors in the table. */
const uint32_t g_cFunctors = R_ARRAY_ELEMENTS(g_aFunctors);

