/*
 * PolarPulse.cpp
 *
 *  Created on: 15.05.2016
 *      Author: niklausd
 */

#include <Timer.h>
#include <Arduino.h>

#include "PolarPulse.h"

const bool PolarPulse::IS_POS_LOGIC = false;
const bool PolarPulse::IS_NEG_LOGIC = true;
const int  PolarPulse::PLS_NC       = -1;
const int  PolarPulse::IND_NC       = -1;

const int  PolarPulse::s_defaultPulsePollTimeMillis = 5;
const int  PolarPulse::s_defaultReportIntervalMillis = 15000;

//-----------------------------------------------------------------------------

class PollingTimerAdapter : public TimerAdapter
{
private:
  PolarPulse* m_pulseSesor;
  bool m_lastWasPulseHigh;
public:
  PollingTimerAdapter(PolarPulse* pulseSesor)
  : m_pulseSesor(pulseSesor)
  , m_lastWasPulseHigh(false)
  { }

  void timeExpired()
  {

  }
};

//-----------------------------------------------------------------------------

class ReportTimerAdapter : public TimerAdapter
{
private:
  PolarPulse* m_pulseSesor;
public:
  ReportTimerAdapter(PolarPulse* pulseSesor)
  : m_pulseSesor(pulseSesor)
  { }

  void timeExpired()
  {

  }
};

//-----------------------------------------------------------------------------

PolarPulse::PolarPulse(int pulsePin, int indicatorPin, bool isPulsePinNegativeLogic, PolarPulseAdapter* adapter)
: m_pollingTimer(new Timer(new PollingTimerAdapter(this), Timer::IS_RECURRING, s_defaultPulsePollTimeMillis))
, m_reportTimer(new Timer(new ReportTimerAdapter(this), Timer::IS_RECURRING, s_defaultReportIntervalMillis))
, m_adapter(adapter)
, m_isPulsePinNegativeLogic(isPulsePinNegativeLogic)
, m_isActive(false)
, m_pulsePin(pulsePin)
, m_indicatorPin(indicatorPin)
{
  if (0 <= m_pulsePin)
  {
    pinMode(m_pulsePin, INPUT);
    digitalWrite(m_pulsePin, m_isPulsePinNegativeLogic ? HIGH : LOW); // pull
  }
  if (0 <= m_indicatorPin)
  {
    pinMode(m_indicatorPin, OUTPUT);
    digitalWrite(m_indicatorPin, m_isActive);
  }
}

PolarPulse::~PolarPulse()
{
  delete m_pollingTimer->adapter();
  m_pollingTimer->attachAdapter(0);

  delete m_pollingTimer;
  m_pollingTimer = 0;

  delete m_reportTimer->adapter();
  m_reportTimer->attachAdapter(0);

  delete m_reportTimer;
  m_reportTimer = 0;
}

PolarPulseAdapter* PolarPulse::adapter()
{
  return m_adapter;
}

void PolarPulse::attachAdapter(PolarPulseAdapter* adapter)
{
  m_adapter = adapter;
}

bool PolarPulse::isPulseActive()
{
  bool active = false;
  if (0 <= m_pulsePin)
  {
    active = digitalRead(m_pulsePin);
    active = (m_isPulsePinNegativeLogic ? !active : active);
  }
  return active;
}
