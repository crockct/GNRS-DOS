/*
 *  Copyright (c) 2010-2013, Rutgers, The State University of New Jersey
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of the organization(s) stated above nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MF_FLOWCTRL_H
#define MF_FLOWCTRL_H

#include <limits>

class MF_ReliabTransport;
class MF_EventQueue;
class TransHeader;

class MF_FlowController {
public:
  MF_FlowController();
  virtual ~MF_FlowController();

  virtual void sendData(MF_ReliabTransport *rtransp, MF_EventQueue *eq, u_int dstGUID) = 0;
  virtual void handleInDataFlag(MF_ReliabTransport *rtransp, TransHeader *thdr, 
        MF_EventQueue *eq, u_int dstGUID) = 0; // see if any flag is present asking for recv buf updates
  virtual void handleFlowCtrlNotif(MF_ReliabTransport *rtransp, TransHeader *thdr,
        MF_EventQueue *eq, u_int dstGUID) = 0;

};

class MF_HighestRateController : public MF_FlowController {
public:
  virtual void sendData(MF_ReliabTransport *rtransp, MF_EventQueue *eq, u_int dstGUID);
  virtual void handleInDataFlag(MF_ReliabTransport *rtransp, TransHeader *thdr, 
        MF_EventQueue *eq, u_int dstGUID);
  virtual void handleFlowCtrlNotif(MF_ReliabTransport *rtransp, TransHeader *thdr,
        MF_EventQueue *eq, u_int dstGUID);
};

class MF_WindowController : public MF_FlowController {
public:
  virtual void sendData(MF_ReliabTransport *rtransp, MF_EventQueue *eq, u_int dstGUID);
  virtual void handleInDataFlag(MF_ReliabTransport *rtransp, TransHeader *thdr, 
        MF_EventQueue *eq, u_int dstGUID); 
  virtual void handleFlowCtrlNotif(MF_ReliabTransport *rtransp, TransHeader *thdr,
        MF_EventQueue *eq, u_int dstGUID);
};

#endif
