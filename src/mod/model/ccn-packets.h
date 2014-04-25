/*
 * ccn-packets.h
 *
 *  Created on: Dec 28, 2013
 *      Author: tsilochr
 */

#ifndef CCN_PACKETS_H_
#define CCN_PACKETS_H_

#include <string>
#include "ns3/core-module.h"
#include "ns3/packet.h"
#include "ns3/mod-module.h"

using std::string;

namespace ns3{



class CCN_Packets{
public:
	static uint8_t INTEREST;
	static uint8_t DATA;
};


class CCN_Interest : public Object
{
    public:
	CCN_Interest(Ptr<CCN_Name>);

	virtual ~CCN_Interest();
	virtual void DoDispose(void);

	Ptr<CCN_Name> getName()const {return name; }

	Ptr<Packet> serializeToPacket() const;
	static Ptr<CCN_Interest> deserializeFromPacket(const Ptr<Packet>);
	string stringForm(int hc);

	friend bool operator== (const Ptr<CCN_Interest>&, const Ptr<CCN_Interest>&);

    private:
	Ptr<CCN_Name> name;
};

class CCN_Data: public Object {
public:
	CCN_Data(  Ptr<CCN_Name>, uint8_t*, uint32_t);
	~CCN_Data();
	virtual void DoDispose(void);



	uint8_t* getData() { return data; }
	uint32_t getLength() const { return dataLength; }

	Ptr<CCN_Name> getName() const {return name; }

	uint32_t serializedSize() const;
	Ptr<Packet> serializeToPacket() const;
	static Ptr<CCN_Data> deserializeFromPacket(const Ptr<Packet>);

	friend bool operator== (const Ptr<CCN_Data>&, const Ptr<CCN_Data>&);

private:

	Ptr<CCN_Name> name;
	uint8_t * data;
	uint32_t dataLength;
};

}
#endif /* CCN_PACKETS_H_ */
