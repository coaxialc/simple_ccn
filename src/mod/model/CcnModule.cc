#include "CcnModule.h"
#include "experiment_globals.h"

#include <iostream>
#include <sstream>
#include <string>

using std::cout;
using std::endl;
using std::string;
using std::stringstream;

namespace ns3 {

uint32_t CcnModule::RX_INTERESTS = 0;
uint32_t CcnModule::RX_DATA = 0;
int CcnModule::COUNT = 0;

Time CcnModule::ONE_NS = NanoSeconds(1);

CcnModule::CcnModule(Ptr<Node> node) {
	COUNT++;
	//std::cout<< "Installing CCN module on ns3::node " << node->GetId() << std::endl;
	nodePtr = node;	

	thePIT = CreateObject<PIT>();
	FIB = CreateObject<Trie>();
	p_RX_Data = 0;
	addresses = map<Ptr<NetDevice>, Address>();

	for (uint32_t i = 0; i < nodePtr->GetNDevices(); i++) {

		Ptr<NetDevice> device = nodePtr->GetDevice(i);
		device->SetReceiveCallback(
				MakeCallback(&CcnModule::handlePacket, this));

		Ptr<Channel> channel = device->GetChannel();
		Address adr;
		if (device == channel->GetDevice(0)) {
			adr = channel->GetDevice(1)->GetAddress();
		} else {
			adr = channel->GetDevice(0)->GetAddress();
		}
		addresses[device] = adr;
	}


}

CcnModule::~CcnModule() {
	thePIT = 0;
	FIB = 0;
	nodePtr = 0;

	addresses.clear();
}

void CcnModule::reset()
{
	neighborModules.clear();

	addresses.clear();
}

void CcnModule::DoDispose(void) {
	thePIT = 0;
	FIB = 0;
	nodePtr = 0;

	addresses.clear();
	COUNT--;
	//std::cout<<"COUNT: "<<COUNT<<std::endl;
}

void CcnModule::reInit() {
	thePIT = 0;
	thePIT = CreateObject<PIT>();
	p_RX_Data = 0;

	FIB = 0;
	FIB = CreateObject<Trie>();
}

void CcnModule::sendInterest(Ptr<CCN_Name> name, Ptr<LocalApp> localApp){
	Simulator::Schedule(CcnModule::ONE_NS, &CcnModule::doSendInterest, this, name, localApp);
}

void CcnModule::sendData(Ptr<CCN_Name> name, uint8_t *buff, uint32_t bufflen){
	uint8_t *newBuff = (uint8_t*)malloc(bufflen);
	memcpy(newBuff, buff, bufflen);
	Simulator::Schedule(CcnModule::ONE_NS, &CcnModule::doSendData, this, name, newBuff, bufflen);
}


void CcnModule::sendThroughDevice(Ptr<const Packet> p, Ptr<NetDevice> nd) {
	uint8_t* b = new uint8_t[p->GetSize()];
	p->CopyData(b, p->GetSize());
	Ptr<Packet> p2 = Create<Packet>(b, p->GetSize());
	delete [] b;
	
	bool sent = nd->Send(p2, addresses[nd], 0x88DD);

	if (!sent) {
		std::cout << "bytes dropped" << std::endl;
		std::cout << "packets dropped" << std::endl;
	}	
}

bool CcnModule::handlePacket(Ptr<NetDevice> nd, Ptr<const Packet> p, uint16_t a,
		const Address& ad) {
	uint8_t type = extract_packet_type(p);
	if (type == CCN_Packets::INTEREST) {
		RX_INTERESTS++;
		handleIncomingInterest(p, nd);
	} else if (type == CCN_Packets::DATA) {
		RX_DATA++;
		p_RX_Data++;
		handleIncomingData(p, nd);
	}

	return true;
}

uint8_t CcnModule::extract_packet_type(Ptr<const Packet> p) {
	uint8_t b2 = 0;
	p->CopyData(&b2, sizeof(uint8_t));
	return b2;
}

void CcnModule::handleIncomingInterest(Ptr<const Packet> p, Ptr<NetDevice> nd) {

	//say it when you get an interest
	//std::cout<<"Node "<<this->nodePtr->GetId()<<" just got the interest."<<std::endl;

	Ptr<CCN_Interest> interest = CCN_Interest::deserializeFromPacket(p->Copy());
	
	Ptr<PTuple> pt = this->thePIT->check(interest->getName());
	
	if (pt != 0)
	{
		pt->addDevice(nd);
		return;
	}	

	//PIT tuple not found
	Ptr<TrieNode> tn = this->FIB->longestPrefixMatch(interest->getName());	
	if(tn->hasLocalApps())
	{
		Ptr<LocalApp> publisher = tn->getLocalApps().at(0);

		//save to PIT first, next hop is publisher
		Ptr<PTuple> tuple=CreateObject<PTuple>();
		tuple->addDevice(nd);
		this->thePIT->update(interest->getName(),tuple);

		publisher->deliverInterest(interest->getName());
		return;
	}

	NS_ASSERT_MSG(tn->hasDevices(),"router " + nodePtr->GetId() << "does not know how to forward " << interest->getName()->toString());
			
	//interest will go to the first netdevice
	Ptr<NetDevice> outport = tn->getDevices().at(0);

	//must store in PIT
	Ptr<PTuple> ptuple = CreateObject<PTuple>();
	ptuple->addDevice(nd);
	thePIT->update(interest->getName(), ptuple);

	Ptr<Packet> packet = interest->serializeToPacket();
	sendThroughDevice(packet, outport);

}

void CcnModule::handleIncomingData(Ptr<const Packet> p, Ptr<NetDevice> nd)
{
	Ptr<CCN_Data> data = CCN_Data::deserializeFromPacket(p->Copy());

	//always check PIT
	Ptr<PTuple> pt = this->thePIT->check(data->getName());

	if (pt != 0)
	{
		thePIT->erase(data->getName());

		//give data to any local app
		vector<Ptr<LocalApp> >& localApps = pt->getLocalApps();
		for (uint32_t i = 0; i < localApps.size(); i++)
		{
			localApps[i]->deliverData(data->getName(), data->getData(),	data->getLength());
		}

		//give data to any device
		vector<Ptr<NetDevice> >& devices = pt->getDevices();
		for (uint32_t i = 0; i < devices.size(); i++)
		{
			Ptr<const Packet> dataP=data->serializeToPacket();

			sendThroughDevice(dataP,devices[i]);
		}
	}
	else
	{
		std::cout<<"Does not know how to forward data."<<std::endl;
	}
}



void CcnModule::doSendInterest(Ptr<CCN_Name> name, Ptr<LocalApp> localApp) {
	Ptr<PTuple> pt = thePIT->check(name);
	if (pt != 0) {
		bool added = pt->addLocalApp(localApp);
		if (!added) {
			cout << "local app has already requested " << name->toString()
					<< endl;
		}
		return;
	}

	Ptr<CCN_Interest> interest = CreateObject<CCN_Interest>(name);
	Ptr<TrieNode> fibLookup = this->FIB->longestPrefixMatch(name);
	if (fibLookup == 0) {
		stringstream sstr;
		sstr << "router " << nodePtr->GetId() << "cannot forward Interests for "
				<< name->toString();
		string mesg = sstr.str();
		NS_ASSERT_MSG(fibLookup != 0, mesg);
	}

	pt = CreateObject<PTuple>();
	pt->addLocalApp(localApp);
	
	thePIT->update(name, pt);

	if (fibLookup->hasLocalApps()) {
		Ptr<LocalApp> publisher = fibLookup->getLocalApps().at(0);
		publisher->deliverInterest(name);
	} else if (fibLookup->hasDevices()) {
		Ptr<NetDevice> netdevice = fibLookup->getDevices().at(0);
		Ptr<Packet> p = interest->serializeToPacket();
		sendThroughDevice(p, netdevice);
	} else {
		stringstream sstr;
		sstr << "router " << nodePtr->GetId() << "cannot forward Interests for "
				<< name->toString();
		string mesg = sstr.str();
		NS_ASSERT_MSG(fibLookup != 0, mesg);
		NS_ASSERT_MSG(false, mesg);
	}
}



Ptr<Trie> CcnModule::getFIB() {
	return FIB;
}

Ptr<Node> CcnModule::getNode() {
	return nodePtr;
}

map<Ptr<NetDevice>, Ptr<CcnModule> > CcnModule::getNeighborModules() {
	return neighborModules;
}

Ptr<PIT> CcnModule::getPIT() {
	return thePIT;
}

void CcnModule::doSendData(Ptr<CCN_Name> name, uint8_t* buff, uint32_t bufflen) {

	Ptr<PTuple> ptuple = thePIT->check(name);

	if (ptuple == 0) {
		stringstream sstr;
		sstr << "router " << nodePtr->GetId() << "has no pit record for "
				<< name->toString();
		string mesg = sstr.str();
		NS_ASSERT_MSG(ptuple != 0, mesg);
	}

	uint32_t deliverd = 0;
	vector<Ptr<LocalApp> >::iterator iter;
	for (iter = ptuple->getLocalApps().begin();
			iter != ptuple->getLocalApps().end(); iter++) {
		deliverd++;
		(*iter)->deliverData(name, buff, bufflen);
	}

	uint32_t fwded=0;
	vector<Ptr<NetDevice> >::iterator iter2;
	for (iter2 = ptuple->getDevices().begin();iter2 != ptuple->getDevices().end(); iter2++)
	{
		Ptr<CCN_Data> data = CreateObject<CCN_Data>(name, buff, bufflen);
		Ptr<const Packet> dataP=data->serializeToPacket();

		fwded++;
		sendThroughDevice(dataP,*iter2);
	}

	NS_ASSERT_MSG(fwded > 0 || deliverd > 0, "data went nowhere");

	free(buff);
}

bool operator<(const Ptr<NetDevice>& lhs, const Ptr<NetDevice>& rhs) {
	if(lhs==0)
	{
		std::cout<<"first was null"<<std::endl;
	}

	if(rhs==0)
	{
		std::cout<<"second was null"<<std::endl;
	}
	return lhs->GetAddress() < rhs->GetAddress();
}

uint32_t CcnModule::getNodeId()
{
	return nodePtr->GetId();
}

uint32_t CcnModule::getTXData()
{
	return p_RX_Data;
}

}
