/*
 * PolarPulse.cpp
 *
 *  Created on: 15.05.2016
 *      Author: niklausd
 */

#include <Timer.h>
#include <Arduino.h>

#include "PolarPulse.h"
#include <DbgTracePort.h>
#include <DbgTraceLevel.h>
#include <DbgCliTopic.h>
#include <DbgCliCommandPulseSim.h>
#include <DbgCliCommandPulseGen.h>

const bool PolarPulse::IS_POS_LOGIC = false;
const bool PolarPulse::IS_NEG_LOGIC = true;
const int  PolarPulse::PLS_NC       = -1;
const int  PolarPulse::IND_NC       = -1;

const unsigned int PolarPulse::s_defaultPulsePollTimeMillis          = 5;
const unsigned int PolarPulse::s_defaultExternalPulsePollTimeMillis  = 2500;
const unsigned int PolarPulse::s_defaultReportIntervalMillis         = 5000;
const unsigned int PolarPulse::s_defaultReportOveralBufferTimeMillis = 20000;
const unsigned int PolarPulse::s_oneMinuteMillis                     = 60000;

//-----------------------------------------------------------------------------

class PollingTimerAdapter : public TimerAdapter
{
private:
  PolarPulse* m_pulseSensor;
  bool m_lastWasPulseActive;
public:
  PollingTimerAdapter(PolarPulse* pulseSensor)
  : m_pulseSensor(pulseSensor)
  , m_lastWasPulseActive(false)
  { }

  void timeExpired()
  {
    if (0 != m_pulseSensor)
    {
      if (m_pulseSensor->isPulseDetectedExternally())
      {
        unsigned int count = m_pulseSensor->adapter()->getCount();
        if (0 != count)
        {
          m_pulseSensor->countPulse(count);
        }
      }
      else
      {
        bool currentIsPulseActive = m_pulseSensor->isPulseActive();

        if (m_lastWasPulseActive != currentIsPulseActive)
        {
          m_lastWasPulseActive = currentIsPulseActive;
          m_pulseSensor->setIndicator(currentIsPulseActive);
          if (currentIsPulseActive /*&& (0 !m_pulseSensor->dbgPulseGenCmd()) && !m_pulseSensor->dbgPulseGenCmd()->isRunning()*/)
          {

            Serial.println("PolarPulse count!");
            m_pulseSensor->countPulse();
          }
        }
      }
    }
  }
};

//-----------------------------------------------------------------------------

class ReportTimerAdapter : public TimerAdapter
{
private:
  PolarPulse* m_pulseSensor;
public:
  ReportTimerAdapter(PolarPulse* pulseSensor)
  : m_pulseSensor(pulseSensor)
  { }

  void timeExpired()
  {
    if (0 != m_pulseSensor)
    {
      m_pulseSensor->reportInterval();
    }
  }
};

//-----------------------------------------------------------------------------

PolarPulse::PolarPulse(int pulsePin, int indicatorPin, bool isPulsePinNegativeLogic, PolarPulseAdapter* adapter)
: m_pollingTimer(0)
, m_reportTimer(new Timer(new ReportTimerAdapter(this), Timer::IS_RECURRING, s_defaultReportIntervalMillis))
, m_adapter(adapter)
, m_trPort(new DbgTrace_Port("plrpls", DbgTrace_Level::info))
, m_dbgTopic(new DbgCli_Topic(DbgCli_Node::RootNode(), "pulse", "Pulse sensor component"))
, m_dbgPulseSimCmd(new DbgCli_Command_PulseSim(this))
, m_dbgPulseGenCmd(new DbgCli_Command_PulseGen(this))
, m_isPulsePinNegativeLogic(isPulsePinNegativeLogic)
, m_count(false)
, m_heartBeatRate(0)
, m_pulsePin(pulsePin)
, m_indicatorPin(indicatorPin)
, m_cMaxReportStores(s_defaultReportOveralBufferTimeMillis / s_defaultReportIntervalMillis)
, m_reportPeriodCount(0)
{
  if (isPulseDetectedExternally())
  {
    // pulse events detected externally
    TR_PRINT_STR(m_trPort, DbgTrace_Level::debug, "PolarPulse: detected externally (outside of this component)\n");
    m_pollingTimer = new Timer(new PollingTimerAdapter(this), Timer::IS_RECURRING, s_defaultExternalPulsePollTimeMillis);
  }
  else
  {
    // polling mode
    Serial.println("PolarPulse: detected internally");
    pinMode(m_pulsePin, INPUT);
    digitalWrite(m_pulsePin, m_isPulsePinNegativeLogic ? HIGH : LOW); // pull
    m_pollingTimer = new Timer(new PollingTimerAdapter(this), Timer::IS_RECURRING, s_defaultPulsePollTimeMillis);
  }

  if (IND_NC < m_indicatorPin)
  {
    pinMode(m_indicatorPin, OUTPUT);
    digitalWrite(m_indicatorPin, m_count);
  }
  m_heartBeatRate = new unsigned int[m_cMaxReportStores];
  for (unsigned char i = 0; i < m_cMaxReportStores; i++)
  {
    m_heartBeatRate[i] = 0;
  }
}

PolarPulse::~PolarPulse()
{
  if (IND_NC < m_indicatorPin)
  {
    pinMode(m_indicatorPin, INPUT);
  }

  delete [] m_heartBeatRate;
  m_heartBeatRate = 0;

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

DbgCli_Topic* PolarPulse::dbgTopic()
{
  return m_dbgTopic;
}

DbgCli_Command_PulseGen* PolarPulse::dbgPulseGenCmd()
{
  return m_dbgPulseGenCmd;
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

bool PolarPulse::isPulseDetectedExternally()
{
  bool isExternally = (PLS_NC >= m_pulsePin);
  return isExternally;
}

void PolarPulse::countPulse(unsigned int count)
{
  if (0 == count)
  {
    m_count++;
  }
  else
  {
    m_count += count;
  }
}

void PolarPulse::reportInterval()
{
  m_reportPeriodCount++;
//  Serial.print("reportInterval(), after incr, m_reportPeriodCount=");
//  Serial.println(m_reportPeriodCount);
  if (0 == (m_reportPeriodCount % m_cMaxReportStores))
  {
    m_reportPeriodCount = 0;
//    Serial.print("reportInterval(), after reset, m_reportPeriodCount=");
//    Serial.println(m_reportPeriodCount);
  }
  const unsigned int c_extrapolationFactor = s_oneMinuteMillis / s_defaultReportIntervalMillis;
  m_heartBeatRate[m_reportPeriodCount] = m_count * c_extrapolationFactor;
  m_count = 0;
  if (0 != m_adapter)
  {
    m_adapter->notifyHeartBeatRate(m_heartBeatRate[m_reportPeriodCount]);
    if (0 == m_reportPeriodCount)
    {
      m_adapter->notifyHeartBeatRate(m_heartBeatRate, m_cMaxReportStores);
    }
  }
}

void PolarPulse::setIndicator(bool isActive)
{
  if (0 <= m_indicatorPin)
  {
    digitalWrite(m_indicatorPin, !isActive);
  }
}
