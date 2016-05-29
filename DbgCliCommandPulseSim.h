/*
 * DbgCliCommandPulseSim.h
 *
 *  Created on: 24.05.2016
 *      Author: niklausd
 */

#ifndef LIB_POLARPULSE_DBGCLICOMMANDPULSESIM_H_
#define LIB_POLARPULSE_DBGCLICOMMANDPULSESIM_H_

#include <DbgCliCommand.h>

class PolarPulse;
class DbgTrace_Port;

class DbgCli_Command_PulseSim : public DbgCli_Command
{
public:
  DbgCli_Command_PulseSim(PolarPulse* polarPulse);
  virtual ~DbgCli_Command_PulseSim();
  void execute(unsigned int argc, const char** args, unsigned int idxToFirstArgToHandle);
private:
  PolarPulse* m_polarPulse;
  DbgTrace_Port* m_trPort;
private:  // forbidden functions
  DbgCli_Command_PulseSim();                                                // default constructor
  DbgCli_Command_PulseSim(const DbgCli_Command_PulseSim& src) ;             // copy constructor
  DbgCli_Command_PulseSim& operator = (const DbgCli_Command_PulseSim& src); // assignment operator
};

#endif /* LIB_POLARPULSE_DBGCLICOMMANDPULSESIM_H_ */
