#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mod-module.h"
#include "ns3/BootstrappingHelper.h"


using namespace ns3;

int main(int argc ,char *argv[])
{
	CommandLine c;
	c.Parse (argc, argv);

	ExperimentGlobals::RANDOM_VAR =CreateObject<UniformRandomVariable>();

	//Topology path
	//--------------------------------------------------
//	string topology = "/home/coaxial/Desktop/topologies/geant.txt";
	string topology = *(new std::string(argv[1]));
	//--------------------------------------------------

	//Output path
	//--------------------------------------------------
//	string output="/home/coaxial/Desktop/exp_output/";
	string output = *(new std::string(argv[2]));
	//--------------------------------------------------

	//Seed
	//--------------------------------------------------
	RngSeedManager::SetSeed (std::atoi(argv[3]));
//	RngSeedManager::SetSeed (1);
	//--------------------------------------------------

	//Group size
	//--------------------------------------------------
//	uint8_t gs=3;
	uint8_t gs=std::atoi(argv[4]);
	//--------------------------------------------------

	//Interest number
	//--------------------------------------------------
//	uint8_t interestNum=100;
	uint8_t interestNum=std::atoi(argv[5]);
	//--------------------------------------------------

	Ptr<BootstrappingHelper> bh=CreateObject<BootstrappingHelper>(topology,output,gs,interestNum,std::atoi(argv[3]));

	bh->parseTopology();

	bh->startExperiment();

	Simulator::Destroy();
	return 0;
}
