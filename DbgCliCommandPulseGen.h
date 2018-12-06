/*
 * DbgCliCommandPulseGen.h
 *
 *  Created on: 26.05.2016
 *      Author: niklausd
 */

#ifndef LIB_POLARPULSE_DBGCLICOMMANDPULSEGEN_H_
#define LIB_POLARPULSE_DBGCLICOMMANDPULSEGEN_H_

#include <DbgCliCommand.h>

class PolarPulse;
class DbgTrace_Port;
class Timer;

class DbgCli_Command_PulseGen: public DbgCli_Command
{
public:
  DbgCli_Command_PulseGen(PolarPulse* polarPulse);
  virtual ~DbgCli_Command_PulseGen();
  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle);
  void timeExpired();
  bool hasToBeRunning();
private:
  void incrementTime();
  void decrementTime();
  void startTimer();
private:
  PolarPulse* m_polarPulse;
  DbgTrace_Port* m_trPort;
  Timer* m_randomIntervalTimer;
  bool m_hasToBeRunning;
  bool m_isIntervalIncreasing;
  unsigned int m_currentTimeMillis;
  unsigned int m_newTimeMillis;
private:  // forbidden functions
  DbgCli_Command_PulseGen();                                                // default constructor
  DbgCli_Command_PulseGen(const DbgCli_Command_PulseGen& src) ;             // copy constructor
  DbgCli_Command_PulseGen& operator = (const DbgCli_Command_PulseGen& src); // assignment operator
};

#endif /* LIB_POLARPULSE_DBGCLICOMMANDPULSEGEN_H_ */
