#include "Depth.hpp"
#include <Hal/KellerLD.h>

Depth::Depth()
{
  bar100.init();
  bar100.setFluidDensity(1029); // kg/m^3 (freshwater, 1029 for seawater)

  if (bar100.isInitialized())
  {
    log_d("Sensor bar100 connected.");
    sensor = BAR_100;
  }
  else
  {
    log_d("Sensor bar100 not connected.");
    bar30.init();
    if (bar30.isInitialised())
    {
      log_d("Sensor bar30 connected.");
      sensor = BAR_30;
    }
  }
}

depth Depth::getDepth()
{
  if (sensor == BAR_30)
    return bar30.getDepth();
  else if (sensor == BAR_100)
    return bar100.getDepth();
  else
    return NAN;
}

temperature Depth::getTemp()
{
  if (sensor == BAR_30)
    return bar30.getTemp();
  else if (sensor == BAR_100)
    return bar100.getTemp();
  else
    return NAN;
}

pressure Depth::getPressure()
{
  if (sensor == BAR_30)
    return bar30.getPressure();
  else if (sensor == BAR_100)
    return bar100.getPressure();
  else
    return NAN;
}
