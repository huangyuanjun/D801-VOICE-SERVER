#include "VprocTw_Hal.h"

/*HAL Init function - To open a file connection to the 
 * microsemi zl380xx_tw kernel char driver. This function must be called by
 * application initialization routine.
 *
 * return: a positive integer value for success, a negative value for failure
 */

#include <android/log.h>
#define  LOG_TAG    "myjni_led"
//#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
//#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
//#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

int VprocHALInit(void)
{
    int status = 0;
	printf(" VprocHALInit():1!!!!\n");
    char *file_name = "/dev/zl380tw";
    /*global file handle*/
	printf(" VprocHALInit():2!!!!\n");
	if (gTwolf_fd != -1) 
       close(gTwolf_fd);
    gTwolf_fd = -1;
    gTwolf_fd = open(file_name, O_RDWR);
	printf(" VprocHALInit():3!!!!\n");
    if (gTwolf_fd == -1)
    {
        perror("unble to open /dev/zl380tw "
        " make sure the driver is installed in the kernel");
	//	LOGE("unble to open /dev/zl380tw "" make sure the driver is installed in the kernel\n");
        return -1;
    }
	printf(" VprocHALInit():4!!!!\n");
    return status;
}

/*HAL clean up function - To close any open file connection
 * microsemi zl380xx_tw kernel kernel char driver
 *
 * return: a positive integer value for success, a negative value for failure
 */
void VprocHALcleanup(void)
{
    /*global file handle*/
    if (gTwolf_fd != -1) 
       close(gTwolf_fd);
    gTwolf_fd = -1;
}


/*HAL read function - To read from the microsemi_spis_tw kernel char driver
 * [param in] pData    - pointer to the data read 
 * [param in] numBytes - the number of bytes to write
 *
 * return: a positive integer value for success, a negative value for failure
 */
int VprocHALread(unsigned char *pData, unsigned short numBytes)
{
    int status = 0;
    if (gTwolf_fd < 0) {
        return -1;         
    } 
    status = read(gTwolf_fd, pData, numBytes);
    if (status < 0) {
        perror("microsemi_spis_tw_read driver");
    }
    return status;
}

/* HAL write function - To write to the microsemi_spis_tw kernel char driver
 * [param in] pData    - pointer to the data to write 
 * [param in] numBytes - the number of bytes to write
 *
 * return: a positive integer value for success, a negative value for failure
 */
int VprocHALwrite(unsigned char *pData, unsigned short numBytes)
{
    int status = 0;
    if (gTwolf_fd < 0) {
        return -1;         
    } 
    status = write(gTwolf_fd, pData, numBytes);
    if (status < 0) {
        perror("microsemi_spis_tw_write driver");
	//	LOGE("DSP write failed %d\n" ,status);
		close(gTwolf_fd);
		VprocHALInit();
		VprocHALwrite(pData,numBytes);
    }
	//LOGD("*****************************%d\n",status);
    return status;
}

/* HAL IOCTL functions 
 * [param in] cmd      - The IOCTL command as defined within microsemi_spis_tw.h
 * [param in] *arg     - the data to pass to the kernel driver funtion
 *
 * return: a positive integer value for success, a negative value for failure
 */
int ioctlHALfunctions (unsigned int cmd, void* arg) 
{
    int status = 0;	
    if (gTwolf_fd < 0) {
        return -1;         
    } 

    switch (cmd) {
    	case TWOLF_BOOT_PREPARE :
            status = ioctl(gTwolf_fd, TWOLF_BOOT_PREPARE);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_BOOT_PREPARE");
                return status;
            } 
                  	
        break;	
    	case TWOLF_BOOT_SEND_MORE_DATA: {
            status = ioctl(gTwolf_fd, TWOLF_BOOT_SEND_MORE_DATA, (char *)arg);	 
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_BOOT_SEND_MORE_DATA");
                return status;
            } 
            
    	    break;
    	}
    	case TWOLF_BOOT_CONCLUDE :
            status = ioctl(gTwolf_fd, TWOLF_BOOT_CONCLUDE);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_BOOT_PREPARE");
                return status;
            } 
                  	
    	break;	
    	
    	case TWOLF_CMD_PARAM_REG_ACCESS : {      
            /* Send a mailbox command and release the mailbox right to the host*/
            status = ioctl(gTwolf_fd, TWOLF_CMD_PARAM_REG_ACCESS, (unsigned short *)arg);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_CMD_PARAM_REG_ACCESS");
                return status; 
            } 
                      	
            break;
        }
    	case TWOLF_CMD_PARAM_RESULT_CHECK :
            status = ioctl(gTwolf_fd, TWOLF_CMD_PARAM_RESULT_CHECK);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_CMD_PARAM_RESULT_CHECK");
                return status;
            }       
            
       	break;
    	case TWOLF_RESET : {
             
            status = ioctl(gTwolf_fd, TWOLF_RESET, (unsigned short *)arg);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_RESET");
                return status;
            }           
            
    	    break;
        }
    	case TWOLF_SAVE_FWR_TO_FLASH :
            status = ioctl(gTwolf_fd, TWOLF_SAVE_FWR_TO_FLASH);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_SAVE_FWR_TO_FLASH");
                return status;
            }          
            
    	break;
    	case TWOLF_SAVE_CFG_TO_FLASH :

            status = ioctl(gTwolf_fd, TWOLF_SAVE_CFG_TO_FLASH);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_SAVE_CFG_TO_FLASH");
                return status;
            }          
        
            
    	break;
    	case TWOLF_HBI_INIT : {
            if (ioctl(gTwolf_fd, TWOLF_HBI_INIT, (unsigned short *)arg) == -1)
            {
                perror("microsemi_spis_tw ioctl TWOLF_HBI_INIT");
                return -1;
            } 
                     
    	    break;
        }
    	case TWOLF_LOAD_FWRCFG_FROM_FLASH : {
            if (ioctl(gTwolf_fd, TWOLF_LOAD_FWRCFG_FROM_FLASH, (unsigned short *)arg) == -1)
            {
                perror("microsemi_spis_tw ioctl TWOLF_LOAD_FWRCFG_FROM_FLASH");
                return -1;
            }           
    	    break;
        }
    	case TWOLF_LOAD_FWR_FROM_FLASH : {
            if (ioctl(gTwolf_fd, TWOLF_LOAD_FWR_FROM_FLASH, (unsigned short *)arg) == -1)
            {
                perror("microsemi_spis_tw ioctl TWOLF_LOAD_FWR_FROM_FLASH");
                return -1;
            }           
    	    break;
        }
    	case TWOLF_ERASE_ALL_FLASH :
            status = ioctl(gTwolf_fd, TWOLF_ERASE_ALL_FLASH);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_ERASE_ALL_FLASH");
                return status;
            }        
            
    	break;
    	case TWOLF_STOP_FWR :
            status = ioctl(gTwolf_fd, TWOLF_STOP_FWR);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_STOP_FWR");
                return status;
            }                  
            
    	break;
    	case TWOLF_START_FWR :
            status = ioctl(gTwolf_fd, TWOLF_START_FWR);
            if (status < 0)
            {
                perror("microsemi_spis_tw ioctl TWOLF_START_FWR");
                return status;
            }                  
            
    	break;
    	default: {
            perror("Invalid IOTCL!!!\n");
           return -1; 
        }   
     }
     return 0;
}
