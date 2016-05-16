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

#include "stim_tracker_t.h"

#include "xid_con_t.h"
#include "xid_glossary.h"

cedrus::stim_tracker_t::stim_tracker_t(
            boost::shared_ptr<xid_con_t> xid_con,
            boost::shared_ptr<const xid_device_config_t> dev_config)
    : m_xidCon(xid_con),
      m_config(dev_config)
{
    clear_lines();
}

cedrus::stim_tracker_t::~stim_tracker_t(void)
{
}

unsigned int cedrus::stim_tracker_t::get_pulse_duration( void )
{
    return xid_glossary::get_pulse_duration(m_xidCon);
}

void cedrus::stim_tracker_t::set_pulse_duration(unsigned int duration)
{
    xid_glossary::set_pulse_duration(m_xidCon, duration);
}

const boost::shared_ptr<const cedrus::xid_device_config_t> cedrus::stim_tracker_t::get_device_config() const
{
    return m_config;
}

int cedrus::stim_tracker_t::close_connection( void )
{
    return m_xidCon->close();
}

int cedrus::stim_tracker_t::open_connection( void )
{
    return m_xidCon->open();
}

bool cedrus::stim_tracker_t::has_lost_connection( void )
{
    return m_xidCon->has_lost_connection();
}

int cedrus::stim_tracker_t::get_baud_rate( void )
{
    return m_xidCon->get_baud_rate();
}

void cedrus::stim_tracker_t::set_baud_rate( unsigned char rate )
{
    m_xidCon->set_baud_rate(rate);
    xid_glossary::set_device_baud_rate(m_xidCon, rate);
}

void cedrus::stim_tracker_t::get_product_and_model_id( int *product_id, int *model_id )
{
    xid_glossary::get_product_and_model_id(m_xidCon, product_id, model_id);
}

int cedrus::stim_tracker_t::get_major_firmware_version( void )
{
    return xid_glossary::get_major_firmware_version(m_xidCon);
}

int cedrus::stim_tracker_t::get_minor_firmware_version( void )
{
    return xid_glossary::get_minor_firmware_version(m_xidCon);
}

std::string cedrus::stim_tracker_t::get_internal_product_name( void )
{
    return xid_glossary::get_internal_product_name(m_xidCon);
}

void cedrus::stim_tracker_t::raise_lines(unsigned int lines_bitmask, bool leave_remaining_lines)
{
    unsigned int output_lines = lines_bitmask;

    if(leave_remaining_lines)
        output_lines |= m_linesState;

    xid_glossary::set_digital_output_lines_st(m_xidCon, output_lines);

    m_linesState = output_lines;
}

void cedrus::stim_tracker_t::clear_lines( void )
{
    xid_glossary::set_digital_output_lines_st(m_xidCon, 0);
    m_linesState = 0;
}

void cedrus::stim_tracker_t::restore_factory_defaults( void )
{
    xid_glossary::restore_factory_defaults(m_xidCon);
}
