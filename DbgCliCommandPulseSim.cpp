/*
 * DbgCliCommandPulseSim.cpp
 *
 *  Created on: 24.05.2016
 *      Author: niklausd
 */

#include <DbgTracePort.h>
#include <DbgCliCommandPulseSim.h>
#include <DbgCliNode.h>
#include <DbgCliTopic.h>
#include <PolarPulse.h>
#include <Timer.h>

DbgCli_Command_PulseSim::DbgCli_Command_PulseSim(PolarPulse* polarPulse)
: DbgCli_Command(polarPulse->dbgTopic(), "sim", "Simulate one Heart Beat pulse.")
, m_polarPulse(polarPulse)
, m_trPort(new DbgTrace_Port("psim", DbgTrace_Level::info))
{ }

DbgCli_Command_PulseSim::~DbgCli_Command_PulseSim()
{ }

void DbgCli_Command_PulseSim::execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle)
{
  if (0 != m_polarPulse)
  {
    m_polarPulse->setIndicator(true);
    m_polarPulse->countPulse();
//    Serial.println("Heart beat simulated.");
    TR_PRINT_STR(m_trPort, DbgTrace_Level::info, "Heart beat simulated.");
    delayAndSchedule(5);
    m_polarPulse->setIndicator(false);
  }
}
