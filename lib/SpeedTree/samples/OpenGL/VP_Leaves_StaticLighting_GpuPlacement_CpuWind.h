///////////////////////////////////////////////////////////////////////  
//  Leaf Vertex Program (Static Lighting, GPU leaf placement, CPU Wind)
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
//      - z = leaf placement index (starts at 34)
//      - w = leaf scalar value

const char szVP_Leaves_StaticLighting_GpuPlacement_CpuWind[ ] = 
{
    "!!ARBvp1.0"
    "PARAM  mvp[4]      = { state.matrix.mvp };"
    "PARAM  env[]       = { program.env[0..95] };"

    // add leaf cluster table value to tWeightedWind
    "ADDRESS aAddr;"
    "ARL aAddr.x, vertex.texcoord[2].z;"
    "TEMP tScaledCorner;"
    "MUL tScaledCorner, env[aAddr.x], vertex.texcoord[2].w;"
    "TEMP tLeafCorner;"
    "ADD tLeafCorner, vertex.position, tScaledCorner;"

    // passthrough the color
    "MOV result.color, vertex.color;"

    // project to screen
    "DP4 result.position.x, mvp[0], tLeafCorner;"
    "DP4 result.position.y, mvp[1], tLeafCorner;"
    "DP4 result.position.z, mvp[2], tLeafCorner;"
    "DP4 result.position.w, mvp[3], tLeafCorner;"

    // texture coordinates are passed to output register
    "MOV result.texcoord.xy, vertex.texcoord;"

    #ifdef WRAPPER_USE_FOG
        "DP4 result.fogcoord.x, mvp[2], vertex.position;"
    #endif

    "END"
};

