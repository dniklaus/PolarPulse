/*
 * DbgCliCommandPulseGen.cpp
 *
 *  Created on: 26.05.2016
 *      Author: niklausd
 */

#include <DbgTracePort.h>
#include <DbgCliCommandPulseGen.h>
#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <PolarPulse.h>
#include <Timer.h>

//-----------------------------------------------------------------------------

class RandomIntervalTimerAdapter : public TimerAdapter
{
private:
  DbgCli_Command_PulseGen* m_gen;
public:
  RandomIntervalTimerAdapter(DbgCli_Command_PulseGen* gen)
  : m_gen(gen)
  { }

  void timeExpired()
  {
    if (0 != m_gen)
    {
      m_gen->timeExpired();
    }
  }
};

//-----------------------------------------------------------------------------

DbgCli_Command_PulseGen::DbgCli_Command_PulseGen(PolarPulse* polarPulse)
: DbgCli_Command(polarPulse->dbgTopic(), "gen", "Start/Stop the random heart beat generator.")
, m_polarPulse(polarPulse)
, m_trPort(new DbgTrace_Port("pgen", DbgTrace_Level::info))
, m_randomIntervalTimer(new Timer(new RandomIntervalTimerAdapter(this), Timer::IS_RECURRING))
, m_hasToBeRunning(false)
, m_isIntervalIncreasing(false)
, m_currentTimeMillis(1000)
, m_newTimeMillis(m_currentTimeMillis)
{ }

DbgCli_Command_PulseGen::~DbgCli_Command_PulseGen()
{
  delete m_randomIntervalTimer->adapter();
  m_randomIntervalTimer->attachAdapter(0);

  delete m_randomIntervalTimer;
  m_randomIntervalTimer = 0;
}

void DbgCli_Command_PulseGen::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  m_hasToBeRunning = !m_hasToBeRunning;
  if (hasToBeRunning())
  {
    m_randomIntervalTimer->startTimer(m_currentTimeMillis);
  }
  else
  {
    m_randomIntervalTimer->cancelTimer();
  }
  TR_PRINT_STR(m_trPort, DbgTrace_Level::info, m_hasToBeRunning ? "Heart beat generator is running." : "Heart beat generator is inactive.")
}

bool DbgCli_Command_PulseGen::hasToBeRunning()
{
  return m_hasToBeRunning;
}

void DbgCli_Command_PulseGen::incrementTime()
{
  if (m_newTimeMillis <= 2000) // min. pulse rate 30/min.
  {
    m_newTimeMillis++;
  }
  else
  {
    m_isIntervalIncreasing = false;
  }
}

void DbgCli_Command_PulseGen::decrementTime()
{
  if (m_newTimeMillis >= 300) // max. pulse rate 200/min.
  {
    m_newTimeMillis--;
  }
  else
  {
    m_isIntervalIncreasing = true;
  }
}

void DbgCli_Command_PulseGen::timeExpired()
{
  if (hasToBeRunning())
  {
    if (m_isIntervalIncreasing)
    {
      incrementTime();
    }
    else
    {
      decrementTime();
    }
    startTimer();
  }
}

void DbgCli_Command_PulseGen::startTimer()
{
  if (0 != m_randomIntervalTimer)
  {
    // (re-)start the timer with new interval value
    m_randomIntervalTimer->startTimer(m_newTimeMillis);
  }
  m_currentTimeMillis = m_newTimeMillis;
  Serial.print("New timer interval [ms]: ");
  Serial.println(m_currentTimeMillis);
}
