
#include "device_factory.h"

#include "xid_device_t.h"
#include "xid2_device.h"
#include "stim_tracker_t.h"
#include "StimTracker2.h"
#include "xid_device_config_t.h"

#include <boost/foreach.hpp>

boost::shared_ptr<cedrus::base_device_t> cedrus::create_device
(
    const int product_id, // d2_value
    const int model_id,   // d3 value
    const int major_firmware_version, // d4 value
    const std::vector< boost::shared_ptr<cedrus::xid_device_config_t> > &config_candidates,
    boost::shared_ptr<xid_con_t> xid_con
)
{
    boost::shared_ptr<cedrus::base_device_t> result;

    // match the product/model id and firmware version to a devconfig
    BOOST_FOREACH(boost::shared_ptr<cedrus::xid_device_config_t> const config, config_candidates)
    {
        if (config->does_config_match_device(product_id, model_id, major_firmware_version))
        {
            // StimTracker or a c-pod
            if (product_id == 'S' || static_cast<char>(product_id) == '4')
            {
                if (major_firmware_version == 1)
                    result.reset(new stim_tracker_t(xid_con, config));
                else
                    result.reset(new StimTracker2(xid_con, config));
            }
            else
            {
                if (major_firmware_version == 1)
                {
                    result.reset(new xid_device_t(xid_con, config));
                }
                else
                {
                    result.reset(new xid2_device(xid_con, config));
                }
            }

            break;
        }
    }

    return result;
}
