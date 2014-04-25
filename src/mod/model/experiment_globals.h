/*
 * random_variable.h
 *
 *  Created on: Dec 24, 2013
 *      Author: tsilochr
 */

#ifndef EXPERIMENT_GLOBALS_H
#define EXPERIMENT_GLOBALS_H

#include "ns3/core-module.h"

namespace ns3 {

class ExperimentGlobals{
public:
	static Ptr<UniformRandomVariable> RANDOM_VAR;

};

}  // namespace ns3
#endif /* EXPERIMENT_GLOBALS_H */
