//==========================================================================
//
//      phy/88E1518.c
//
//      Ethernet PHY driver for Marvell 88E1518
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System. 
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 
// 2008, 2009 Free Software Foundation, Inc. 
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    vcesson
// Date:         2012-12-06
// Purpose:      
// Description:  Hardware driver for Ethernet PHY Marvell 88E1518
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include "MV88E1518.h"

static bool MV88E1518_stat(eth_phy_access_t *f, int *state)
{
    unsigned short phy_state;
    int tries;
    int auto_completed = 1;

    // Read negotiated state from the Copper Status Register
    if(_eth_phy_read(f, MV88E1518_COPPER_STATUS_REGISTER, f->phy_addr, &phy_state))
    {
        if((phy_state & MV88E1518_COPPER_STATUS_AUTO_COMPLETE) == 0)
        {
            auto_completed = 0;
            eth_phy_printf("... waiting for auto-negotiation");
            for(tries = 0; tries < CYGINT_DEVS_ETH_PHY_AUTO_NEGOTIATION_TIME; tries++)
            {
                if(_eth_phy_read(f, MV88E1518_COPPER_STATUS_REGISTER, f->phy_addr, &phy_state))
                {
                    if((phy_state & MV88E1518_COPPER_STATUS_AUTO_COMPLETE) != 0)
                    {
                        auto_completed = 1;
                        break;
                    }
                }
                CYGACC_CALL_IF_DELAY_US(1000000);   // 1 second
                eth_phy_printf(".");
            }
            eth_phy_printf("\n");
        }

        // Find configuration
        if(auto_completed)
        {
            // Read Copper Specific Status Register 1 (register 17)
            _eth_phy_read(f, MV88E1518_COPPER_SPEC_STATUS_REGISTER_1, f->phy_addr, &phy_state);

            // Check link state
            if((phy_state & MV88E1518_COPPER_SPEC_STATUS_LINK) && (phy_state & MV88E1518_COPPER_SPEC_STATUS_GLOBAL_LINK))
                *state |= ETH_PHY_STAT_LINK;
            // Check speed and duplex
            if(phy_state & MV88E1518_COPPER_SPEC_STATUS_1000MBPS )
                *state |= ETH_PHY_STAT_1000MB;
            if(phy_state & MV88E1518_COPPER_SPEC_STATUS_100MBPS )
                *state |= ETH_PHY_STAT_100MB;
            if(phy_state & MV88E1518_COPPER_SPEC_STATUS_FULL_DUPLEX)
                *state |= ETH_PHY_STAT_FDX;
            return true;
        }
    }
    return false;
}

_eth_phy_dev("Marvell 88E1518", 0x001410dd1, MV88E1518_stat)
