#include "ccn-packets.h"

#include <cstdlib>
#include <ostream>
#include <map>

using std::ostringstream;

namespace ns3 {

uint8_t CCN_Packets::INTEREST = 0;
uint8_t CCN_Packets::DATA = 1;



CCN_Interest::CCN_Interest(Ptr<CCN_Name> nameIn) {
	name = nameIn;

}

CCN_Interest::~CCN_Interest() {

	name = 0;
}

void CCN_Interest::DoDispose(void) {

	name = 0;
}

Ptr<Packet> CCN_Interest::serializeToPacket() const {
	uint8_t bufflen = sizeof(uint8_t) + name->serializedSize();
	uint8_t *packetBuff = (uint8_t*) malloc(bufflen * sizeof(uint8_t));

	memcpy(packetBuff, &CCN_Packets::INTEREST, sizeof(uint8_t));
	uint32_t offset = sizeof(uint8_t);

	name->serializeToBuffer((uint8_t*) (packetBuff + offset));

	Ptr<Packet> p = Create<Packet>(packetBuff, bufflen);
	free(packetBuff);

	return p;
}

Ptr<CCN_Interest> CCN_Interest::deserializeFromPacket(
		const Ptr<Packet> packet) {
	uint32_t offset = sizeof(uint8_t);

	uint32_t packetSize = packet->GetSize();
	uint8_t *buff = (uint8_t*) malloc(packetSize * sizeof(uint8_t));
	packet->CopyData(buff, packetSize);

	pair<Ptr<CCN_Name>, uint32_t> name = CCN_Name::deserializeFromBuffer(
			buff + offset);

	free(buff);

	Ptr<CCN_Interest> interest = CreateObject<CCN_Interest>(name.first);
	return interest;
}

bool operator==(const Ptr<CCN_Interest>& lhs, const Ptr<CCN_Interest>& rhs) {
	return lhs->getName() == rhs->getName();
}

CCN_Data::CCN_Data( Ptr<CCN_Name> nameIn,uint8_t* buffer, uint32_t buffsize)
{

	name = nameIn;
	dataLength = buffsize;
	data = (uint8_t*) malloc(buffsize * sizeof(uint8_t));
	memcpy(data, buffer, buffsize * sizeof(uint8_t));
}

CCN_Data::~CCN_Data() {

	name = 0;
	if (data) {
		free(data);
		data = 0;
	}
}

void CCN_Data::DoDispose(void) {

	name = 0;
	if (data) {
		free(data);
		data = 0;
	}
}

Ptr<Packet> CCN_Data::serializeToPacket() const {
	uint32_t packetBuffLen = sizeof(CCN_Packets::DATA)
			 + name->serializedSize() + sizeof(dataLength)
			+ dataLength;

	//std::cout<< "packetBuffLen " << (int)packetBuffLen << std::endl;
	uint8_t *packetBuff = (uint8_t*) malloc(packetBuffLen);

	//CCN_Packets::DATA
	memcpy(packetBuff, &CCN_Packets::DATA, sizeof(uint8_t));
	uint32_t offset = sizeof(uint8_t);
//	std::cout<< "offset " << (int)offset << std::endl;



//	std::cout<< "offset " << (int)offset << std::endl;


//	std::cout<< "offset " << (int)offset << std::endl;

	//name
	offset += name->serializeToBuffer((uint8_t*) (packetBuff + offset));
//	std::cout<< "offset " << (int)offset << std::endl;

	//data
	memcpy((void*) (packetBuff + offset), &dataLength, sizeof(dataLength));
	offset += sizeof(dataLength);
//	std::cout<< "offset " << (int)offset << std::endl;

	memcpy((void*) (packetBuff + offset), data, dataLength);

	Ptr<Packet> p = Create<Packet>(packetBuff, packetBuffLen);
	free(packetBuff);
	return p;
}

Ptr<CCN_Data> CCN_Data::deserializeFromPacket(const Ptr<Packet> packet) {
	uint8_t* buff = (uint8_t*) malloc(packet->GetSize() * sizeof(uint8_t));
	packet->CopyData(buff, packet->GetSize());

	uint32_t offset = sizeof(uint8_t); //ignore CCN_Packets::DATA
	//std::cout<< "d offset " << (int)offset << std::endl;


	//std::cout<< "d offset " << (int)offset << std::endl;



	//std::cout<< "d offset " << (int)offset << std::endl;

	pair<Ptr<CCN_Name>, uint32_t> ccnNamePair = CCN_Name::deserializeFromBuffer(
			buff + offset);
	//std::cout << "read name " << ccnNamePair.first->toString() << std::endl;
	offset += ccnNamePair.second;
	//std::cout<< "d offset " << (int)offset << std::endl;

	uint32_t datalen = 0;
	memcpy(&datalen, (void*) (buff + offset), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	//std::cout<< "d offset " << (int)offset << std::endl;

	Ptr<CCN_Data> data = CreateObject<CCN_Data>(
			ccnNamePair.first, (uint8_t*) (buff + offset), datalen);
	free(buff);
	return data;
}

bool buffEquals(uint8_t *rhs, uint8_t* lhs, uint32_t length) {
	for (uint32_t i = 0; i < length; i++) {
		if (rhs[i] != lhs[i]) {
			return false;
		}
	}
	return true;
}

bool operator==(const Ptr<CCN_Data>& lhs, const Ptr<CCN_Data>& rhs) {
	return lhs->getName() == rhs->getName()

			&& lhs->getLength() == rhs->getLength()
			&& buffEquals(lhs->getData(), rhs->getData(), lhs->getLength());
}

}  // namespace ns3
