/*
 * PolarPulse.h
 *
 *  Created on: 15.05.2016
 *      Author: niklausd
 */

#ifndef COMP_POLARPULSE_POLARPULSE_H_
#define COMP_POLARPULSE_POLARPULSE_H_

class Timer;
class DbgCli_Topic;
class DbgCli_Command_PulseSim;
class DbgCli_Command_PulseGen;

//-----------------------------------------------------------------------------

class PolarPulseAdapter
{
public:
  /**
   * Get Heart Beat Pulse Count from an external counter since last call of this method.
   * Can be implemented optionally, useful i.e. when pulse pin is triggering an ISR.
   * @return count Heart Beat Pulse Count since last call.
   */
  virtual unsigned int getCount() { return 0; }

  /**
   * Notify a heart beat rate.
   * @param  heartBeatRate Array of heart beat rate values [1/min]
   * @param  numOfValues Number of heartBeatRate array elements
   */
  virtual void notifyHeartBeatRate(unsigned int* heartBeatRate, unsigned char numOfValues) = 0;
  
  /**
   * Notify a heart beat rate.
   * @param  heartBeatRate Heart beat rate value [1/min]
   */
  virtual void notifyHeartBeatRate(unsigned int heartBeatRate) = 0;

protected:
  PolarPulseAdapter() { }
  virtual ~PolarPulseAdapter() { }

private:  // forbidden functions
  PolarPulseAdapter(const PolarPulseAdapter& src);              // copy constructor
  PolarPulseAdapter& operator = (const PolarPulseAdapter& src); // assignment operator
};

//-----------------------------------------------------------------------------

/**
 * Polar Heart Rate sensor model.
 * The Polar Heart Rate sensor (https://www.adafruit.com/product/1077) events are recognized.
 * The events are counted, the number of events are reported any 15 seconds, extrapolated the rate per minute.
 * The puls input pin can be configured, so also it's logic (rising or falling edge).
 * A pulse indicator output can be defined.
 */
class PolarPulse
{
public:
  /**
   * Constructor.
   * @param pulsePin                Arduino Pin where the pulse sensor is connected to; default: PLS_NC (not connected)
   * @param indicatorPin            Arduino Pin where the (LED) status indication is connected to; default: IND_NC (not connected)
   * @param isPulsePinNegativeLogic Consider active pulse starting with a High to LOW edge (true) or LOW to HIGH edge (false);
   *                                default: false (pulse sensor object triggering on the rising egde)
   * @param adapter                 Inject pointer to PolarPulseAdapter object; NULL pointer: no status change notification will be sent out.
   */
  PolarPulse(int pulsePin = PLS_NC, int indicatorPin = IND_NC, bool isPulsePinNegativeLogic = false, PolarPulseAdapter* adapter = 0);
  virtual ~PolarPulse();

  /**
   * Retrieve pointer to currently injected adapter object.
   * @return Pointer to currently injected PolarPulseAdapter object; NULL pointer if none is injected.
   */
  PolarPulseAdapter* adapter();

  /**
   * Attach adapter object.
   * @param adapter Pointer to specific PolarPulseAdapter object; NULL pointer: no status change notification will be sent out.
   */
  void attachAdapter(PolarPulseAdapter* adapter);

  /**
   * Retreive pointer to this component's debug topic node object.
   * @return dbgTopic Pointer to this component's debug topic node object.
   */
  DbgCli_Topic* dbgTopic();

  /**
   * Retreive pointer to this component's debug pulse generator topic object.
   * @return dbgPulseGenCmd Pointer to this component's debug pulse generator topic object.
   */
  DbgCli_Command_PulseGen* dbgPulseGenCmd();

  /**
   * Retrieve current pulse status.
   * @return Pulse status: active (true) or inactive (false).
   */
  bool isPulseActive();

  /**
   *
   */
  bool isPulseDetectedExternally();

  /**
   * Count one or add multiple Pulses.
   * @param count Counts to be added if not 0, else increment b one count (default).
   */
  void countPulse(unsigned int count = 0);

  /**
   * Report Interval is over, extrapolate the heart beat rate per minute.
   */
  void reportInterval();

  /**
   * Set indicator pin.
   * @param isActive Indicator set active.
   */
  void setIndicator(bool isActive);

  /**
   * Constant for isButtonNegativeLogic parameter of the constructor (@see PolarPulse()), to create a pulse sensor object triggering on the rising egde.
   */
  static const bool IS_POS_LOGIC;

  /**
   * Constant for isButtonNegativeLogic parameter of the constructor (@see PolarPulse()), to create a pulse sensor object triggering on the falling egde.
   */
  static const bool IS_NEG_LOGIC;

  /**
   * Constant for constructor's pulsePin parameter (@see PolarPulse()), to create a pulse sensor object w/o any pulse sensor connected to a pin, i.e. useful for tests.
   */
  static const int PLS_NC;

  /**
   * Constant for constructor's indicatorPin parameter (@see PolarPulse()), to create a pulse sensor object w/o any indicator output connected to a pin.
   */
  static const int IND_NC;

private:
  Timer* m_pollingTimer;
  Timer* m_reportTimer;
  PolarPulseAdapter* m_adapter;
  DbgCli_Topic* m_dbgTopic;
  DbgCli_Command_PulseSim* m_dbgPulseSimCmd;
  DbgCli_Command_PulseGen* m_dbgPulseGenCmd;
  bool m_isPulsePinNegativeLogic;
  unsigned int m_count;
  unsigned int* m_heartBeatRate;
  int m_pulsePin;
  int m_indicatorPin;
  static const unsigned int s_defaultPulsePollTimeMillis;
  static const unsigned int s_defaultExternalPulsePollTimeMillis;
  static const unsigned int s_defaultReportIntervalMillis;
  static const unsigned int s_defaultReportOveralBufferTimeMillis;
  static const unsigned int s_oneMinuteMillis;
  const unsigned char m_cMaxReportStores;
  unsigned char m_reportPeriodCount;

private:  // forbidden functions
  PolarPulse(const PolarPulse& src);              // copy constructor
  PolarPulse& operator = (const PolarPulse& src); // assignment operator
};

#endif /* COMP_POLARPULSE_POLARPULSE_H_ */
