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

#include "xid_device_scanner_t.h"
#include "xid_device_config_t.h"
#include "xid_con_t.h"
#include "xid_device_t.h"
#include "stim_tracker_t.h"

#include <iostream>

#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/exceptions.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include <sstream>

#ifdef __APPLE__
#   include "xid_device_scanner_helper_mac.h"
#elif defined(_WIN32)
#   include "xid_device_scanner_helper_win.h"
#endif

enum { OS_FILE_ERROR = -1,
    NO_FILE_SELECTED = 0};

cedrus::xid_device_scanner_t::xid_device_scanner_t(void)
{
    load_available_com_ports();
}

cedrus::xid_device_scanner_t::~xid_device_scanner_t(void)
{
}

void cedrus::xid_device_scanner_t::load_available_com_ports()
{
    load_com_ports_platform_specific( &available_com_ports_ );
}

void cedrus::xid_device_scanner_t::drop_every_connection()
{
    for (unsigned int i = 0; i < rb_devices_.size(); i++)
        rb_devices_[i]->close_connection();
    
    for (unsigned int i = 0; i < st_devices_.size(); i++)
        st_devices_[i]->close_connection();

    rb_devices_.clear();
    st_devices_.clear();
}

int cedrus::xid_device_scanner_t::detect_valid_xid_devices(const std::string &config_file_location)
{
    int devices = 0; //the return value
    boost::filesystem::path targetDir(config_file_location);
	boost::filesystem::directory_iterator it(targetDir), eod;

    // This will contain every devconfig we can find.
    std::vector< boost::shared_ptr<cedrus::xid_device_config_t> > master_config_list;

    load_available_com_ports();

    rb_devices_.clear();
    st_devices_.clear();
    
	BOOST_FOREACH(boost::filesystem::path const &p, std::make_pair(it, eod))
	{ 
	    if( is_regular_file(p) && p.extension() == ".devconfig" )
        {
            boost::property_tree::ptree pt;
            boost::property_tree::ini_parser::read_ini(p.string(), pt);

            master_config_list.push_back(cedrus::xid_device_config_t::config_for_device(&pt));
        }
	}

    for(std::vector<std::string>::iterator iter = available_com_ports_.begin(),
        end = available_com_ports_.end();
        iter != end; ++iter)
    {
        std::vector<boost::shared_ptr< cedrus::xid_device_config_t> > config_candidates;

        // If a config is for a device that can potentially be on this port, it's a candidate. This step
        // is important because "dry" port scans are very time consuming!
        BOOST_FOREACH(boost::shared_ptr<cedrus::xid_device_config_t> const config, master_config_list)
        {
            if ( !config->is_port_on_ignore_list( *iter ) )
                config_candidates.push_back(config);
        }

        if ( config_candidates.empty() )
            continue;

        bool device_found = false;
        const int baud_rate[] = { 115200, 19200, 9600, 57600, 38400 };
        const int num_bauds   = sizeof(baud_rate)/sizeof(int);
        
        // Here we're going to actually connect to a port and send it some signals. Our aim here is to
        // get an XID device's product/device and model IDs.
        for(int i = 0; i < num_bauds && !device_found; ++i)
        {
            boost::shared_ptr<cedrus::xid_con_t> xid_con(new xid_con_t(*iter, baud_rate[i]));

            if(xid_con->open() == NO_ERR)
            {
                char return_info[200];
                xid_con->flush_input();
                xid_con->flush_output();

                xid_con->send_xid_command("_c1",
                                          0,
                                          return_info,
                                          sizeof(return_info),
                                          5,
                                          1000,
                                          100);
                
                std::string info;
                if(return_info[0] == NULL)
                {
                    // there's a possibility that the device is in E-Prime mode.
                    // Go through the buffer and discard NULL characters, and
                    // only keep the non NULL characters.
                    for(int j = 0; j < sizeof(return_info); ++j)
                    {
                        if(return_info[j] != NULL)
                        {
                            info.append(&return_info[j], 1);
                        }
                    }
                }
                else
                    info = std::string(return_info);
                
                if( strstr(info.c_str(), "_xid") )
                {
                    // Found an XID device, see if it's one of the candidates. If not, panic.
                    device_found = true;
                    int product_id;
                    int model_id;

                    //What device is it? Get product/model ID, find the corresponding config
                    xid_con->get_product_and_model_id(product_id, model_id);

                    BOOST_FOREACH(boost::shared_ptr<cedrus::xid_device_config_t> const config, config_candidates)
                    {
                        if ( config->does_config_match_ids(product_id, model_id) )
                        {
                            ++devices;

                            if(strcmp(info.c_str(), "_xid0") != 0)
                            {
                                // device is not in XID mode.  Currently this library
                                // only supports XID mode so we issue command 'c10' to
                                // set the device into XID mode
                                char empty_return[10];
                                xid_con->send_xid_command("c10",
                                                          0,
                                                          empty_return,
                                                          sizeof(empty_return),
                                                          0);
                                
                                xid_con->flush_input();
                                xid_con->flush_output();
                            }

                            char dev_type[10];
                            xid_con->send_xid_command("_d2",
                                                      0,
                                                      dev_type,
                                                      sizeof(dev_type),
                                                      1,
                                                      1000);

                            if(dev_type[0] == 'S')
                            {
                                boost::shared_ptr<cedrus::stim_tracker_t> stim_tracker (new stim_tracker_t(xid_con, config));
                                st_devices_.push_back(stim_tracker);
                            }
                            else
                            {
                                boost::shared_ptr<cedrus::xid_device_t> rb_device (new xid_device_t(xid_con, config));
                                rb_devices_.push_back(rb_device);
                            }
                        }
                    }
                }
            }
            
            xid_con->close();
        }
    }
    return devices;
}

boost::shared_ptr<cedrus::xid_device_t> 
cedrus::xid_device_scanner_t::response_device_connection_at_index(unsigned int i)
{
    if(i >= rb_devices_.size())
        return boost::shared_ptr<xid_device_t>();

    return rb_devices_[i];
}

boost::shared_ptr<cedrus::stim_tracker_t>
cedrus::xid_device_scanner_t::stimtracker_connection_at_index(unsigned int i)
{
    if( i > st_devices_.size())
        return boost::shared_ptr<stim_tracker_t>();

    return st_devices_[i];
}

int cedrus::xid_device_scanner_t::rb_device_count() const
{
    return rb_devices_.size();
}

int cedrus::xid_device_scanner_t::st_device_count() const
{
    return st_devices_.size();
}
