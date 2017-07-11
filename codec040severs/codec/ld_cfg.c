#include <stdio.h>
#include <stdlib.h>  /* malloc, free, rand */

#include "vproc_common.h"
#include "zl380twolf_config.h"
#include "vprocTwolf_access.h"
#include "zl380twolf_firmware.h"

#if 1
#include "vproc_common.c"
#include "vprocTwolf_access.c"
//#include "zl380twolf_config.c"
#include "zl380twolf_firmware.c"
#endif




#undef SAVE_IMAGE_TO_FLASH /*define this macro to save the firmware from RAM to flash*/
#define SAVE_CFG_TO_FLASH   /*define this macro to save the cfg from RAM to flash*/
/*quick test*/

#undef TW_HAL_VERIFY_DEBUG

#define MAX_WORDS_FOR_MULTIWORD_ACCESS_TEST  125


VprocStatusType LoadFwrConfig_Alt(uint8 mode);


//int init_dsp(void);

//int ld_cfg(void);
/*
int main (int argc, char** argv) {

	if(init_dsp())
		printf("-----------------------------------------\n");

	if(ld_cfg())
		printf("+++++++++++++++++++++++++++++++++++++++++\n");
	
    

    return 0;
}
*/
int init_dsp(void)
{
    int status = 0;
       
    status = VprocTwolfHbiInit();
    if (status < 0)
    {
        perror("tw_spi_access open");
        return -1;
    }
	return 0;
}

int ld_cfg(void)
{
	/*ÉÕÂ¼ÅäÖÃÎÄ¼þ
	 0 - load both firmware and confing
	 1 - load firmware only
     2 - load config only  
	*/
    if (LoadFwrConfig_Alt(2) !=  VPROC_STATUS_SUCCESS) {
         printf("ÉÕÂ¼Ê§°Ü.....\n"); 
		 return -1;
    } else          
        printf("ÉÕÂ¼Íê³É\n\n");       

    printf("\n");
    VprocTwolfHbiCleanup();
	
	return 0;
}

/*LoadFwrConfig_Alt - to load a converted *s3, *cr2 to c code into the device.
* Basically instead of loading the *.s3, *cr2 directly,
* use the tw_convert tool to convert the ascii hex fwr mage into code and compile 
* with the application
*
* input arg: mode:  0 - load both firmware and confing
*                   1 - load firmware only
*                   2 - load config only  
*/
VprocStatusType LoadFwrConfig_Alt(uint8 mode) {
		VprocStatusType status= VPROC_STATUS_SUCCESS;
        if ((mode == 0) || (mode == 1)) {
    		twFirmware st_Firmware;
    		st_Firmware.st_Fwr = (twFwr *)st_twFirmware;
    		st_Firmware.twFirmwareStreamLen = (uint16)firmwareStreamLen;
    		st_Firmware.execAddr  = (uint32)executionAddress;
    		st_Firmware.havePrgmBase = (uint8)haveProgramBaseAddress;
    		st_Firmware.prgmBase = (uint32)programBaseAddress;
                 
    		printf("\t1- Firmware boot loading started ....\n");
    
    		status  = VprocTwolfHbiBoot_alt(&st_Firmware);
    		if (status != VPROC_STATUS_SUCCESS) {
    			printf("Error %d:VprocTwolfHbiBoot()\n", status);
    			      
    			return -1;
    		}
    
    		printf("\t2- Loading the image to RAM....done\n");
#ifdef SAVE_IMAGE_TO_FLASH
    		printf("\t-- Saving firmware to flash....\n");
    		status = VprocTwolfSaveImgToFlash();
    		if (status != VPROC_STATUS_SUCCESS) {
    			printf("Error %d:VprocTwolfSaveImgToFlash()\n", status);
    			      
    			return status;
    		}
    		printf("\t-- Saving firmware to flash....done\n");
    
#endif
    
    		status  = VprocTwolfFirmwareStart();
    		if (status != VPROC_STATUS_SUCCESS) {
    			printf("Error %d:VprocTwolfFirmwareStart()\n", status);
    			      
    			return status;
    		}
        }
        if ((mode == 0) || (mode == 2)) {
    		printf("\t3- Loading the config file into the device RAM....\n");
    	
    		status  = VprocTwolfLoadConfig((dataArr *)st_twConfig, 
                                                    (uint16)configStreamLen);
    		if (status != VPROC_STATUS_SUCCESS) {
    			printf("Error %d:VprocTwolfLoadConfig()\n", status);
    			      
    			return status;
    		}
#ifdef SAVE_CFG_TO_FLASH
    	   printf("\t-- Saving config to flash....\n");
    	   status = VprocTwolfSaveCfgToFlash();
    	   if (status != VPROC_STATUS_SUCCESS) {
    			printf("Error %d:VprocTwolfSaveCfgToFlash()\n", status);
    			      
    			return status;
    	   }
    		printf("\t-- Saving config to flash....done\n");
    
#endif
        }
        {  /*Verify that the boot loading PASS or Fail*/
            uint16 val =0;

        	status  = VprocTwolfHbiRead(0x0022, 1, &val);
            if (status != VPROC_STATUS_SUCCESS) {
                printf("Error %d:VprocTwolfHbiRead()\n", status);
                VprocTwolfHbiCleanup();
                return -1;
            }
            if ((val == 38040) || 
                (val == 38050) || 
                (val == 38060) || 
                (val == 38080) ||
                (val == 38051) || 
                (val == 38041)) 
                 printf("\tDevice boot loading completed successfully...\n");
            else {
                 printf("\tDevice boot loading failed!!!...\n");
                 return VPROC_STATUS_FAILURE;
        	}
        }         
        
		/*Firmware reset - in order for the configuration to take effect
		 * NOTE:  The ZL38040 needs a soft reset for the uploaded configuration
		 *       to take effect. This soft-reset is sent below
		 *       if the ZL38040 is an I2S slave, if the I2S master is not stable
		 *       at the time of this reset, then that reset will not take effect. 
		 *       In that case the host has to to simply resend the reset 
		 *       command once the I2S master
		 *       is up and running and is at a stable state.
		 */
		status  = VprocTwolfReset(VPROC_RST_SOFTWARE);
		if (status != VPROC_STATUS_SUCCESS) {
			printf("Error %d:VprocTwolfReset()\n", status);
			      
			return status;
		}

		printf("\tDevice boot loading completed successfully...\n"); 
        return status;   
}

