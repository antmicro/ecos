#ifndef CYGONCE_PHY_MV88E1518_HEADER_
#define CYGONCE_PHY_MV88E1518_HEADER_
//==========================================================================
//
//      phy/MV88E1518.h
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
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_phy.h>

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_tables.h>

#include <cyg/io/eth_phy.h>
#include <cyg/io/eth_phy_dev.h>

#define BIT_(__n)       (1<<(__n))

// Define register addresses
#define MV88E1518_COPPER_CONTROL_REGISTER              0
#define MV88E1518_COPPER_STATUS_REGISTER               1
#define MV88E1518_COPPER_SPEC_STATUS_REGISTER_1        17

#define MV88E1518_COPPER_STATUS_AUTO_COMPLETE          BIT_(5)

#define MV88E1518_COPPER_SPEC_STATUS_GLOBAL_LINK       BIT_(3)
#define MV88E1518_COPPER_SPEC_STATUS_LINK              BIT_(10)
#define MV88E1518_COPPER_SPEC_STATUS_FULL_DUPLEX       BIT_(13)
#define MV88E1518_COPPER_SPEC_STATUS_100MBPS           BIT_(14)
#define MV88E1518_COPPER_SPEC_STATUS_1000MBPS          BIT_(15)

#endif // CYGONCE_PHY_MV88E1518_HEADER_
