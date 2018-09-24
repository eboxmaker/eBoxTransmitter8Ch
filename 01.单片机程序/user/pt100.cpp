#include "pt100.h"

PtData_t pt100;


double RtoT(double R, uint8_t type)
{
    uint16_t R0;// 
    double T, tmp, min, mid, max;
    double x, y, z;
    double d = 3.9083/1e3, b = -5.775/1e7, c = -4.183/1e12;
    switch(type)
    {
        case 0: R0 = 10; min = 1.852; mid = 10; max = 39.049;break;// Pt10
        case 1: R0 = 100; min = 18.52; mid = 100; max = 390.49;break; // Pt100
        case 2: R0 = 1000; min = 185.2; mid = 1000; max = 3904.9;break; // Pt1000
//        case 3: R0 = 50;break; // Cu50
//        case 4: R0 = 100;break; // Cu100
        default:
             R0 = 100; min = 18.52; mid = 100; max = 390.49;break; // Pt100
    }
    
    switch(type)
    {
        case 0:
        case 1:
        case 2:
        default:
                if(R>=mid && R<=max)
                {
                    y = sqrt(d*d - 4*b*(1-R/R0));
                    T = (y-d) / (2*b); // 
                }else if(R<mid && R>=min)
                {
                    x = 0; y = -100; z = -201;
                    tmp = R0*(1 + d*y + b*y*y + c*(y-100)*y*y*y);
                    while(fabs(tmp-R)>=0.007)
                    {
                        if(R > tmp)
                        {
                            x = x; z = y;
                            y = (x+y)/2;
                        }
                        else
                        {
                            x = y; z = z;
                            y = (y+z)/2;
                        }
                        tmp = R0*(1 + d*y + b*y*y + c*(y-100)*y*y*y);
                    }
                    T = y; // 
                }
                break;
//        case 3:
//        case 4:
//                x = 150; y = 50; z = -50;
//                tmp = R0*(1 + 4.289/1e3*y - 2.133/1e7*y*y + 1.233/1e9*y*y*y);
//                while(fabs(tmp-R)>=0.007)
//                {
//                    if(R > tmp)
//                    {
//                        x = x; z = y; 
//                        y = (x+y)/2;
//                    }
//                    else
//                    {
//                        x = y; z = z;
//                        y = (y+z)/2;
//                    }
//                    tmp = R0*(1 + 4.28899/1e3*y - 2.133/1e7*y*y + 1.233/1e9*y*y*y);
//                }
//                T = y;
//                break;
            
    }
/*
    if(type <= 1)
    {
        if(type == 0)
        {
            R0 = 10; min = 1.852; mid = 10; max = 39.049;// Pt10
        }else
        {
            R0 = 100; min = 18.52; mid = 100; max = 390.49; // Pt100
        }
        if(R>=mid && R<=max)
        {
            y = sqrt(d*d - 4*b*(1-R/R0));
            T = (y-d) / (2*b); // ????
        }else if(R<mid && R>=min)
        {
            x = 0; y = -100; z = -201;
            tmp = R0*(1 + d*y + b*y*y + c*(y-100)*y*y*y);
            while(fabs(tmp-R)>=0.007)
            {
                if(R > tmp)
                {
                    x = x; z = y;
                    y = (x+y)/2;
                }
                else
                {
                    x = y; z = z;
                    y = (y+z)/2;
                }
                tmp = R0*(1 + d*y + b*y*y + c*(y-100)*y*y*y);
            }
            T = y; // ????
        }
    }
    else
    {
        if(type == 2)// Cu50
            R0 = 50;
        else // Cu100
            R0 = 100;
            
        x = 150; y = 50; z = -50;
        tmp = R0*(1 + 4.289/1e3*y - 2.133/1e7*y*y + 1.233/1e9*y*y*y);
        while(fabs(tmp-R)>=0.007)
        {
            if(R > tmp)
            {
                x = x; z = y; 
                y = (x+y)/2;
            }
            else
            {
                x = y; z = z;
                y = (y+z)/2;
            }
            tmp = R0*(1 + 4.28899/1e3*y - 2.133/1e7*y*y + 1.233/1e9*y*y*y);
        }
        T = y;
    }
*/
    return T;
}

