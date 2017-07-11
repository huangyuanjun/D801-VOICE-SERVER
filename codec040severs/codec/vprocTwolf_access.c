/*
 * User Space API wrapper for the "/dev/microsemi_spis_tw" linux kernel driver
 * A host can use these functions to access the the microsemi Z
 * L38040/050/051/060/080 Voice Processing devices over a spi interface.  
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * Microsemi Inc. 2014, Jean Bony
 */

#include "vprocTwolf_access.h"
 
/* VprocTwolfcmdRegWr(): use this function to
 *   access the host command register
 *
 * Input Argument: cmd - the command to send
 * Return: (VprocStatusType) type error code (0 = success, else= fail)
 */


#if 0 /*not used*/
static VprocStatusType VprocTwolfcmdRegWr(unsigned short cmd)
{
    int status = 0;

    /* Send a mailbox command and release the mailbox right to the host*/

    status = ioctlHALfunctions(TWOLF_CMD_PARAM_REG_ACCESS, &cmd);
    if (status < 0)
    {
        return VPROC_STATUS_ERR_HBI;
    } 
    return status;       
}

static VprocStatusType VprocTwolfCheckCmdResult(void)
{
    int status = 0;
    /* verify the status of the last mailbox command*/

    status = ioctlHALfunctions(TWOLF_CMD_PARAM_RESULT_CHECK, NULL);
    if (status !=0) {
        VPROG_DBG_ERROR("Command failed...Resultcode = 0x%04x\n", status);
        return VPROC_STATUS_ERR_VERIFY;
    }
     /*wait*/
    return VPROC_STATUS_SUCCESS;
}
#endif
/******************************************************************************
 * TwolfPagedWrite()
 * This function selects the specified page, writes the number of specified
 * words, starting at the specified offset from a source buffer.
 *
 * \param[in] page Page to select
 * \param[in] offset Offset of the requested Page to read from
 * \param[in] numWords Number of words to read starting from the offset
 * \param[in] pSrc Pointer to the date to write
 *
 * \retval ::VP_STATUS_SUCCESS
 * \retval ::VP_STATUS_ERR_HBI
 ******************************************************************************/
static VprocStatusType
TwolfHbiPage255Write(
    unsigned char page,
    unsigned char offset,
    unsigned char numWords,
    unsigned short *pDdata)
{
    uint16 cmdWrd = (uint16)(page<<8) | (uint16)offset;
	if (VprocTwolfHbiWrite(cmdWrd, numWords, pDdata)!= VPROC_STATUS_SUCCESS) {
        return VPROC_STATUS_ERR_HBI;
    }
    return VPROC_STATUS_SUCCESS;
} /* TwolfHbiPagedWrite() */

/*------------------------------------------------------
 * Higher level functions - Can be called by a host application
 *------------------------------------------------------*/

/*VprocTwolfHbiRead - use this function to read up to 252 words from the device
 * \param[in] cmd of the requested device register to read from
 * \param[in] numWords Number of words to read starting from the offset
 * \param[in] pData Pointer to the data read
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfHbiRead(
    unsigned short cmd,   /*register to read from*/
    unsigned char numwords,
    unsigned short *pData)
{
    VprocStatusType status = VPROC_STATUS_SUCCESS;
    int i=0, j=0;
    unsigned char buf[256];

    if ((numwords == 0) || (numwords > 126)) {
        VPROG_DBG_ERROR("number of words is out of range. Maximum is 126\n");
        return VPROC_STATUS_INVALID_ARG;
    }
   	buf[0] = (cmd >> 8) & 0xff;
   	buf[1] = (cmd & 0xFF) ;

    status = VprocHALread(buf, numwords*2);
    if (status < 0) {
        return VPROC_STATUS_ERR_HBI;
    }

    /*print the data - status is the total number of bytes received*/
    for (i = 0; i < numwords; i++) {
        *(pData+i) = (buf[j]<<8) | buf[j+1];
        j +=2;
      //  VPROG_DBG_INFO("RD: addr 0x%04x = 0x%04x\n", (cmd+i), *(pData+i)); 
                                                          
    }
    return VPROC_STATUS_SUCCESS;
}

/*VprocTwolfHbiWrite - use this function to write up to 252 words to the device
 * \param[in] cmd of the requested device register to write to
 * \param[in] numWords Number of words to write starting from the offset
 * \param[in] pData Pointer to the data to write
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfHbiWrite(
    unsigned short cmd,   /*register to read from*/
    unsigned char numwords,
    unsigned short *pData)
{
    VprocStatusType status = VPROC_STATUS_SUCCESS;
    int i=0, j=2;
    unsigned char buf[256];
    if ((numwords == 0) || (numwords > 126)) {
        VPROG_DBG_INFO("number of words is out of range. Maximum is 126\n");
        return VPROC_STATUS_INVALID_ARG;
    }

   	buf[0] = (cmd >> 8) & 0xff;
   	buf[1] = (cmd & 0xFF);
   	VPROG_DBG_INFO("\n\nbuf[]=0x%02x, 0x%02x, ", buf[0], buf[1]);
    for (i=0;i<numwords;i++) {
        buf[j]   = (pData[i] >> 8) & 0xFF;
        buf[j+1] = (pData[i] & 0xFF);
        VPROG_DBG_INFO("0x%02x, 0x%02x, ", buf[j], buf[j+1]);
        j += 2;
    }
    VPROG_DBG_INFO("numBytes send = 0x%02x, ",(numwords+1)*2);
    /*add the command type bytes in the count*/
    status = VprocHALwrite(buf, (numwords+1)*2);
    if (status < 0) {
        perror("microsemi_spis_tw_write driver"); 
        return VPROC_STATUS_WR_FAILED;    
    }    
    return VPROC_STATUS_SUCCESS;
}

/*VprocTwolfHbiInit - use this function to initialize the device HBI
 *   This function must be the first function called by the application in order
 * to configure the interface before any of the API function can 
 * communicate with the device
 * at startup during the system init
 *   Configure the HBI_CONFIG_VAL as per the host system. But default
 *   config is good for most cases. See HBI section in device datasheet for 
 *   details
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfHbiInit(void)
{   
	printf(" VprocTwolfHbiInit:start!!!!\n");      
    unsigned short buf = HBI_CONFIG_VAL;
    if (VprocHALInit() < 0)
    {
		printf(" VprocHALInit():start!!!!\n");
        return VPROC_STATUS_DEV_NOT_INITIALIZED;
    }           
    if (ioctlHALfunctions(TWOLF_HBI_INIT, &buf) < 0) 
    {
		printf(" ioctlHALfunctions:start!!!!\n");
        return VPROC_STATUS_INIT_FAILED;
    }           
    return VPROC_STATUS_SUCCESS;
}

/*VprocTwolfHbiCleanup - To close any open communication path to
 * to the device
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfHbiCleanup(void)
{                
    VprocHALcleanup();          
    return VPROC_STATUS_SUCCESS;
}


/*VprocTwolfHbiDeviceCheck - Check whether the HBI and the device are working
 * properly
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfHbiDeviceCheck(void)
{                
    VprocStatusType status = VPROC_STATUS_SUCCESS;
    uint16 buf[2] = {0x1234, 0x5678};
    status = VprocTwolfHbiWrite(0x00C, 2, buf);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("Can not communicate with the device !!!\n\n");  
        return VPROC_STATUS_ERR_HBI;
    }
	status  = VprocTwolfHbiRead(0x00C, 2, buf);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("Can not communicate with the device !!!\n\n");  
        return VPROC_STATUS_ERR_HBI;
    }
    if ((buf[0] != 0x1234) && (buf[1] != 0x5600)) {
           VPROG_DBG_ERROR("Device is not responding properly !!!\n\n");  
           return VPROC_STATUS_ERR_HBI;   
    }
          
    return VPROC_STATUS_SUCCESS;
}

/*VprocTwolfLoadConfig() - use this function to load a custom or new config 
 * record into the device RAM to override the default config
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfLoadConfig(
    dataArr *pCr2Buf,
    unsigned short numElements)
{
    VprocStatusType status = VPROC_STATUS_SUCCESS;
    unsigned short i, buf;
    /*stop the current firmware but do not reset the device and do not go to boot mode*/

    /*send the config to the device RAM*/
    for (i=0; i<numElements; i++) {
        buf = pCr2Buf[i].value;
        status = VprocTwolfHbiWrite(pCr2Buf[i].reg, 1, &buf);
        if (status != VPROC_STATUS_SUCCESS) {
            return VPROC_STATUS_ERR_HBI;
        }

    }
    
    return status;
}

/* These 3 functions provide an alternative method to loading an *.s3
 *  firmware image into the device
 * Procedure:
 * 1- Call VprocTwolfHbiBootPrepare() to put the device in boot mode
 * 2- Call the VprocTwolfHbiBootMoreData() repeatedly by passing it a pointer
 *    to one line of the *.s3 image at a time until the full image (all lines)
 *    are transferred to the device successfully.
 *    When the transfer of a line is complete, this function will return the sta-
 *    tus VPROC_STATUS_BOOT_LOADING_MORE_DATA. Then when all lines of the image
 *    are transferred the function will return the status 
 *         VPROC_STATUS_BOOT_LOADING_CMP
 * 3- VprocTwolfHbiBootConclude() to complete and verify the image boot loading
 *    process 
 *    
 */
VprocStatusType VprocTwolfHbiBootPrepare(void){
                
	VprocStatusType status = VPROC_STATUS_SUCCESS;	

    status = ioctlHALfunctions(TWOLF_BOOT_PREPARE, NULL);
    if (status != VPROC_STATUS_SUCCESS)
    {
        return VPROC_STATUS_FW_LOAD_FAILED;
    } 
    return status;          
}


VprocStatusType VprocTwolfHbiBootMoreData(char *blockOfFwrData) 
{
    //return ioctl(twolf_fd, TWOLF_BOOT_SEND_MORE_DATA, blockOfFwrData);
    return ioctlHALfunctions(TWOLF_BOOT_SEND_MORE_DATA, blockOfFwrData);
       
}


VprocStatusType VprocTwolfHbiBootConclude(void)
{
	VprocStatusType status = VPROC_STATUS_SUCCESS;

    status = ioctlHALfunctions(TWOLF_BOOT_CONCLUDE, NULL);
    if (status != VPROC_STATUS_SUCCESS)
    {
        return VPROC_STATUS_FW_LOAD_FAILED;
    }        
 
    return status;    
         
}


/* HbiSrecBoot_alt() Use this alternate method to load the st_twFirmware.c 
 *(converted *.s3 to c code) to the device
 */
static VprocStatusType HbiSrecBoot_alt(
    twFirmware *st_firmware)
{
    uint16 index = 0;
    uint16 gTargetAddr[2] = {0,0};
    
    VprocStatusType status = VPROC_STATUS_SUCCESS;
    
    
    while (index < st_firmware->twFirmwareStreamLen) {
        
		/* put the address into our global target addr */
		gTargetAddr[0] = (uint16)((st_firmware->st_Fwr[index].targetAddr & 
					   	 									   0xFFFF0000)>>16);
		gTargetAddr[1] = (uint16)(st_firmware->st_Fwr[index].targetAddr & 
					   	 									   0x0000FFFF);
		
		VPROG_DBG_INFO("gTargetAddr[0] = 0x%04x, gTargetAddr[1] = 0x%04x: \n", 
		  								 gTargetAddr[0], gTargetAddr[1]);
		
		VPROG_DBG_INFO("numWords = %d: \n", 
		                             st_firmware->st_Fwr[index].numWords);
		
		/* write the data to the device */
		if (st_firmware->st_Fwr[index].numWords != 0) {
           uint8 offset = gTargetAddr[1] & 0x00FF;
           gTargetAddr[1] &= 0xFF00; /*zero out the lsbyte*/                                    
		   if (st_firmware->st_Fwr[index].useTargetAddr) {
		      status = VprocTwolfHbiWrite(PAGE_255_BASE_HI_REG, 2, gTargetAddr);
		      if (status != VPROC_STATUS_SUCCESS) {
		            VPROG_DBG_ERROR("Unable to set gTargetAddr[0] = 0x%04x,"
				" gTargetAddr[1] = 0x%04x: \n", gTargetAddr[0], gTargetAddr[1]);
		            return VPROC_STATUS_ERR_HBI;
		        }
		   }
		   status = TwolfHbiPage255Write(0xFF, offset, 
		   st_firmware->st_Fwr[index].numWords, st_firmware->st_Fwr[index].buf);
		   if(status != VPROC_STATUS_SUCCESS) {
		        VPROG_DBG_ERROR("status = %d, numWords = %d: \n", 
				                status, st_firmware->st_Fwr[index].numWords);
		        return status;
		   }
		
		}
		index++;
    }

    /*
     * convert the number of bytes to two 16 bit
     * values and write them to the requested page register
     */
    /* even number of bytes required */

    /* program the program's execution start register */
    gTargetAddr[0] = (uint16)((st_firmware->execAddr & 0xFFFF0000) >> 16);
    gTargetAddr[1] = (uint16)(st_firmware->execAddr & 0x0000FFFF);
    status = VprocTwolfHbiWrite(0x12C, 2, gTargetAddr);
    if(status != VPROC_STATUS_SUCCESS) {
       VPROG_DBG_ERROR(" unable to program page 1 execution address\n");
       return status;
    }

    /* print out the srecord program info */
    VPROG_DBG_INFO("prgmBase 0x%08lx\n", st_firmware->prgmBase);
    VPROG_DBG_INFO("execAddr 0x%08lx\n", st_firmware->execAddr);
    VPROG_DBG_INFO("DONE\n");
    return VPROC_STATUS_SUCCESS;
}

/*VprocTwolfHbiBoot_alt - use this function to bootload the firmware 
 * into the device
 * \param[in] pointer to image data structure
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 * \retval ::VPROC_STATUS_MAILBOX_BUSY
*/
VprocStatusType VprocTwolfHbiBoot_alt(twFirmware *st_firmware)
{
	VprocStatusType status = VPROC_STATUS_SUCCESS;

	/* write a value of 1 to address 0x14 (direct page offset 0x0A).
	 * to stop current firmware, reset the device into the Boot Rom mode.
	 */
    status = VprocTwolfHbiBootPrepare();
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }
    
    /*Transfer the image*/
    status =  HbiSrecBoot_alt(st_firmware);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }
   
    /*tell Twolf that the firmware loading is complete*/
    
    return VprocTwolfHbiBootConclude();
}



VprocStatusType VprocTwolfEraseFlash(void)
{
    VprocStatusType status = VPROC_STATUS_SUCCESS; 
    /*go to boot mode first */
    status = VprocTwolfReset(VPROC_RST_BOOT);
    if (status != VPROC_STATUS_SUCCESS)
    {
        return VPROC_STATUS_DEV_NOT_INITIALIZED;
    }        
               
    status = ioctlHALfunctions(TWOLF_ERASE_ALL_FLASH, NULL);
    if (status != VPROC_STATUS_SUCCESS)
    {
         return VPROC_STATUS_ERR_HBI;
    }        
 
    return status;
}

VprocStatusType VprocTwolfLoadFwrCfgFromFlash(uint16 image_number)
{
    //VprocStatusType status = VPROC_STATUS_SUCCESS;
    uint16 buf = image_number; 
               
    if (ioctlHALfunctions(TWOLF_LOAD_FWRCFG_FROM_FLASH, &buf) == -1)
    {
        return VPROC_STATUS_FW_LOAD_FAILED;
    }   
  
    return VPROC_STATUS_SUCCESS;
}

VprocStatusType VprocTwolfLoadFwrFromFlash(uint16 image_number)
{
    uint16 buf = image_number; 

    if (ioctlHALfunctions(TWOLF_LOAD_FWR_FROM_FLASH, &buf) == -1)
    {
        return VPROC_STATUS_FW_LOAD_FAILED;
    }
 
    return VPROC_STATUS_SUCCESS;
}


/* VprocTwolfReset(): use this function to reset the device.
 *  
 *
 * Input Argument: mode  - the reset mode (VPROC_RST_HARDWARE_ROM, 
 *         VPROC_RST_HARDWARE_ROM, VPROC_RST_SOFT, VPROC_RST_AEC)
 * Return: (VprocStatusType) type error code (0 = success, else= fail)
 */
VprocStatusType VprocTwolfReset(VprocResetMode mode)
{
    uint16 buf = mode;

    if (ioctlHALfunctions(TWOLF_RESET, &buf) == -1)
    {
        return VPROC_STATUS_DEV_NOT_INITIALIZED;
    }           
    
    //Vproc_msDelay(50); /*wait for device to settle*/
    return VPROC_STATUS_SUCCESS;
}

/* VprocTwolfSaveImgToFlash(): use this function to
 *     save both the config record and the firmware to flash. It Sets the bit
 *     which initiates a firmware save to flash 
 *
 * Input Argument: None
 * Return: (VprocStatusType) type error code (0 = success, else= fail)
 */

VprocStatusType VprocTwolfSaveImgToFlash(void)
{

    int status = 0;

    status = ioctlHALfunctions(TWOLF_SAVE_FWR_TO_FLASH, NULL);
    if (status < 0)
    {
        return VPROC_STATUS_FW_SAVE_FAILED;
    }          
    return status;
}

/* VprocTwolfSaveCfgToFlash(): use this function to
 *     save the config record to flash. It Sets the bit
 *     which initiates a config save to flash 
 *
 * Input Argument: None
 * Return: (VprocStatusType) type error code (0 = success, else= fail)
 * The firmware must be stopped first with VprocTwolfFirmwareStop()
 */

VprocStatusType VprocTwolfSaveCfgToFlash(void)
{

    int status = 0;
    /* verify the status of the last mailbox command*/

    status = ioctlHALfunctions(TWOLF_SAVE_CFG_TO_FLASH, NULL);
     if (status < 0)
    {
        return VPROC_STATUS_GFG_SAVE_FAILED;
    }          

    return status;
}

/*VprocTwolfFirmwareStart - use this function to start/restart the firmware
 * previously stopped with VprocTwolfFirmwareStop()
 * \param[in] none
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfFirmwareStart(void)
{
   
    int status = 0;
    /* verify the status of the last mailbox command*/

    status = ioctlHALfunctions(TWOLF_START_FWR, NULL);
    if (status < 0)
    {
        return VPROC_STATUS_ERR_HBI;
    }          

    return VPROC_STATUS_SUCCESS;
}

/*VprocTwolfFirmwareStop - use this function to stop the firmware currently running
 * And set the device in boot mode
 * \param[in] none
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfFirmwareStop(void)
{
   
    int status = 0;
    /* verify the status of the last mailbox command*/

   status = ioctlHALfunctions(TWOLF_STOP_FWR, NULL);
   if (status < 0)
    {
        return VPROC_STATUS_ERR_HBI;
    }         

    return VPROC_STATUS_SUCCESS;
}



/*VprocTwolfRecordConfigure() - use this function to onfigure the Twolf for audio
 * recording mode
 * [param in]  clockrate - the (16-bit) TDM/I2S clock rate in KHz
 * [param in]  fsrate    - the (16-bit) sample rate  in KHz
 * [param in]  aecon     - to enable or disable Echo Canceller processing
 */

VprocStatusType VprocTwolfUpstreamConfigure(unsigned short clockrate,/*in kHz*/ 
                                       unsigned short fsrate,   /*in Hz*/
                                       unsigned char aecOn)  /*0 for OFF, 1 for ON*/
{
    int status = 0;
    uint16 temp = 0;
    /* verify the status of the last mailbox command*/
    /* verify */
    if (!((fsrate ==48000) || 
        (fsrate ==44100) || 
        (fsrate ==16000) ||
        (fsrate ==8000)))
    {
       printf("Invalid sample rate of %u HZ...\n", fsrate);
       return VPROC_STATUS_INVALID_ARG;
    }
    if ((clockrate <512) || (clockrate >16384)) 
    {
       printf("Invalid clock rate of %u KHz...\n", clockrate);
       return VPROC_STATUS_INVALID_ARG;
    }
    temp = 0x8004;
    status = VprocTwolfHbiWrite(0x0260, 1, &temp);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }            
    /* verify the status of the last mailbox command*/
    temp = ((((1000*clockrate)/fsrate) - 1)<<4) | (fsrate/8000);
    printf("pclkrate = %u KHz, fsrate = %u Hz, calculated clkrate = 0x%04x\n", 
                              clockrate, fsrate, temp); 
    status = VprocTwolfHbiWrite(0x0262, 1, &temp);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }
    
    temp = 0x0003;
    status = VprocTwolfHbiWrite(0x02B0, 1, &temp);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }

    if (aecOn == 0) {  /*pure stereo bypass*/
        uint16 buf[] ={0x000d, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                     0x0000, 0x000d, 0x0001, 0x0002, 0x0000, 0x0000, 0x0000,
                     0x0000, 0x0000, 0x0000, 0x0000, 0x0000};    
    
        status = VprocTwolfHbiWrite(0x0202, 19, buf);
        if (status != VPROC_STATUS_SUCCESS) {
            VPROG_DBG_ERROR("ERROR %d: \n", status);
            return status;
        }
#ifdef TW_ADD_SIDETONE        
        temp = 0x0001; /*Connect DAC1 directly to the MIC1*/
        status = VprocTwolfHbiWrite(0x0210, 1, &temp);
        if (status != VPROC_STATUS_SUCCESS) {
            VPROG_DBG_ERROR("ERROR %d: \n", status);
            return status;
        }
#endif /*TW_ADD_SIDETONE*/   
       
    
    } else if (aecOn == 1) {
        uint16 buf[] ={0x0c05, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                     0x000d, 0x0000, 0x000e, 0x0000, 0x0000, 0x0000, 0x0000,
                     0x0000, 0x0000, 0x000d, 0x0001, 0x0005};    
           
          VPROG_DBG_INFO("AEC runs at 8 or 16KHz, fsrate > 16Kz cause audio"
                    " to be decimated to 16KHa, then re-sampled to 48KHz\n"); 

        status = VprocTwolfHbiWrite(0x0202, 19, buf);
        if (status != VPROC_STATUS_SUCCESS) {
            VPROG_DBG_ERROR("ERROR %d: \n", status);
            return status;
        }

    }
    
    /*soft-reset to apply the configuration*/
    temp = 0x0002;
    status = VprocTwolfHbiWrite(0x0006, 1, &temp);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }
    
    return VPROC_STATUS_SUCCESS;
}

/*VprocTwolfPlayConfigure() - use this function to configure the Twolf for I2S audio
 * recording mode
 * [param in]  clockrate - the (16-bit) TDM/I2S clock rate in KHz
 * [param in]  fsrate    - the (16-bit) sample rate  in KHz
 * [param in]  aecon     - to enable or disable Echo Canceller processing
 */

VprocStatusType VprocTwolfDownstreamConfigure(unsigned short clockrate, /*in kHz*/
                                       unsigned short fsrate,    /*in Hz*/
                                     unsigned char aecOn)/*0 for OFF, 1 for ON*/
{
    int status = 0;
    uint16 temp = 0; 

    /* verify */
    if (!((fsrate ==48000) || 
        (fsrate ==44100) || 
        (fsrate ==16000) ||
        (fsrate ==8000)))
    {
       printf("Invalid sample rate of %u HZ...\n", fsrate);
       return VPROC_STATUS_INVALID_ARG;
    }
    if ((clockrate <512) || (clockrate >16384)) 
    {
       printf("Invalid clock rate of %u KHz...\n", clockrate);
       return VPROC_STATUS_INVALID_ARG;
    }
    temp = 0x8004;
    status = VprocTwolfHbiWrite(0x0260, 1, &temp);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }  
         
    temp = ((((1000*clockrate)/fsrate) - 1)<<4) | (fsrate/8000);
    printf("pclkrate = %u KHz, fsrate = %u Hz, calculated clkrate = 0x%04x\n", 
                              clockrate, fsrate, temp); 
    status = VprocTwolfHbiWrite(0x0262, 1, &temp);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }

    if (aecOn == 0) {
        /*Playback in 2-channel stereo
         * I2S-1L -> DAC1, I2S-1R -> DAC2
         * DAC 1,2 enable, I2S-1l, 1R enable
         */      
        uint16 buf[] ={0x000f, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                     0x0005, 0x0006, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                     0x0000, 0x0000, 0x0000, 0x0000, 0x0000}; 

        /*DAC1-2 enable*/
        temp = 0xC000;
        status = VprocTwolfHbiWrite(0x2A0, 1, &temp);
        if (status != VPROC_STATUS_SUCCESS) {
            VPROG_DBG_ERROR("ERROR %d: \n", status);
            return status;
        }

        temp = 0xC000;
        status = VprocTwolfHbiWrite(0x2A2, 1, &temp);
        if (status != VPROC_STATUS_SUCCESS) {
            VPROG_DBG_ERROR("ERROR %d: \n", status);
            return status;
        }
              
        /*pure stereo bit matching bypass - AEC is removed from the audio path*/      
        status = VprocTwolfHbiWrite(0x0202, 19, buf);
        if (status != VPROC_STATUS_SUCCESS) {
            VPROG_DBG_ERROR("ERROR %d: \n", status);
            return status;
        }

    } else if (aecOn == 1) {
          /* AEC is in the audio path - audio will be decimated  
           * to either 8 or 16KHz - because the AEC can only process
           * one audio cahnnel of either 8 or 16 KHz sample rate
           */ 
        uint16 buf[] ={0x0c05, 0x0010, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                     0x000d, 0x0000, 0x000e, 0x0000, 0x0000, 0x0000, 0x0000,
                     0x0000, 0x0000, 0x000d, 0x0001, 0x0005}; 
                        
        VPROG_DBG_INFO("AEC runs at 8 or 16KHz only");
        status = VprocTwolfHbiWrite(0x0202, 2, buf);
        if (status != VPROC_STATUS_SUCCESS) {
            VPROG_DBG_ERROR("ERROR %d: \n", status);
            return status;
        }
          
    }

   
    /*soft-reset to apply the configuration*/
    temp = 0x0002;
    status = VprocTwolfHbiWrite(0x0006, 1, &temp);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }
    
    return VPROC_STATUS_SUCCESS;
}

#if 1
/*VprocTwolfMute - use this function to mute or unmute one the following 
 * audio ports
 * And set the device in boot mode
 * \param[in] on  : 0 for unmute; 1: for mute
 *
 * \retval ::VPROC_STATUS_SUCCESS
 * \retval ::VPROC_STATUS_ERR_HBI
 */
VprocStatusType VprocTwolfMute(VprocAudioPortsSel port, uint8 on)
{
   
    int status = 0;
    uint16 buf = 0;
    uint16 reg = 0;
    uint16 muteBit= 0;

    if (port == VPROC_SOUT) {
        reg =   0x0300; 
        muteBit = 0x0080;  
    } else if  (port == VPROC_ROUT) {
        reg =   0x0300; 
        muteBit = 0x0100;  
    } else {
        VPROG_DBG_ERROR("This port %d do not support mute!!! \n", port); 
    }
    
    /* verify the status of the last mailbox command*/
    
	status  = VprocTwolfHbiRead(reg, 1, &buf);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("Error %d: VprocTwolfHbiRead() !!!\n", status);  
        return VPROC_STATUS_ERR_HBI;
    }
    if (on) 
        buf |= muteBit;  /*set the mute*/
    else 
        buf &= ~muteBit; /*clear the mute*/

    status = VprocTwolfHbiWrite(reg, 1, &buf);
    if (status != VPROC_STATUS_SUCCESS) {
        VPROG_DBG_ERROR("ERROR %d: \n", status);
        return status;
    }

    return VPROC_STATUS_SUCCESS;
}

#endif
