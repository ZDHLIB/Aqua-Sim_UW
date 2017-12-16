#ifndef __uw_h__
#define __uw_h__


#include <math.h>
#include <stdio.h>
#include <stdlib.h>


class UnderwaterChannelMetrics {
   
   public:
      double Pathloss(double, double);
      double thorp(double);
      double noise(double);
      double PER(double, int);
      double NTX(double);
      double utilization(double, double, double, double);
};



#endif
