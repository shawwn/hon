///////////////////////////////////////////////////////////////////////  
//  Branch Vertex Program (Static Lighting, GPU Wind)
//
//  (c) 2003 IDV, Inc.
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
//
//  - Wind matrices are stored in constants [0 - 15] (four matrices)
//  - Wind information is stored in the 3rd texture layer
//      - x = wind matrix index
//      - y = wind weight

const char szVP_Basic_StaticLighting_GpuWind[ ] = 
{
    "!!ARBvp1.0"
    "PARAM  mvp[4]      = { state.matrix.mvp };"
    "PARAM  env[]       = { program.env[0..32] };"

    // load index into address register (texcoord[2].x holds the matrix index)
    "ADDRESS aMatrixIndex;"
    "ARL aMatrixIndex.x, vertex.texcoord[2].x;"

    // multiply position by wind matrix
    "TEMP tFullWind;"
    "DP4 tFullWind.x, vertex.position, env[aMatrixIndex.x + 0];"        
    "DP4 tFullWind.y, vertex.position, env[aMatrixIndex.x + 1];"        
    "DP4 tFullWind.z, vertex.position, env[aMatrixIndex.x + 2];"        
    "DP4 tFullWind.w, vertex.position, env[aMatrixIndex.x + 3];"        

    // interpolate between no-wind and full-wind
    "TEMP tDiff;"
    "SUB tDiff, tFullWind, vertex.position;"

    // texcoord[2].y holds the wind weight in range [0,1]
    "TEMP tWeightedWind;"
    "MAD tWeightedWind, tDiff, vertex.texcoord[2].y, vertex.position;"

    // passthrough the color
    "MOV result.color, vertex.color;"

    // project to screen
    "DP4 result.position.x, mvp[0], tWeightedWind;"
    "DP4 result.position.y, mvp[1], tWeightedWind;"
    "DP4 result.position.z, mvp[2], tWeightedWind;"
    "DP4 result.position.w, mvp[3], tWeightedWind;"

    // texture coordinates are passed to output register
    "MOV result.texcoord.xy, vertex.texcoord;"
    "MOV result.texcoord[1], vertex.texcoord[1];"

    #ifdef WRAPPER_USE_FOG
        "DP4 result.fogcoord.x, mvp[2], vertex.position;"
    #endif

    "END"
};

