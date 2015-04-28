#include <cyg/infra/cyg_type.h>
#include <cyg/infra/testcase.h>         // Test macros
#include <cyg/infra/cyg_ass.h>          // Assertion macros
#include <cyg/infra/diag.h>             // Diagnostic output

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL

#include <cyg/io/i2c.h>
#include <cyg/io/i2c_xc7z.h>
#include <string.h>

CYG_ZYNQ_I2C_BUS(zynq_bus_0, 100000);
CYG_I2C_DEVICE(i2c_eeprom, &zynq_bus_0, 0x5c, 0x00, CYG_I2C_DEFAULT_DELAY);
CYG_I2C_DEVICE(i2c_rtc, &zynq_bus_0, 0x6f, 0x00, CYG_I2C_DEFAULT_DELAY);

cyg_uint32 ConvU32(cyg_uint8* Array)
{
	cyg_uint32 Value = (*Array << 24);
	Value += (*(Array+1) << 16);
	Value += (*(Array+2) << 8);
	Value +=  *(Array+3);
	return Value;
}

cyg_uint8 bcd(cyg_uint16 dec)
{
	return ((dec/10)<<4)+(dec%10);
}

cyg_uint16 decimal(cyg_uint8 bcd)
{
	return ((bcd>>4)*10)+bcd%16;
}

cyg_uint16 iic_test(cyg_uint8 ModInfo, cyg_uint8 RTC)
{
    cyg_uint8 tx_data[10];
    cyg_uint8 rx_data[10];
    cyg_uint32 SerialNo, ProdNo, ModConf, Ddr2Size, FlashSize;
    cyg_uint8 MacAddr[6];
    cyg_uint16 Hour, Min, Sec;
    
    diag_printf("I2C test started\n");

    if(ModInfo)
    {
        // EPROM init
		tx_data[0] = 0xa8;
		tx_data[1] = 0x00;
		cyg_i2c_transaction_tx(&i2c_eeprom, true, tx_data, 2, true);

		tx_data[0] = 0x00;
		cyg_i2c_transaction_tx(&i2c_eeprom, true, tx_data, 1, false);
        cyg_i2c_transaction_rx(&i2c_eeprom, true, rx_data, 4, false, true);
		SerialNo = ConvU32(rx_data);

		tx_data[0] = 0x04;
		cyg_i2c_transaction_tx(&i2c_eeprom, true, tx_data, 1, false);
        cyg_i2c_transaction_rx(&i2c_eeprom, true, rx_data, 4, false, true);
		ProdNo = ConvU32(rx_data);

		tx_data[0] = 0x08;
		cyg_i2c_transaction_tx(&i2c_eeprom, true, tx_data, 1, false);
        cyg_i2c_transaction_rx(&i2c_eeprom, true, rx_data, 4, false, true);
		ModConf = ConvU32(rx_data);

		tx_data[0] = 0x10;
		cyg_i2c_transaction_tx(&i2c_eeprom, true, tx_data, 1, false);
        cyg_i2c_transaction_rx(&i2c_eeprom, true, MacAddr, 6, false, true);
        
		if ((ProdNo >> 16) == 0x321)
			diag_printf("Module Type: Mars ZX3 Rev %d\n", ProdNo & 0x00ff);
		else
    		diag_printf("Unknown Module\n");
		diag_printf("SerialNo = 0x%x\n", SerialNo);

		Ddr2Size = 8*(0x1 << (((ModConf >> 4) & 0xf)-1));
		FlashSize = (0x1 << ((ModConf & 0xf)-1));
		diag_printf("DDR2 memory: %dMB\n", Ddr2Size);
		diag_printf("Flash memory: %dMB\n", FlashSize);
		diag_printf("MAC0 Addr: %X:%X:%X:%X:%X:%X\n", MacAddr[0], MacAddr[1],
				MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]);
		diag_printf("MAC1 Addr: %X:%X:%X:%X:%X:%X\n", MacAddr[0], MacAddr[1],
					MacAddr[2], MacAddr[3], MacAddr[4], MacAddr[5]+1);
	}
	if (RTC)
	{
		diag_printf("Real time clock:\n");

        //RTC init
		tx_data[0] = 0x08;
		tx_data[0] = 0x41;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 2, true);

		tx_data[0] = 0x0d;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 1, false);
		cyg_i2c_transaction_rx(&i2c_rtc, true, rx_data, 1, false, true);
		tx_data[1] = rx_data[0] | 0x80;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 2, true);

        //set time 11:22:33
        tx_data[0] = 0x02;
        tx_data[1] = bcd(11) | 0x80;//h:24h mode
        tx_data[2] = 0x01;
        tx_data[3] = bcd(22);//m
        tx_data[4] = 0x00;
        tx_data[5] = bcd(33);//s
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 2, true);
		cyg_i2c_transaction_tx(&i2c_rtc, true, &tx_data[2], 2, true);
		cyg_i2c_transaction_tx(&i2c_rtc, true, &tx_data[4], 2, true);

        //set date 22.11.10
        tx_data[0] = 0x03;
        tx_data[1] = bcd(22);//d
        tx_data[2] = 0x04;
        tx_data[3] = bcd(11);//m
        tx_data[4] = 0x05;
        tx_data[5] = bcd(10);//y
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 2, true);
		cyg_i2c_transaction_tx(&i2c_rtc, true, &tx_data[2], 2, true);
		cyg_i2c_transaction_tx(&i2c_rtc, true, &tx_data[4], 2, true);

        //read time
		tx_data[0] = 0x00;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 1, false);
		cyg_i2c_transaction_rx(&i2c_rtc, true, &rx_data[0], 1, false, true);
		tx_data[0] = 0x01;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 1, false);
		cyg_i2c_transaction_rx(&i2c_rtc, true, &rx_data[1], 1, false, true);
		tx_data[0] = 0x02;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 1, false);
		cyg_i2c_transaction_rx(&i2c_rtc, true, &rx_data[2], 1, false, true);

		Sec = decimal(rx_data[0]);
		Min = decimal(rx_data[1]);
		Hour = decimal(rx_data[2] & 0x3f);
		diag_printf("Time %d:%d:%d\n", Hour, Min, Sec);
		if(Hour != 11 || Min != 22 || Sec != 33)
		    CYG_TEST_FAIL("Time set and read differs!");
		
        //read date
        tx_data[0] = 0x03;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 1, false);
		cyg_i2c_transaction_rx(&i2c_rtc, true, &rx_data[0], 1, false, true);
		tx_data[0] = 0x04;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 1, false);
		cyg_i2c_transaction_rx(&i2c_rtc, true, &rx_data[1], 1, false, true);
		tx_data[0] = 0x05;
		cyg_i2c_transaction_tx(&i2c_rtc, true, tx_data, 1, false);
		cyg_i2c_transaction_rx(&i2c_rtc, true, &rx_data[2], 1, false, true);
		diag_printf("Date %d.%d.%d\n", decimal(rx_data[0]), decimal(rx_data[1]), decimal(rx_data[2]));
	}
  
    CYG_TEST_PASS_FINISH ("I2C test finished\n");
}

void cyg_start(void)
{
    CYG_TEST_INIT();
    iic_test(1, 1);
}
