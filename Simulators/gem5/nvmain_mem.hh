/*
 * Copyright (c) 2012-2013 Pennsylvania State University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  This file is part of NVMain- A cycle accurate timing, bit-accurate
 *  energy simulator for non-volatile memory. Originally developed by
 *  Matt Poremba at the Pennsylvania State University.
 *
 *  Website: http://www.cse.psu.edu/~poremba/nvmain/
 *  Email: mrp5060@psu.edu
 *
 *  ---------------------------------------------------------------------
 *
 *  If you use this software for publishable research, please include
 *  the original NVMain paper in the citation list and mention the use
 *  of NVMain.
 *
 */

#ifndef __MEM_NVMAIN_MEM_HH__
#define __MEM_NVMAIN_MEM_HH__


#include <fstream>
#include <deque>
#include <map>
#include <vector>
#include <ostream>

#include "NVM/nvmain.h"
#include "include/NVMTypes.h"
#include "include/NVMainRequest.h"
#include "mem/abstract_mem.hh"
#include "mem/tport.hh"
#include "params/NVMainMemory.hh"
#include "sim/eventq.hh"
#include "sim/serialize.hh"
#include "src/Config.h"
#include "src/EventQueue.h"
#include "src/NVMObject.h"
#include "src/SimInterface.h"
#include "src/TagGenerator.h"

namespace gem5
{

namespace memory
{

class NVMainMemory : public AbstractMemory, public NVM::NVMObject
{
  private:

    class MemoryPort : public ResponsePort
    {
        friend class NVMainMemory;

        NVMainMemory& memory;
        NVMainMemory& forgdb;

      public:

        MemoryPort(const std::string& _name, NVMainMemory& _memory);

      protected:

        Tick recvAtomic(PacketPtr pkt);

        Tick recvAtomicBackdoor(PacketPtr pkt, MemBackdoorPtr &backdoor);

        void recvFunctional(PacketPtr pkt);

        void recvMemBackdoorReq(const MemBackdoorReq &req,
                                MemBackdoorPtr &backdoor);

        bool recvTimingReq(PacketPtr pkt);

        void recvRespRetry( );

        AddrRangeList getAddrRanges() const override;

    };

    void tick();
    void SendResponses( );
    MemberEventWrapper<&NVMainMemory::tick> clockEvent;
    MemberEventWrapper<&NVMainMemory::SendResponses> respondEvent;

    void CheckDrainState( );
    void ScheduleResponse( );
    void ScheduleClockEvent( Tick );
    void SetRequestData(NVM::NVMainRequest *request, PacketPtr pkt);

    class NVMainStatPrinter
    {
        friend class NVMainMemory;

      public:
        NVMainMemory *memory;
        NVMainMemory *forgdb;

        void process();

        NVM::NVMain *nvmainPtr;
        std::ofstream statStream;
    };

    class NVMainStatReseter
    {
      public:
        void process();

        NVM::NVMain *nvmainPtr;
    };

    struct NVMainMemoryRequest
    {
        PacketPtr packet;
        NVM::NVMainRequest *request;
        Tick issueTick;
        bool atomic;
    };

    NVM::NVMain *m_nvmainPtr;
    NVM::Stats *m_statsPtr;
    NVM::EventQueue *m_nvmainEventQueue;
    NVM::GlobalEventQueue *m_nvmainGlobalEventQueue;
    NVM::Config *m_nvmainConfig;
    NVM::SimInterface *m_nvmainSimInterface;
    NVM::TagGenerator *m_tagGenerator;
    std::string m_nvmainConfigPath;

    bool m_nacked_requests;
    float m_avgAtomicLatency;
    uint64_t m_numAtomicAccesses;
    NVM::ncycle_t nextEventCycle;

    Tick clock;
    Tick lat;
    Tick lat_var;
    bool nvmain_atomic;

    uint64_t BusWidth;
    uint64_t tBURST;
    uint64_t RATE;

    bool NVMainWarmUp;

    NVMainStatPrinter statPrinter;
    NVMainStatReseter statReseter;
    Tick lastWakeup;

    uint64_t m_requests_outstanding;

  public:

    typedef NVMainMemoryParams Params;
    NVMainMemory(const Params &p);
    virtual ~NVMainMemory();

    Port& getPort(const std::string& if_name,
                  PortID idx = InvalidPortID) override;
    void init() override;
    void startup() override;
    void wakeup();

    bool RequestComplete( NVM::NVMainRequest *req );

    void Cycle(NVM::ncycle_t) { }

    DrainState drain() override;

    void serialize(CheckpointOut &cp) const override;
    void unserialize(CheckpointIn &cp) override;

    MemoryPort port;
    static NVMainMemory *masterInstance;
    NVMainMemory *otherInstance;
    std::vector<NVMainMemory *> allInstances;
    bool retryRead, retryWrite, retryResp;
    std::deque<PacketPtr> responseQueue;
    std::vector<PacketPtr> pendingDelete;
    std::map<NVM::NVMainRequest *, NVMainMemoryRequest *> m_request_map;

  protected:

    Tick doAtomicAccess(PacketPtr pkt);
    Tick recvAtomicBackdoor(PacketPtr pkt, MemBackdoorPtr &backdoor);
    void doFunctionalAccess(PacketPtr pkt);
    void recvMemBackdoorReq(const MemBackdoorReq &req,
                            MemBackdoorPtr &backdoor);
    void recvRetry();

};

} // namespace memory
} // namespace gem5

#endif

