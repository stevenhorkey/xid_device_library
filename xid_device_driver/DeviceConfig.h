/* Copyright (c) 2010, Cedrus Corporation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * Neither the name of Cedrus Corporation nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "constants.h"
#include "XidDriverImpExpDefs.h"

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/property_tree/ptree.hpp>

namespace Cedrus
{
    struct DevicePort
    {
        DevicePort():
            portName(""),
            portNumber(-1),
            numberOfLines(-1),
            keyMap(8, -1),
            isResponsePort(false)
        {
        }

        std::string portName;
        int portNumber;
        int numberOfLines;
        std::vector<int> keyMap;
        bool isResponsePort;

        bool operator==( const DevicePort& other ) const
        {
            return ( portName == other.portName &&
                     portNumber == other.portNumber &&
                     numberOfLines == other.numberOfLines &&
                     keyMap == other.keyMap );
        }
    };

    class CEDRUS_XIDDRIVER_IMPORTEXPORT DeviceConfig
    {
    private:
        DeviceConfig(boost::property_tree::ptree * pt);

    public:
        // This is exported for testing purposes only!
        static boost::shared_ptr<DeviceConfig> ConfigForDevice(boost::property_tree::ptree * pt);

        ~DeviceConfig(void);

        int GetMappedKey(int port, int key) const;

        std::string GetDeviceName() const;

        int GetProductID() const;

        int GetModelID() const;

        int GetMajorVersion() const;

        const std::vector<DevicePort> * GetVectorOfPorts() const;

        const Cedrus::DevicePort * GetPortPtrByIndex(unsigned int portNum) const;

        bool DoesConfigMatchDevice( int deviceID, int modelID, int majorFirmwareVer ) const;

        bool NeedsDelay() const;

        bool IsLumina() const
        {
            return m_ProductID == PRODUCT_ID_LUMINA;
        }

        bool IsSV1() const
        {
            return m_ProductID == PRODUCT_ID_SV1;
        }

        bool IsRB() const
        {
            return m_ProductID == PRODUCT_ID_RB;
        }

        bool IsMPod() const
        {
            return m_ProductID == PRODUCT_ID_MPOD;
        }

        bool IsCPod() const
        {
            return m_ProductID == PRODUCT_ID_CPOD;
        }

        bool IsStimTracker() const
        {
            return m_ProductID == PRODUCT_ID_STIMTRACKER;
        }

        bool IsXID1() const
        {
            return m_MajorFirmwareVer == 1;
        }

        bool IsXID2() const
        {
            return m_MajorFirmwareVer == 2;
        }

        bool IsXID1InputDevice() const
        {
            return IsXID1() && !IsStimTracker();
        }

    private:
        std::string m_DeviceName;
        int m_MajorFirmwareVer;
        int m_ProductID;
        int m_ModelID;
        bool m_requiresDelay;

        std::vector<DevicePort> m_DevicePorts;
    };
} // namespace Cedrus