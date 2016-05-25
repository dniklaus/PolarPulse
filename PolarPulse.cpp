/*
 * PolarPulse.cpp
 *
 *  Created on: 15.05.2016
 *      Author: niklausd
 */

#include <Timer.h>
#include <Arduino.h>

#include "PolarPulse.h"
#include <DbgCliCommandPulseSim.h>

const bool PolarPulse::IS_POS_LOGIC = false;
const bool PolarPulse::IS_NEG_LOGIC = true;
const int  PolarPulse::PLS_NC       = -1;
const int  PolarPulse::IND_NC       = -1;

const unsigned int PolarPulse::s_defaultPulsePollTimeMillis  = 5;
const unsigned int PolarPulse::s_defaultReportIntervalMillis = 5000;

//-----------------------------------------------------------------------------

class PollingTimerAdapter : public TimerAdapter
{
private:
  PolarPulse* m_pulseSesor;
  bool m_lastWasPulseActive;
public:
  PollingTimerAdapter(PolarPulse* pulseSesor)
  : m_pulseSesor(pulseSesor)
  , m_lastWasPulseActive(false)
  { }

  void timeExpired()
  {
    if (0 != m_pulseSesor)
    {
      bool currentIsPulseActive = m_pulseSesor->isPulseActive();

      if (m_lastWasPulseActive != currentIsPulseActive)
      {
        m_lastWasPulseActive = currentIsPulseActive;
        m_pulseSesor->setIndicator(currentIsPulseActive);
        if (currentIsPulseActive)
        {
          m_pulseSesor->countPulse();
        }
      }
    }
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
    if (0 != m_pulseSesor)
    {
      m_pulseSesor->reportInterval();
    }
  }
};

//-----------------------------------------------------------------------------

PolarPulse::PolarPulse(int pulsePin, int indicatorPin, bool isPulsePinNegativeLogic, PolarPulseAdapter* adapter)
: m_pollingTimer(new Timer(new PollingTimerAdapter(this), Timer::IS_RECURRING, s_defaultPulsePollTimeMillis))
, m_reportTimer(new Timer(new ReportTimerAdapter(this), Timer::IS_RECURRING, s_defaultReportIntervalMillis))
, m_adapter(adapter)
, m_dbgPulseSimCmd(new DbgCli_Command_PulseSim(this))
, m_isPulsePinNegativeLogic(isPulsePinNegativeLogic)
, m_count(false)
, m_heartBeatRate(0)
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
    digitalWrite(m_indicatorPin, m_count);
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

void PolarPulse::countPulse()
{
  m_count++;
}

void PolarPulse::reportInterval()
{
  const unsigned int c_extrapolationFactor = 60000 / s_defaultReportIntervalMillis;
  m_heartBeatRate = m_count * c_extrapolationFactor;
  m_count = 0;
  if (0 != m_adapter)
  {
    m_adapter->notifyHeartBeatRate(m_heartBeatRate);
  }
}

void PolarPulse::setIndicator(bool isActive)
{
  if (0 <= m_indicatorPin)
  {
    digitalWrite(m_indicatorPin, !isActive);
  }
}
