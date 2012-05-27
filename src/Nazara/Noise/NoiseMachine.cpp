// Copyright (C) 2012 R�mi B�ges
// This file is part of the "Nazara Engine".
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Noise/NoiseMachine.hpp>
#include <cmath>

#include <Nazara/Core/Error.hpp>
#include <Nazara/Noise/Config.hpp>
#include <Nazara/Noise/Debug.hpp>

NzNoiseMachine::NzNoiseMachine(int seed)
{
    pi = 3.14159265;

    SkewCoeff2D = 0.5*(sqrt(3.0) - 1.0);
    UnskewCoeff2D  = (3.0-sqrt(3.0))/6;

    SkewCoeff3D = 1/3;
    UnskewCoeff3D = 1/6;

    SkewCoeff4D = (sqrt(5) - 1)/4;
    UnskewCoeff4D = (5 - sqrt(5))/20;

    Ua = 16807;
    Uc = 0;
    Um = 2147483647;
    UcurrentSeed = 0;
    Uprevious = 0;

    SetNewSeed(seed);

    for(int i(0) ; i < 256 ; i++)
    PermutationTemp[i] = i;

    int lookupTemp4D[][4] =
    {
        {0,1,2,3},{0,1,3,2},{0,0,0,0},{0,2,3,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,2,3,0},
        {0,2,1,3},{0,0,0,0},{0,3,1,2},{0,3,2,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,3,2,0},
        {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
        {1,2,0,3},{0,0,0,0},{1,3,0,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,3,0,1},{2,3,1,0},
        {1,0,2,3},{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,0,3,1},{0,0,0,0},{2,1,3,0},
        {0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
        {2,0,1,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,0,1,2},{3,0,2,1},{0,0,0,0},{3,1,2,0},
        {2,1,0,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,1,0,2},{0,0,0,0},{3,2,0,1},{3,2,1,0}
    };

    for(int i(0) ; i < 64 ; ++i)
        for(int j(0) ; j < 4 ; ++j)
            lookupTable4D[i][j] = lookupTemp4D[i][j];

    float unit = 1.0/sqrt(2);
    float grad2Temp[][2] = {{unit,unit},{-unit,unit},{unit,-unit},{-unit,-unit},
                            {1,0},{-1,0},{0,1},{0,-1}};

    for(int i(0) ; i < 8 ; ++i)
        for(int j(0) ; j < 2 ; ++j)
            gradient2[i][j] = grad2Temp[i][j];

    int grad3Temp[][3] = {
        {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
        {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
        {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1},
        {1,1,0},{-1,1,0},{0,-1,1},{0,-1,-1}
    };

    for(int i(0) ; i < 16 ; ++i)
        for(int j(0) ; j < 3 ; ++j)
            gradient3[i][j] = grad3Temp[i][j];

    int grad4Temp[][4] =
    {
        {0,1,1,1}, {0,1,1,-1}, {0,1,-1,1}, {0,1,-1,-1},
        {0,-1,1,1},{0,-1,1,-1},{0,-1,-1,1},{0,-1,-1,-1},
        {1,0,1,1}, {1,0,1,-1}, {1,0,-1,1}, {1,0,-1,-1},
        {-1,0,1,1},{-1,0,1,-1},{-1,0,-1,1},{-1,0,-1,-1},
        {1,1,0,1}, {1,1,0,-1}, {1,-1,0,1}, {1,-1,0,-1},
        {-1,1,0,1},{-1,1,0,-1},{-1,-1,0,1},{-1,-1,0,-1},
        {1,1,1,0}, {1,1,-1,0}, {1,-1,1,0}, {1,-1,-1,0},
        {-1,1,1,0},{-1,1,-1,0},{-1,-1,1,0},{-1,-1,-1,0}
    };

    for(int i(0) ; i < 32 ; ++i)
        for(int j(0) ; j < 4 ; ++j)
            gradient4[i][j] = grad4Temp[i][j];

    m_lacunarity = 2.5f;
    m_octaves = 3.0f;
    m_hurst = 1.1f;
    m_parametersModified = true;
}

NzNoiseMachine::~NzNoiseMachine()
{
}

void NzNoiseMachine::SetNewSeed(int seed)
{
    Uprevious = seed;
    UcurrentSeed = seed;
}

int NzNoiseMachine::GetUniformRandomValue()
{
    Ulast = Ua*Uprevious + Uc%Um;
    Uprevious = Ulast;
    return Ulast;
}

void NzNoiseMachine::ShufflePermutationTable()
{
    int xchanger;
    unsigned int ncase;

    for(int j(0) ; j < 10 ; ++j)
        for (int i(0); i < 256 ; ++i)
        {
            ncase = this->GetUniformRandomValue() & 255;
            xchanger = PermutationTemp[i];
            PermutationTemp[i] = PermutationTemp[ncase];
            PermutationTemp[ncase] = xchanger;
        }

    for(int i(0) ; i < 512; ++i)
        perm[i]=PermutationTemp[i & 255];
}

//------------------------------ PERLIN ------------------------------

float NzNoiseMachine::Get1DPerlinNoiseValue(float x, float res)
{
    nx = x/res;
    x0 = (int)(nx);
    ii = x0 & 255;

    gi0 = perm[ii] % 16;
    gi1 = perm[ii + 1] % 16;

    temp.x = nx-x0;
    temp.y = ny-y0;

    s[0] = gradient3[gi0][0]*temp.x;

    temp.x = nx-(x0+1);

    t[0] = gradient3[gi1][0]*temp.x;

    tmp = nx-x0;
    Cx = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    return s[0] + Cx*(t[0]-s[0]);
}

float NzNoiseMachine::Get2DPerlinNoiseValue(float x, float y, float res)
{
    nx = x/res;
    ny = y/res;

    x0 = (int)(nx);
    y0 = (int)(ny);

    ii = x0 & 255;
    jj = y0 & 255;

    gi0 = perm[ii + perm[jj]] & 7;
    gi1 = perm[ii + 1 + perm[jj]] & 7;
    gi2 = perm[ii + perm[jj + 1]] & 7;
    gi3 = perm[ii + 1 + perm[jj + 1]] & 7;

    temp.x = nx-x0;
    temp.y = ny-y0;
    s[0] = gradient2[gi0][0]*temp.x + gradient2[gi0][1]*temp.y;

    temp.x = nx-(x0+1);
    temp.y = ny-y0;
    t[0] = gradient2[gi1][0]*temp.x + gradient2[gi1][1]*temp.y;

    temp.x = nx-x0;
    temp.y = ny-(y0+1);
    u[0] = gradient2[gi2][0]*temp.x + gradient2[gi2][1]*temp.y;

    temp.x = nx-(x0+1);
    temp.y = ny-(y0+1);
    v[0] = gradient2[gi3][0]*temp.x + gradient2[gi3][1]*temp.y;

    tmp = nx-x0;
    Cx = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    Li1 = s[0] + Cx*(t[0]-s[0]);
    Li2 = u[0] + Cx*(v[0]-u[0]);

    tmp = ny - y0;
    Cy = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    return Li1 + Cy*(Li2-Li1);
}
float NzNoiseMachine::Get3DPerlinNoiseValue(float x, float y, float z, float res)
{
    nx = x/res;
    ny = y/res;
    nz = z/res;

    x0 = (int)(nx);
    y0 = (int)(ny);
    z0 = (int)(nz);

    ii = x0 & 255;
    jj = y0 & 255;
    kk = z0 & 255;

    gi0 = perm[ii + perm[jj + perm[kk]]] % 16;
    gi1 = perm[ii + 1 + perm[jj + perm[kk]]] % 16;
    gi2 = perm[ii + perm[jj + 1 + perm[kk]]] % 16;
    gi3 = perm[ii + 1 + perm[jj + 1 + perm[kk]]] % 16;

    gi4 = perm[ii + perm[jj + perm[kk + 1]]] % 16;
    gi5 = perm[ii + 1 + perm[jj + perm[kk + 1]]] % 16;
    gi6 = perm[ii + perm[jj + 1 + perm[kk + 1]]] % 16;
    gi7 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]] % 16;

    temp.x = nx-x0;
    temp.y = ny-y0;
    temp.z = nz-z0;
    s[0] = gradient3[gi0][0]*temp.x + gradient3[gi0][1]*temp.y + gradient3[gi0][2]*temp.z;

    temp.x = nx-(x0+1);
    temp.y = ny-y0;
    t[0] = gradient3[gi1][0]*temp.x + gradient3[gi1][1]*temp.y + gradient3[gi1][2]*temp.z;

    temp.x = nx-x0;
    temp.y = ny-(y0+1);
    u[0] = gradient3[gi2][0]*temp.x + gradient3[gi2][1]*temp.y + gradient3[gi2][2]*temp.z;

    temp.x = nx-(x0+1);
    temp.y = ny-(y0+1);
    v[0] = gradient3[gi3][0]*temp.x + gradient3[gi3][1]*temp.y + gradient3[gi3][2]*temp.z;

    temp.x = nx-x0;
    temp.y = ny-y0;
    temp.z = nz-(z0+1);
    s[1] = gradient3[gi4][0]*temp.x + gradient3[gi4][1]*temp.y + gradient3[gi4][2]*temp.z;

    temp.x = nx-(x0+1);
    temp.y = ny-y0;
    t[1] = gradient3[gi5][0]*temp.x + gradient3[gi5][1]*temp.y + gradient3[gi5][2]*temp.z;

    temp.x = nx-x0;
    temp.y = ny-(y0+1);
    u[1] = gradient3[gi6][0]*temp.x + gradient3[gi6][1]*temp.y + gradient3[gi6][2]*temp.z;

    temp.x = nx-(x0+1);
    temp.y = ny-(y0+1);
    v[1] = gradient3[gi7][0]*temp.x + gradient3[gi7][1]*temp.y + gradient3[gi7][2]*temp.z;

    tmp = nx-x0;
    Cx = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    Li1 = s[0] + Cx*(t[0]-s[0]);
    Li2 = u[0] + Cx*(v[0]-u[0]);
    Li3 = s[1] + Cx*(t[1]-s[1]);
    Li4 = u[1] + Cx*(v[1]-u[1]);

    tmp = ny-y0;
    Cy = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    Li5 = Li1 + Cy*(Li2-Li1);
    Li6 = Li3 + Cy*(Li4-Li3);

    tmp = nz-z0;
    Cz = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    return Li5 + Cz*(Li6-Li5);
}

float NzNoiseMachine::Get4DPerlinNoiseValue(float x, float y, float z, float w, float res)
{
    nx = x/res;
    ny = y/res;
    nz = z/res;
    nw = w/res;

    x0 = (int)(nx);
    y0 = (int)(ny);
    z0 = (int)(nz);
    w0 = (int)(nw);

    ii = x0 & 255;
    jj = y0 & 255;
    kk = z0 & 255;
    ll = w0 & 255;

    gi0 =  perm[ii     + perm[jj     + perm[kk     + perm[ll    ]]]] % 32;
    gi1 =  perm[ii + 1 + perm[jj     + perm[kk     + perm[ll    ]]]] % 32;
    gi2 =  perm[ii     + perm[jj + 1 + perm[kk     + perm[ll    ]]]] % 32;
    gi3 =  perm[ii + 1 + perm[jj + 1 + perm[kk     + perm[ll    ]]]] % 32;

    gi4 =  perm[ii     + perm[jj +   + perm[kk + 1 + perm[ll    ]]]] % 32;
    gi5 =  perm[ii + 1 + perm[jj +   + perm[kk + 1 + perm[ll    ]]]] % 32;
    gi6 =  perm[ii     + perm[jj + 1 + perm[kk + 1 + perm[ll    ]]]] % 32;
    gi7 =  perm[ii + 1 + perm[jj + 1 + perm[kk + 1 + perm[ll    ]]]] % 32;

    gi8 =  perm[ii     + perm[jj     + perm[kk     + perm[ll + 1]]]] % 32;
    gi9 =  perm[ii + 1 + perm[jj     + perm[kk     + perm[ll + 1]]]] % 32;
    gi10 = perm[ii     + perm[jj + 1 + perm[kk     + perm[ll + 1]]]] % 32;
    gi11 = perm[ii + 1 + perm[jj + 1 + perm[kk     + perm[ll + 1]]]] % 32;

    gi12 = perm[ii     + perm[jj     + perm[kk + 1 + perm[ll + 1]]]] % 32;
    gi13 = perm[ii + 1 + perm[jj     + perm[kk + 1 + perm[ll + 1]]]] % 32;
    gi14 = perm[ii     + perm[jj + 1 + perm[kk + 1 + perm[ll + 1]]]] % 32;
    gi15 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1 + perm[ll + 1]]]] % 32;

    temp.x = nx-x0;
    temp.y = ny-y0;
    temp.z = nz-z0;
    temp.w = nw-w0;
    s[0] = gradient4[gi0][0]*temp.x + gradient4[gi0][1]*temp.y + gradient4[gi0][2]*temp.z + gradient4[gi0][3]*temp.w;

    temp.x = nx-(x0+1);
    temp.y = ny-y0;
    t[0] = gradient4[gi1][0]*temp.x + gradient4[gi1][1]*temp.y + gradient4[gi1][2]*temp.z + gradient4[gi1][3]*temp.w;

    temp.x = nx-x0;
    temp.y = ny-(y0+1);
    u[0] = gradient4[gi2][0]*temp.x + gradient4[gi2][1]*temp.y + gradient4[gi2][2]*temp.z + gradient4[gi2][3]*temp.w;

    temp.x = nx-(x0+1);
    temp.y = ny-(y0+1);
    v[0] = gradient4[gi3][0]*temp.x + gradient4[gi3][1]*temp.y + gradient4[gi3][2]*temp.z + gradient4[gi3][3]*temp.w;

    temp.x = nx-x0;
    temp.y = ny-y0;
    temp.z = nz-(z0+1);
    s[1] = gradient4[gi4][0]*temp.x + gradient4[gi4][1]*temp.y + gradient4[gi4][2]*temp.z + gradient4[gi4][3]*temp.w;

    temp.x = nx-(x0+1);
    temp.y = ny-y0;
    t[1] = gradient4[gi5][0]*temp.x + gradient4[gi5][1]*temp.y + gradient4[gi5][2]*temp.z + gradient4[gi5][3]*temp.w;

    temp.x = nx-x0;
    temp.y = ny-(y0+1);
    u[1] = gradient4[gi6][0]*temp.x + gradient4[gi6][1]*temp.y + gradient4[gi6][2]*temp.z + gradient4[gi6][3]*temp.w;

    temp.x = nx-(x0+1);
    temp.y = ny-(y0+1);
    v[1] = gradient4[gi7][0]*temp.x + gradient4[gi7][1]*temp.y + gradient4[gi7][2]*temp.z + gradient4[gi7][3]*temp.w;

    temp.x = nx-x0;
    temp.y = ny-y0;
    temp.z = nz-z0;
    temp.w = nw-(w0+1);
    s[2] = gradient4[gi8][0]*temp.x + gradient4[gi8][1]*temp.y + gradient4[gi8][2]*temp.z + gradient4[gi8][3]*temp.w;

    temp.x = nx-(x0+1);
    temp.y = ny-y0;
    t[2] = gradient4[gi9][0]*temp.x + gradient4[gi9][1]*temp.y + gradient4[gi9][2]*temp.z + gradient4[gi9][3]*temp.w;

    temp.x = nx-x0;
    temp.y = ny-(y0+1);
    u[2] = gradient4[gi10][0]*temp.x + gradient4[gi10][1]*temp.y + gradient4[gi10][2]*temp.z + gradient4[gi10][3]*temp.w;

    temp.x = nx-(x0+1);
    temp.y = ny-(y0+1);
    v[2] = gradient4[gi11][0]*temp.x + gradient4[gi11][1]*temp.y + gradient4[gi11][2]*temp.z + gradient4[gi11][3]*temp.w;

    temp.x = nx-x0;
    temp.y = ny-y0;
    temp.z = nz-(z0+1);
    s[3] = gradient4[gi12][0]*temp.x + gradient4[gi12][1]*temp.y + gradient4[gi12][2]*temp.z + gradient4[gi12][3]*temp.w;

    temp.x = nx-(x0+1);
    temp.y = ny-y0;
    t[3] = gradient4[gi13][0]*temp.x + gradient4[gi13][1]*temp.y + gradient4[gi13][2]*temp.z + gradient4[gi13][3]*temp.w;

    temp.x = nx-x0;
    temp.y = ny-(y0+1);
    u[3] = gradient4[gi14][0]*temp.x + gradient4[gi14][1]*temp.y + gradient4[gi14][2]*temp.z + gradient4[gi14][3]*temp.w;

    temp.x = nx-(x0+1);
    temp.y = ny-(y0+1);
    v[3] = gradient4[gi15][0]*temp.x + gradient4[gi15][1]*temp.y + gradient4[gi15][2]*temp.z + gradient4[gi15][3]*temp.w;

    tmp = nx-x0;
    Cx = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    Li1 = s[0] + Cx*(t[0]-s[0]);
    Li2 = u[0] + Cx*(v[0]-u[0]);
    Li3 = s[1] + Cx*(t[1]-s[1]);
    Li4 = u[1] + Cx*(v[1]-u[1]);
    Li5 = s[2] + Cx*(t[2]-s[2]);
    Li6 = u[2] + Cx*(v[2]-u[2]);
    Li7 = s[3] + Cx*(t[3]-s[3]);
    Li8 = u[3] + Cx*(v[3]-u[3]);

    tmp = ny-y0;
    Cy = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    Li9 = Li1 + Cy*(Li2-Li1);
    Li10 = Li3 + Cy*(Li4-Li3);
    Li11 = Li5 + Cy*(Li6-Li5);
    Li12 = Li7 + Cy*(Li8-Li7);

    tmp = nz-z0;
    Cz = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    Li13 = Li9 + Cz*(Li10-Li9);
    Li14 = Li11 + Cz*(Li12-Li11);

    tmp = nw-w0;
    Cw = tmp * tmp * tmp * (tmp * (tmp * 6 - 15) + 10);

    return Li13 + Cw*(Li14-Li13);
}

//------------------------------ SIMPLEX ------------------------------

float NzNoiseMachine::Get2DSimplexNoiseValue(float x, float y, float resolution)
{
    x /= resolution;
    y /= resolution;


    Origin.x = fastfloor(x + (x + y) * SkewCoeff2D);
    Origin.y = fastfloor(y + (x + y) * SkewCoeff2D);

    A.x = Origin.x - (Origin.x + Origin.y) * UnskewCoeff2D;
    A.y = Origin.y - (Origin.x + Origin.y) * UnskewCoeff2D;

    IsoOriginDist.x = x - A.x;
    IsoOriginDist.y = y - A.y;

    if(IsoOriginDist.x > IsoOriginDist.y)
    {
        off1.x = 1;
        off1.y = 0;
    }
    else
    {
        off1.x = 0;
        off1.y = 1;
    }

    d1.x = A.x - x;
    d1.y = A.y - y;

    d2.x = d1.x + off1.x - UnskewCoeff2D;
    d2.y = d1.y + off1.y - UnskewCoeff2D;

    d3.x = d1.x + 1.0 - 2 * UnskewCoeff2D;
    d3.y = d1.y + 1.0 - 2 * UnskewCoeff2D;

    ii = Origin.x & 255;
    jj = Origin.y & 255;

    gi0 = perm[ii + perm[jj]] % 12;
    gi1 = perm[ii + off1.x + perm[jj + off1.y]] % 12;
    gi2 = perm[ii + 1 + perm[jj + 1]] % 12;

    H[0].x = gradient3[gi0][0];
    H[0].y = gradient3[gi0][1];

    H[1].x = gradient3[gi1][0];
    H[1].y = gradient3[gi1][1];

    H[2].x = gradient3[gi2][0];
    H[2].y = gradient3[gi2][1];

    n1 = H[0].x * d1.x + H[0].y * d1.y;
    n2 = H[1].x * d2.x + H[1].y * d2.y;
    n3 = H[2].x * d3.x + H[2].y * d3.y;

    lenght = 0.5;

    c1 = lenght - d1.x * d1.x - d1.y * d1.y;
    c2 = lenght - d2.x * d2.x - d2.y * d2.y;
    c3 = lenght - d3.x * d3.x - d3.y * d3.y;

    if(c1 < 0)
        c1 = 0;
    if(c2 < 0)
        c2 = 0;
    if(c3 < 0)
        c3 = 0;

    n1 = c1*c1*c1*n1;
    n2 = c2*c2*c2*n2;
    n3 = c3*c3*c3*n3;

    return (n1+n2+n3)*23.2;
}

float NzNoiseMachine::Get3DSimplexNoiseValue(float x, float y, float z, float resolution)
{
    x /= resolution;
    y /= resolution;
    z /= resolution;

    Origin.x = fastfloor(x + (x + y + z) * SkewCoeff3D);
    Origin.y = fastfloor(y + (x + y + z) * SkewCoeff3D);
    Origin.z = fastfloor(z + (x + y + z) * SkewCoeff3D);

    A.x = Origin.x - (Origin.x + Origin.y + Origin.z) * UnskewCoeff3D;
    A.y = Origin.y - (Origin.x + Origin.y + Origin.z) * UnskewCoeff3D;
    A.z = Origin.z - (Origin.x + Origin.y + Origin.z) * UnskewCoeff3D;

    IsoOriginDist.x = x - A.x;
    IsoOriginDist.y = y - A.y;
    IsoOriginDist.z = z - A.z;

    if(IsoOriginDist.x >= IsoOriginDist.y)
    {
        if(IsoOriginDist.y >= IsoOriginDist.z)
        {
            off1.x = 1;
            off1.y = 0;
            off1.z = 0;
            off2.x = 1;
            off2.y = 1;
            off2.z = 0;
        }
        else if(IsoOriginDist.x >= IsoOriginDist.z)
        {
            off1.x = 1;
            off1.y = 0;
            off1.z = 0;
            off2.x = 1;
            off2.y = 0;
            off2.z = 1;
        }
        else
        {
            off1.x = 0;
            off1.y = 0;
            off1.z = 1;
            off2.x = 1;
            off2.y = 0;
            off2.z = 1;
        }
    }
    else
    {
        if(IsoOriginDist.y < IsoOriginDist.z)
        {
            off1.x = 0;
            off1.y = 0;
            off1.z = 1;
            off2.x = 0;
            off2.y = 1;
            off2.z = 1;
        }
        else if(IsoOriginDist.x < IsoOriginDist.z)
        {
            off1.x = 0;
            off1.y = 1;
            off1.z = 0;
            off2.x = 0;
            off2.y = 1;
            off2.z = 1;
        }
        else
        {
            off1.x = 0;
            off1.y = 1;
            off1.z = 0;
            off2.x = 1;
            off2.y = 1;
            off2.z = 0;
        }
    }

    d1.x = A.x - x;
    d1.y = A.y - y;
    d1.z = A.z - z;

    d2.x = d1.x + off1.x - UnskewCoeff3D;
    d2.y = d1.y + off1.y - UnskewCoeff3D;
    d2.z = d1.z + off1.z - UnskewCoeff3D;

    d3.x = d1.x + off2.x - 2*UnskewCoeff3D;
    d3.y = d1.y + off2.y - 2*UnskewCoeff3D;
    d3.z = d1.z + off2.z - 2*UnskewCoeff3D;

    d4.x = d1.x + 1.0 - 3*UnskewCoeff3D;
    d4.y = d1.y + 1.0 - 3*UnskewCoeff3D;
    d4.z = d1.z + 1.0 - 3*UnskewCoeff3D;

    ii = Origin.x & 255;
    jj = Origin.y & 255;
    kk = Origin.z & 255;

    gi0 = perm[ii + perm[jj + perm[kk]]] % 12;
    gi1 = perm[ii + off1.x + perm[jj + off1.y + perm[kk + off1.z]]] % 12;
    gi2 = perm[ii + off2.x + perm[jj + off2.y + perm[kk + off2.z]]] % 12;
    gi3 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1]]] % 12;

    n1 = gradient3[gi0][0] * d1.x + gradient3[gi0][1] * d1.y + gradient3[gi0][2] * d1.z;
    n2 = gradient3[gi1][0] * d2.x + gradient3[gi1][1] * d2.y + gradient3[gi1][2] * d2.z;
    n3 = gradient3[gi2][0] * d3.x + gradient3[gi2][1] * d3.y + gradient3[gi2][2] * d3.z;
    n4 = gradient3[gi3][0] * d4.x + gradient3[gi3][1] * d4.y + gradient3[gi3][2] * d4.z;

    lenght = 0.6;

    c1 = lenght - d1.x * d1.x - d1.y * d1.y - d1.z * d1.z;
    c2 = lenght - d2.x * d2.x - d2.y * d2.y - d2.z * d2.z;
    c3 = lenght - d3.x * d3.x - d3.y * d3.y - d3.z * d3.z;
    c4 = lenght - d4.x * d4.x - d4.y * d4.y - d4.z * d4.z;

    if(c1 < 0)
        c1 = 0;
    if(c2 < 0)
        c2 = 0;
    if(c3 < 0)
        c3 = 0;
    if(c4 < 0)
        c4 = 0;

    n1 = c1*c1*c1*n1;
    n2 = c2*c2*c2*n2;
    n3 = c3*c3*c3*n3;
    n4 = c4*c4*c4*n4;

    return (n1+n2+n3+n4)*17.6995;
}

float NzNoiseMachine::Get4DSimplexNoiseValue(float x, float y, float z, float w, float resolution)
{
    x /= resolution;
    y /= resolution;
    z /= resolution;
    w /= resolution;

    Origin.x = fastfloor(x + (x + y + z + w) * SkewCoeff4D);
    Origin.y = fastfloor(y + (x + y + z + w) * SkewCoeff4D);
    Origin.z = fastfloor(z + (x + y + z + w) * SkewCoeff4D);
    Origin.w = fastfloor(w + (x + y + z + w) * SkewCoeff4D);

    A.x = Origin.x - (Origin.x + Origin.y + Origin.z + Origin.w) * UnskewCoeff4D;
    A.y = Origin.y - (Origin.x + Origin.y + Origin.z + Origin.w) * UnskewCoeff4D;
    A.z = Origin.z - (Origin.x + Origin.y + Origin.z + Origin.w) * UnskewCoeff4D;
    A.w = Origin.w - (Origin.x + Origin.y + Origin.z + Origin.w) * UnskewCoeff4D;

    IsoOriginDist.x = x - A.x;
    IsoOriginDist.y = y - A.y;
    IsoOriginDist.z = z - A.z;
    IsoOriginDist.w = w - A.w;

    c1 = (IsoOriginDist.x > IsoOriginDist.y) ? 32 : 0;
    c2 = (IsoOriginDist.x > IsoOriginDist.z) ? 16 : 0;
    c3 = (IsoOriginDist.y > IsoOriginDist.z) ? 8  : 0;
    c4 = (IsoOriginDist.x > IsoOriginDist.w) ? 4  : 0;
    c5 = (IsoOriginDist.y > IsoOriginDist.w) ? 2  : 0;
    c6 = (IsoOriginDist.z > IsoOriginDist.w) ? 1  : 0;
    c = c1 + c2 + c3 + c4 + c5 + c6;

    off1.x = lookupTable4D[c][0] >= 3 ? 1 : 0;
    off1.y = lookupTable4D[c][1] >= 3 ? 1 : 0;
    off1.z = lookupTable4D[c][2] >= 3 ? 1 : 0;
    off1.w = lookupTable4D[c][3] >= 3 ? 1 : 0;

    off2.x = lookupTable4D[c][0] >= 2 ? 1 : 0;
    off2.y = lookupTable4D[c][1] >= 2 ? 1 : 0;
    off2.z = lookupTable4D[c][2] >= 2 ? 1 : 0;
    off2.w = lookupTable4D[c][3] >= 2 ? 1 : 0;

    off3.x = lookupTable4D[c][0] >= 1 ? 1 : 0;
    off3.y = lookupTable4D[c][1] >= 1 ? 1 : 0;
    off3.z = lookupTable4D[c][2] >= 1 ? 1 : 0;
    off3.w = lookupTable4D[c][3] >= 1 ? 1 : 0;

    d1.x = A.x - x;
    d1.y = A.y - y;
    d1.z = A.z - z;
    d1.w = A.w - w;

    d2.x = d1.x + off1.x - UnskewCoeff4D;
    d2.y = d1.y + off1.y - UnskewCoeff4D;
    d2.z = d1.z + off1.z - UnskewCoeff4D;
    d2.w = d1.w + off1.w - UnskewCoeff4D;

    d3.x = d1.x + off2.x - 2*UnskewCoeff4D;
    d3.y = d1.y + off2.y - 2*UnskewCoeff4D;
    d3.z = d1.z + off2.z - 2*UnskewCoeff4D;
    d3.w = d1.w + off2.w - 2*UnskewCoeff4D;

    d4.x = d1.x + off3.x - 3*UnskewCoeff4D;
    d4.y = d1.y + off3.y - 3*UnskewCoeff4D;
    d4.z = d1.z + off3.z - 3*UnskewCoeff4D;
    d4.w = d1.w + off3.w - 3*UnskewCoeff4D;

    d5.x = d1.x + 1.0 - 4*UnskewCoeff4D;
    d5.y = d1.y + 1.0 - 4*UnskewCoeff4D;
    d5.z = d1.z + 1.0 - 4*UnskewCoeff4D;
    d5.w = d1.w + 1.0 - 4*UnskewCoeff4D;

    ii = Origin.x & 255;
    jj = Origin.y & 255;
    kk = Origin.z & 255;
    ll = Origin.w & 255;

    gi0 = perm[ii + perm[jj + perm[kk + perm[ll]]]] % 32;
    gi1 = perm[ii + off1.x + perm[jj + off1.y + perm[kk + off1.z + perm[ll + off1.w]]]] % 32;
    gi2 = perm[ii + off2.x + perm[jj + off2.y + perm[kk + off2.z + perm[ll + off2.w]]]] % 32;
    gi3 = perm[ii + off3.x + perm[jj + off3.y + perm[kk + off3.z + perm[ll + off3.w]]]] % 32;
    gi4 = perm[ii + 1 + perm[jj + 1 + perm[kk + 1 + perm[ll + 1]]]] % 32;

    n1 = gradient4[gi0][0]*d1.x + gradient4[gi0][1]*d1.y + gradient4[gi0][2]*d1.z + gradient4[gi0][3]*d1.w;
    n2 = gradient4[gi1][0]*d2.x + gradient4[gi1][1]*d2.y + gradient4[gi1][2]*d2.z + gradient4[gi1][3]*d2.w;
    n3 = gradient4[gi2][0]*d3.x + gradient4[gi2][1]*d3.y + gradient4[gi2][2]*d3.z + gradient4[gi2][3]*d3.w;
    n4 = gradient4[gi3][0]*d4.x + gradient4[gi3][1]*d4.y + gradient4[gi3][2]*d4.z + gradient4[gi3][3]*d4.w;
    n5 = gradient4[gi4][0]*d5.x + gradient4[gi4][1]*d5.y + gradient4[gi4][2]*d5.z + gradient4[gi4][3]*d5.w;

    lenght = 0.6;

    c1 = lenght - d1.x*d1.x - d1.y*d1.y - d1.z*d1.z - d1.w*d1.w;
    c2 = lenght - d2.x*d2.x - d2.y*d2.y - d2.z*d2.z - d2.w*d2.w;
    c3 = lenght - d3.x*d3.x - d3.y*d3.y - d3.z*d3.z - d3.w*d3.w;
    c4 = lenght - d4.x*d4.x - d4.y*d4.y - d4.z*d4.z - d4.w*d4.w;
    c5 = lenght - d5.x*d5.x - d5.y*d5.y - d5.z*d5.z - d5.w*d5.w;

    if(c1 < 0)
        c1 = 0;
    if(c2 < 0)
        c2 = 0;
    if(c3 < 0)
        c3 = 0;
    if(c4 < 0)
        c4 = 0;
    if(c5 < 0)
        c5 = 0;

    n1 = c1*c1*c1*n1;
    n2 = c2*c2*c2*n2;
    n3 = c3*c3*c3*n3;
    n4 = c4*c4*c4*n4;
    n5 = c5*c5*c5*n5;

    return (n1+n2+n3+n4+n5)*17.6995;
}

//------------------------------ CELL ------------------------------

float NzNoiseMachine::Get2DCellNoiseValue(float x, float y, float res)
{
    return 0;
}

float NzNoiseMachine::Get3DCellNoiseValue(float x, float y, float z, float res)
{
    x /= res;
    y /= res;
    z /= res;

    x0 = (int)(x);
    y0 = (int)(y);
    z0 = (int)(z);

    return 0;
}
float NzNoiseMachine::Get4DCellNoiseValue(float x, float y, float z, float w, float res)
{
    x /= res;
    y /= res;
    z /= res;
    w /= res;

    x0 = (int)(x) & 255;
    y0 = (int)(y) & 255;
    z0 = (int)(z) & 255;
    w0 = (int)(w) & 255;

    return 0;
}

void NzNoiseMachine::SetLacunarity(float lacunarity)
{
    if(lacunarity != m_lacunarity)
    {
        m_lacunarity = lacunarity;
        m_parametersModified = true;
    }
}

void NzNoiseMachine::SetHurstParameter(float h)
{
    if(h != m_hurst)
    {
        m_hurst = h;
        m_parametersModified = true;
    }
}

void NzNoiseMachine::SetOctavesNumber(float octaves)
{
    if(octaves != m_octaves && octaves < 30)
    {
        m_octaves = octaves;
        m_parametersModified = true;
    }
}

void NzNoiseMachine::RecomputeExponentArray()
{
    if(m_parametersModified)
    {
        float frequency = 1.0;
        m_sum = 0.f;
        for (int i(0) ; i < m_octaves; ++i)
        {
            exponent_array[i] = pow( frequency, -m_hurst );
            frequency *= m_lacunarity;

            m_sum += exponent_array[i];

        }
        m_parametersModified = false;
    }
}

//------------------------------ FBM ------------------------------

float NzNoiseMachine::Get1DFBMNoiseValue(float x, float res)
{
    value = 0.0;
    RecomputeExponentArray();

    for (int i(0); i < m_octaves; ++i)
    {
        value += Get1DPerlinNoiseValue(x,res) * exponent_array[i];
        x *= m_lacunarity;
    }
    remainder = m_octaves - (float)m_octaves;

    if(remainder != 0)
        value += remainder * Get1DPerlinNoiseValue(x,res) * exponent_array[(int)m_octaves-1];

    return value * m_sum;
}

float NzNoiseMachine::Get2DFBMNoiseValue(float x, float y, float res)
{
    value = 0.0;
    RecomputeExponentArray();

    for (int i(0); i < m_octaves; ++i)
    {
        value += Get2DPerlinNoiseValue(x,y,res) * exponent_array[i];
        x *= m_lacunarity;
        y *= m_lacunarity;
    }
    remainder = m_octaves - (int)m_octaves;

    if(remainder != 0)
        value += remainder * Get2DSimplexNoiseValue(x,y,res) * exponent_array[(int)m_octaves-1];

    return value * m_sum;
}

float NzNoiseMachine::Get3DFBMNoiseValue(float x, float y, float z, float res)
{
    value = 0.0;
    RecomputeExponentArray();

    for(int i(0); i < m_octaves; ++i)
    {
        value += Get3DSimplexNoiseValue(x,y,z,res) * exponent_array[i];
        x *= m_lacunarity;
        y *= m_lacunarity;
        z *= m_lacunarity;
    }
    remainder = m_octaves - (int)m_octaves;

    if(remainder != 0)
        value += remainder * Get3DSimplexNoiseValue(x,y,z,res) * exponent_array[(int)m_octaves-1];

    return value * m_sum;
}

//------------------------------ HYBRID MULTIFRACTAL ------------------------------

float NzNoiseMachine::Get2DHybridMultiFractalNoiseValue(float x, float y, float res)
{
    float result, signal, weight, remainder;
    float offset = 1;

    RecomputeExponentArray();

    result = (Get2DSimplexNoiseValue(x,y,res) + offset) * exponent_array[0];
    weight = result;

    x *= m_lacunarity;
    y *= m_lacunarity;

    for(int i(1) ; i < m_octaves; ++i)
    {
        if(weight > 1.0)
            weight = 1.0;

        signal = (Get2DSimplexNoiseValue(x,y,res) + offset) * exponent_array[i];
        result += weight * signal;

        weight *= signal;

        x *= m_lacunarity;
        y *= m_lacunarity;

    }

    remainder = m_octaves - (int)m_octaves;

    if(remainder != 0)
        result += remainder * Get2DSimplexNoiseValue(x,y,res) * exponent_array[(int)m_octaves-1];

    return result;

}

float NzNoiseMachine::Get3DHybridMultiFractalNoiseValue(float x, float y, float z, float res)
{
    float result, signal, weight, remainder;
    float offset = 1;

    RecomputeExponentArray();

    result = (Get3DSimplexNoiseValue(x,y,z,res) + offset) * exponent_array[0];
    weight = result;

    x *= m_lacunarity;
    y *= m_lacunarity;

    for(int i(1) ; i < m_octaves; ++i)
    {

        if(weight > 1.0)
            weight = 1.0;

        signal = ( Get3DSimplexNoiseValue(x,y,z,res) + offset ) * exponent_array[i];
        result += weight * signal;

        weight *= signal;

        x *= m_lacunarity;
        y *= m_lacunarity;

    }

    remainder = m_octaves - (int)m_octaves;

    if(remainder != 0)
        result += remainder * Get3DSimplexNoiseValue(x,y,z,res) * exponent_array[(int)m_octaves-1];

    return result;
}

int NzNoiseMachine::fastfloor(float n)
{
    if(n >= 0)
        return int(n);
    else
        return int(n-1);
}
