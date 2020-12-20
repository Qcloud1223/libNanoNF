// (Some kind of) NAT, which cannot become an NAT become response packet
// will not adjust destination according to this NF in a simple pcap-based
// demostration.

#include <unordered_map>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <cstdio>

using namespace std;

// https://en.cppreference.com/w/cpp/memory/new/operator_new
// replacement of a minimal set of functions:
void *operator new(size_t sz)
{
    // std::printf("global op new called, size = %zu\n", sz);
    void *ptr = malloc(sz);
    if (ptr)
        return ptr;
    else
        throw bad_alloc{};
}

void operator delete(void *ptr) noexcept
{
    // std::puts("global op delete called");
    free(ptr);
}

struct Tuple4
{
    uint32_t srcIP, dstIP;
    uint16_t srcPort, dstPort;
};

bool operator==(const Tuple4 &t1, const Tuple4 &t2)
{
    return t1.srcIP == t2.srcIP && t1.dstIP == t2.dstIP &&
           t1.srcPort == t2.srcPort && t1.dstPort == t2.dstPort;
}

struct HashTuple4
{
    size_t operator()(Tuple4 const &t) const noexcept
    {
        // printf("[hash] in operator()\n");
        size_t h1 = hash<uint32_t>{}(t.srcIP), h2 = hash<uint32_t>{}(t.dstIP),
               h3 = hash<uint16_t>{}(t.srcPort), h4 = hash<uint16_t>{}(t.dstPort);
        size_t r1 = h1 ^ (h2 << 1), r2 = r1 ^ (h3 << 1), r3 = r2 ^ (h4 << 1);
        // printf("[hash] out operator(): %lu\n", r3);
        return r3;
    }
};

unordered_map<Tuple4, uint16_t, HashTuple4> table;
uint16_t nextPort = 0;

extern "C" void InitializeFunction()
{
    //
}

struct __attribute__((__packed__)) Headers
{
    uint8_t eth[14];
    uint8_t ipFields[12];
    uint32_t srcIP;
    uint32_t dstIP;
    uint16_t srcPort;
    uint16_t dstPort;
    uint8_t tcpFields[9];
    // FIN should be the last bit in the protocol spec, so it should
    // to declared first in C to my exprience
    uint8_t fin : 1;
    uint8_t tcpFlags : 7;
};

extern "C" void ProcessPacket(uint8_t *packet, size_t packetLength)
{
    // printf("[hashfunc] start\n");
    Headers *headers = (Headers *)packet;
    Tuple4 tuple = {
        .srcIP = headers->srcIP,
        .dstIP = headers->dstIP,
        .srcPort = headers->srcPort,
        .dstPort = headers->dstPort};
    uint16_t mappedPort;
    // printf("[hashfunc] before select\n");
    try
    {
        // printf("[hashfunc] before call `at`\n");
        mappedPort = table.at(tuple);
        // printf("[hashfunc] after call `at`\n");
        if (headers->fin)
        {
            table.erase(tuple);
        }
    }
    catch (out_of_range)
    {
        // printf("[hashfunc] in catch\n");
        mappedPort = nextPort;
        if (!headers->fin)
        {
            table.insert({tuple, mappedPort});
        }
        nextPort += 1;
        if (nextPort == 65536)
        {
            nextPort = 0;
        }
    }
    headers->srcIP = (((((192u << 8u) | 168u) << 8u) | 1u) << 8u) | 1u;
    headers->srcPort = mappedPort;
}