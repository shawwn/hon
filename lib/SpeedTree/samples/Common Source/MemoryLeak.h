///////////////////////////////////////////////////////////////////////  
//  CLeakTest Class
//
//  (c) 2003 IDV, Inc.
//
//  This class is used for testing memory leaks in debug mode.
//
//
//  *** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Interactive Data Visualization and may
//  not be copied or disclosed except in accordance with the terms of
//  that agreement.
//
//      Copyright (c) 2001-2003 IDV, Inc.
//      All Rights Reserved.
//
//      IDV, Inc.
//      1233 Washington St. Suite 610
//      Columbia, SC 29201
//      Voice: (803) 799-1699
//      Fax:   (803) 931-0320
//      Web:   http://www.idvinc.com

#pragma once

// memory leak tool
#include <crtdbg.h>

///////////////////////////////////////////////////////////////////////  
//  class CLeakTest declaration

class CLeakTest
{
public:
        void Start(void)
        {
#ifdef _DEBUG
            _CrtMemCheckpoint(&m_sMemStateStart);
#endif
        }

        void End(const char* pLabel)
        {
#ifdef _DEBUG
            _CrtMemState sMemStateDiff, sMemStateEnd;

            _CrtMemCheckpoint(&sMemStateEnd);
            if (_CrtMemDifference(&sMemStateDiff, &m_sMemStateStart, &sMemStateEnd))
            {
                _RPT1(_CRT_WARN, "\n%s BEGIN memory report\n", pLabel);
                _CrtMemDumpStatistics(&sMemStateDiff);
                _RPT1(_CRT_WARN, "%s END memory report\n\n", pLabel);
            }
            else
                _RPT1(_CRT_WARN, "\n%s - no memory leaks\n\n", pLabel);
#endif
        }

private:
        _CrtMemState        m_sMemStateStart;
};
