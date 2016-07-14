#include <common.h>
#include <command.h>
#include <environment.h>
#include <linux/stddef.h>
#include <search.h>
#include <errno.h>
#include <malloc.h>
#include <asm/arch/system.h>
#include <asm/arch/extern_param.h>
#include <asm/arch/fw_info.h>
#include <asm/arch/factorylib.h>
#include <asm/arch/factorylib_ro.h>
#include <generated/version_autogenerated.h>
#include <customized_feature.h>

#define BOOT_LETENV_FILE_NAME		FACTORY_HEADER_FILE_NAME"letvenv.txt"
#define BOOT_UI_RESOLUTION_FILE_NAME		FACTORY_HEADER_FILE_NAME"fb.bin"

	
//*** EXECUTE_CUSTOMIZED_FUNCTION_4 ***//
#include <asm/arch/flash_writer_u/mcp.h>
#include <asm/arch/rtk_ipc_shm.h>
#include <asm/arch/rbus/nand_reg.h>
#include <watchdog.h>

extern struct RTK119X_ipc_shm ipc_shm;
extern uchar boot_logo_enable;
extern uint custom_logo_src_width;
extern uint custom_logo_src_height;
extern uint custom_logo_dst_width;
extern uint custom_logo_dst_height;
extern uchar sys_logo_is_HDMI;
extern uint eMMC_fw_desc_table_start;
extern BOOT_MODE boot_mode;
//*** EXECUTE_CUSTOMIZED_FUNCTION_4 ***//

struct UI_RESOLUTION
{
	int width ;
	int height;	
};




struct hsearch_data env_htab_customized;


int env_import_customized(const char *buf, int check)
{
	env_t *ep = (env_t *)buf;

	if (check) {
		uint32_t crc;

		memcpy(&crc, &ep->crc, sizeof(crc));

		if (crc32(0, ep->data, ENV_SIZE) != crc) {
			set_default_env(NULL); // ck modify that prevent serious error messages
			return 0;
		}
	}

	if (himport_r(&env_htab_customized, (char *)ep->data, ENV_SIZE, '\0', 0)) {
		return 1;
	}

	printf("%s failed\n",__FUNCTION__);

	return 0;
}

/*
 * Look up variable from environment,
 * return address of storage for that variable,
 * or NULL if not found
 */
char *getenv_customized(const char *name)
{	
		ENTRY e, *ep;
		
		e.key	= name;
		e.data	= NULL;
		hsearch_r(e, FIND, &ep, &env_htab_customized);

		return ep ? ep->data : NULL;
}

void env_handler_customized(char *str)
{
	char *dst_addr= (char*)malloc(0x20000*sizeof(char));;
	char *retAddr;
	int dst_length;	
	
	printf("%s:\n", str);

	if (factory_read(BOOT_LETENV_FILE_NAME, &dst_addr, &dst_length)) {
		printf("------------can't find %s\n", BOOT_LETENV_FILE_NAME);
		
	}
	else {
		printf("------------%s found\n", BOOT_LETENV_FILE_NAME);

		//printf("dst_addr=%x\n, dst_length=%d\n",dst_addr,dst_length);
			
		env_import_customized(dst_addr,1);
		
		setenv("ethaddr",getenv_customized("ethaddr"));
			
	}
}

#define MISC_START_ADDRESS 0x638000

typedef struct _bootloader_message
{
    char command[32];
    char status[32];
    char recovery[1024];
} bootloader_message;

int detect_recovery_flag(char *str)
{
#ifdef CONFIG_SYS_RTK_EMMC_FLASH
	uint block_no;
	uint imageSize = 0;
	char *tmp_data = NULL;
	unsigned int misc_start_add=0;
	int i;
	
	printf("%s:\n", str);
	
	imageSize= sizeof(bootloader_message);
	
	if((misc_start_add = getenv_ulong("misc_add", 16, 0)) != 0)	
		block_no = misc_start_add / EMMC_BLOCK_SIZE ;
	else
		block_no = MISC_START_ADDRESS / EMMC_BLOCK_SIZE ;
	
	if (imageSize&(EMMC_BLOCK_SIZE-1)) {
		imageSize &= ~(EMMC_BLOCK_SIZE-1);
		imageSize += EMMC_BLOCK_SIZE;
	}
	
	tmp_data = (char*)malloc(imageSize);
                                
	if (!rtk_eMMC_read(block_no, imageSize, (uint *)tmp_data))
	{		
		printf("read MISC partition failed\n");
		return 0;
	}
	
	if(!memcmp(tmp_data,"boot-recovery",13))	
		return 1;	
	else
	{
		printf("dump misc info(32bytes):");
		for(i=0;i<32;i++)
			printf("%c",*(tmp_data+i));
		printf("\n");	
	}
	
	free(tmp_data);	
#endif	
	return 0;
}

#ifdef CONFIG_RESCUE_FROM_USB

#define RESCUE_DTB_FILENAME_IN_INSTALL_IMG		"package5/"CONFIG_RESCUE_FROM_USB_DTB
#define RESCUE_ROOTFS_FILENAME_IN_INSTALL_IMG	"package5/"CONFIG_RESCUE_FROM_USB_ROOTFS
#define RESCUE_KERNEL_FILENAME_IN_INSTALL_IMG	"package5/"CONFIG_RESCUE_FROM_USB_VMLINUX
#define RESCUE_FW_FILENAME_IN_INSTALL_IMG		"package5/"CONFIG_RESCUE_FROM_USB_AUDIO_CORE
#define RESCUE_COMPRESSED_INSTALL_IMG_TMP_BUFFER 0x20000000

extern BOOT_MODE boot_mode;

int boot_rescue_from_usb_OTA(char *str)
{
	char tmpbuf[128];
	int ret = RTK_PLAT_ERR_OK;
	char *filename;
	
	uint target_addr = 0;
	uint data_bytes = 0;
	char *dst_addr = NULL;
	uint dst_length = 0;
	uint tmp = 0;	
	char *bootm_argv[] = { "bootm", NULL, "-", NULL, NULL };
	int argc=4;
	
	printf("%s:\n", str);
	
	target_addr = RESCUE_COMPRESSED_INSTALL_IMG_TMP_BUFFER;
	
	filename="install.img";
	run_command("usb start", 0);
	sprintf(tmpbuf, "fatload usb 0:1 %x %s",RESCUE_COMPRESSED_INSTALL_IMG_TMP_BUFFER,filename);
	if (run_command(tmpbuf, 0) != 0) {
		goto loading_failed;
	}

	printf("Loading %s to 0x%x is OK.\n", filename,RESCUE_COMPRESSED_INSTALL_IMG_TMP_BUFFER);
	
	data_bytes = getenv_ulong("filesize", 16, 0);
	
	//copy rescue dtb	
	ret = tar_read(RESCUE_DTB_FILENAME_IN_INSTALL_IMG, target_addr, data_bytes, &dst_addr, &dst_length);
	
	if(ret>0)
	{	
		tmp = getenv_ulong("fdt_loadaddr", 16, 0);
		memcpy((char *)tmp, dst_addr, dst_length);
		printf("untar rescue dtb to 0x%x ok.\n",tmp);
	}
	else
	{
		filename="rescue dtb";
		goto untar_failed;		
	}
		
	//copy kernel		
	ret = tar_read(RESCUE_KERNEL_FILENAME_IN_INSTALL_IMG, target_addr, data_bytes, &dst_addr, &dst_length);
	
	if(ret>0)
	{	
		tmp = getenv_ulong("kernel_loadaddr", 16, 0);
		memcpy((char *)tmp, dst_addr, dst_length);
		printf("untar kernel to 0x%x ok.\n",tmp);
	}
	else
	{
		filename="kernel";
		goto untar_failed;		
	}	
			
	//copy rescue rootfs
	ret = tar_read(RESCUE_ROOTFS_FILENAME_IN_INSTALL_IMG, target_addr, data_bytes, &dst_addr, &dst_length);
	
	if(ret>0)
	{	
		tmp = getenv_ulong("rootfs_loadaddr", 16, 0);
		memcpy((char *)tmp, dst_addr, dst_length);
		printf("untar rescue rootfs to 0x%x ok.\n", tmp);
	}
	else
	{
		filename="rescue rootfs";
		goto untar_failed;		
	}

	//copy fw	
	ret = tar_read(RESCUE_FW_FILENAME_IN_INSTALL_IMG, target_addr, data_bytes, &dst_addr, &dst_length);
	
	if(ret>0)
	{	
		tmp = MIPS_AUDIO_FW_ENTRY_ADDR;
		memcpy((char *)tmp, dst_addr, dst_length);
		printf("untar fw to 0x%x ok.\n",tmp);
		run_command("go a", 0);
	}
	else
		printf("untar fw to %x failed.\n",tmp);	
	
			
	boot_mode = BOOT_RESCUE_MODE;
	
	if ((bootm_argv[1] = getenv("kernel_loadaddr")) == NULL) {
		bootm_argv[1] =(char*) CONFIG_KERNEL_LOADADDR;
	}

	if ((bootm_argv[3] = getenv("fdt_loadaddr")) == NULL) {
		bootm_argv[3] =(char*) CONFIG_FDT_LOADADDR;
	}


	ret = do_bootm(find_cmd("do_bootm"), 0, argc,bootm_argv);


	if (ret) {
		printf("ERROR do_bootm failed!\n");
		return -1;
	}

	/* Should not reach here */
	return 1;
	
	
untar_failed:
	printf("untar %s failed.\n\n", filename);	
	return RTK_PLAT_ERR_READ_RESCUE_IMG;	
	
loading_failed:
	printf("Loading \"%s\" from USB failed.\n", filename);
	return RTK_PLAT_ERR_READ_RESCUE_IMG;	

}
#endif

/**********************************************************
 * Append the information of uboot version to bootargs.
 **********************************************************/
int rtk_plat_boot_prep_version(void)
{	
	char *commandline = getenv("bootargs");
	char *tmp_cmdline = NULL;
	

	tmp_cmdline = (char *)malloc(strlen(commandline) +8+ strlen(PLAIN_VERSION)+2);
	if (!tmp_cmdline) {
		printf("%s: Malloc failed\n", __func__);
	}
	else {
		sprintf(tmp_cmdline, "%s U-boot=%s", commandline, PLAIN_VERSION);
		setenv("bootargs", tmp_cmdline);		
		free(tmp_cmdline);
	}
	debug("%s\n",getenv("bootargs"));

	return 0;
}

/**********************************************************
 * Modify UI RESOLUTION by dtb.
 **********************************************************/
struct UI_RESOLUTION get_UI_resolution_info(void)
{
	char *dst_addr;
	int dst_length;
	struct UI_RESOLUTION UI={0};
		
	if (factory_read(BOOT_UI_RESOLUTION_FILE_NAME, &dst_addr, &dst_length)) {
		debug("------------can't find %s\n", BOOT_UI_RESOLUTION_FILE_NAME);
		debug("dst_addr=%x\n, dst_length=%d\n",dst_addr,dst_length);
	}
	else {
		//printf("------------%s found\n", BOOT_UI_RESOLUTION_FILE_NAME);
				
		memcpy(&UI, dst_addr ,dst_length);		
		debug("UI.width=%d\n",UI.width);
		debug("UI.height=%d\n",UI.height);
								
	}
	
	return UI;
	
}


/* Calls "fdt set" */
int rtk_call_fdt_set_cmd(enum RTK_DTB_PATH path)
{
	struct UI_RESOLUTION UI={0};
	char fdt_set_cmd[50];
	int ret = 0;
		
	switch (path) {
	case RTK_DTB_PATH_FB: /* base */
		UI = get_UI_resolution_info();
		if( UI.width != 0 && UI.height!= 0)
			sprintf(fdt_set_cmd,"fdt set /fb resolution <%d %d>",UI.width,UI.height);
		else
			ret=-1;
		break;

	default:
		break;
	}
	
	debug("fdt_set_cmd=%s\n",fdt_set_cmd);
	
	if (run_command(fdt_set_cmd, 0) != 0) {
		printf("%s failed!\n",fdt_set_cmd);
		ret= -1;
	}

	return ret;	
}

void rtk_modify_dtb(int type,enum RTK_DTB_PATH path,int target_addr,int size)
{
	char fdt_addr_cmd[50];
	
	if(type!=FW_TYPE_KERNEL_DT)
		return;
	
	//Calls "fdt addr"
	sprintf(fdt_addr_cmd,"fdt addr %x %x",target_addr,size);
	debug("fdt_addr_cmd=%s\n",fdt_addr_cmd);
	
	if (run_command(fdt_addr_cmd, 0) != 0) {
		debug("%s failed!\n",fdt_addr_cmd);
		return -1;
	}
	
	//Calls "fdt set"
	if(!rtk_call_fdt_set_cmd(path))
		run_command("fdt print /fb resolution", 0);
	else
		printf("[warning]%s failed\n",__FUNCTION__);
			
}

//*** EXECUTE_CUSTOMIZED_FUNCTION_4 ***//
int rtk_plat_pre_init_fw_from_eMMC(
		uint fw_desc_table_base, part_desc_entry_v1_t* part_entry, int part_count,
		void* fw_entry, int fw_count,
		uchar version)
{
#ifdef CONFIG_SYS_RTK_EMMC_FLASH
	fw_desc_entry_v1_t *this_entry;
	fw_desc_entry_v11_t *v11_entry;
	fw_desc_entry_v21_t *v21_entry;
	int i;
	uint unit_len;
	char buf[64];
	uint fw_checksum = 0;
	unsigned int secure_mode;
	unsigned char ks[16],kh[16],kimg[16];
#ifdef CONFIG_CMD_KEY_BURNING
	unsigned int * Kh_key_ptr = NULL; 
#else
	unsigned int * Kh_key_ptr = Kh_key_default; 
#endif
	unsigned int img_truncated_size; // install_a will append 256-byte signature data to it
	int ret;
	boot_av_info_t *boot_av;
	uint block_no;
	MEM_LAYOUT_WHEN_READ_IMAGE_T mem_layout;
	uint imageSize = 0;
	uint decompressedSize = 0;

	// extern variable
	extern unsigned int mcp_dscpt_addr;
	mcp_dscpt_addr = 0;

	secure_mode = rtk_get_secure_boot_type();
	img_truncated_size = RSA_SIGNATURE_LENGTH;
	
	unsigned char str[16];// old array size is 5, change to 16. To avoid the risk in memory overlap.
	int magic = SWAPEND32(0x16803001);
	int offset = SWAPEND32(MIPS_SHARED_MEMORY_ENTRY_ADDR);
	
	/* find fw_entry structure according to version */
	switch (version)
	{
		case FW_DESC_TABLE_V1_T_VERSION_1:
			unit_len = sizeof(fw_desc_entry_v1_t);
			break;

		case FW_DESC_TABLE_V1_T_VERSION_11:
			unit_len = sizeof(fw_desc_entry_v11_t);
			break;

		case FW_DESC_TABLE_V1_T_VERSION_21:
			unit_len = sizeof(fw_desc_entry_v21_t);
			break;

		default:
			return RTK_PLAT_ERR_READ_FW_IMG;
	}

	/* clear boot_av_info memory */
	boot_av = (boot_av_info_t *) MIPS_BOOT_AV_INFO_ADDR;
	memset(boot_av, 0, sizeof(boot_av_info_t));
	

	for (i = fw_count-1; 0<=i ; i--)
	{	
		EXECUTE_CUSTOMIZE_FUNC(0); // insert execute customer callback at here

		this_entry = (fw_desc_entry_v1_t *)(fw_entry + unit_len * i);
		
		if (this_entry->target_addr)
		{
			if (boot_mode == BOOT_RESCUE_MODE || boot_mode == BOOT_ANDROID_MODE)
			{
				switch(this_entry->type)
				{
					case FW_TYPE_AUDIO:
						if(boot_mode == BOOT_KERNEL_ONLY_MODE)
							continue;
						else
						{
							ipc_shm.audio_fw_entry_pt = CPU_TO_BE32(this_entry->target_addr | MIPS_KSEG0BASE);
							printf("Audio FW:\n");
						}
						break;
						
					default:
						//printf("Unknown FW (%d):\n", this_entry->type);
						continue;
				}
			}
			else
			{
				switch(this_entry->type)
				{
					case FW_TYPE_AUDIO:
						if(boot_mode == BOOT_KERNEL_ONLY_MODE)
							continue;
						else
						{
							ipc_shm.audio_fw_entry_pt = CPU_TO_BE32(this_entry->target_addr | MIPS_KSEG0BASE);
							printf("Audio FW:\n");
						}
						break;

					case FW_TYPE_IMAGE_FILE:
						printf("IMAGE FILE:\n");

						/* assign boot_av structure */
						boot_av->dwMagicNumber = SWAPEND32(BOOT_AV_INFO_MAGICNO);					
						if(boot_logo_enable)
						{
							boot_av-> logo_enable = boot_logo_enable;
							boot_av-> logo_addr = CPU_TO_BE32(this_entry->target_addr);
							boot_av-> src_width = CPU_TO_BE32(custom_logo_src_width);
							boot_av-> src_height = CPU_TO_BE32(custom_logo_src_height);
							boot_av-> dst_width = CPU_TO_BE32(custom_logo_dst_width);
							boot_av-> dst_height = CPU_TO_BE32(custom_logo_dst_height);		
						}

						break;

					default:
						//printf("Unknown FW (%d):\n", this_entry->type);
						continue;
				}
			}
																			
			printf("\t FW Image to 0x%08x, size=0x%08x (0x%08x)\n",
				this_entry->target_addr, this_entry->length, this_entry->target_addr + this_entry->length);

			printf("\t FW Image fr 0x%08x %s\n", eMMC_fw_desc_table_start + this_entry->offset, this_entry->lzma ? "(lzma)" : "(non-lzma)");													

			WATCHDOG_KICK();

				/* secure mode and lzma will only apply to fw image */
				if (this_entry->type == FW_TYPE_AUDIO )
				{
					/* get memory layout before copy fw image */
					mem_layout.bIsEncrypted = (secure_mode != NONE_SECURE_BOOT);
					mem_layout.bIsCompressed = this_entry->lzma;
					mem_layout.image_target_addr = this_entry->target_addr & (~MIPS_KSEG_MSK);
				}
				else
				{
					/* get memory layout before copy other image */
					mem_layout.bIsEncrypted = 0;
					mem_layout.bIsCompressed = 0;
					mem_layout.image_target_addr = this_entry->target_addr & (~MIPS_KSEG_MSK);
				}

				get_mem_layout_when_read_image(&mem_layout);

				imageSize = this_entry->length;

				// 512B aligned
				if (imageSize&(EMMC_BLOCK_SIZE-1)) {
					imageSize &= ~(EMMC_BLOCK_SIZE-1);
					imageSize += EMMC_BLOCK_SIZE;
				}
								
				block_no = (eMMC_fw_desc_table_start + this_entry->offset) / EMMC_BLOCK_SIZE ;
                                
				if (!rtk_eMMC_read(block_no, imageSize, (uint *)mem_layout.flash_to_ram_addr))
				{
					printf("[ERR] Read fw error (type:%d)!\n", this_entry->type);

					return RTK_PLAT_ERR_READ_FW_IMG;
				}
#if 1
				/* Check checksum */
				fw_checksum = get_checksum((uchar *)mem_layout.flash_to_ram_addr, this_entry->length);

				if (this_entry->checksum != fw_checksum && secure_mode!= RTK_SECURE_BOOT)
				{
					printf("\t FW Image checksum FAILED\n");
					printf("\t FW Image entry  checksum=0x%08x\n", this_entry->checksum);
					printf("\t FW Image result checksum=0x%08x\n", fw_checksum);
					return RTK_PLAT_ERR_READ_FW_IMG;
				}
#endif
				/* if secure mode, do AES CBC decrypt */
				if (mem_layout.bIsEncrypted)
				{
					if (secure_mode == RTK_SECURE_BOOT)
					{       
						//rtk_hexdump("the first 32-byte encrypted data", (unsigned char *)mem_layout.encrpyted_addr, 32);
						//rtk_hexdump("the last 512-byte encrypted data", (unsigned char *)(ENCRYPTED_LINUX_KERNEL_ADDR+this_entry->length-512), 512);

                        memset(ks,0x00,16);
                        memset(kh,0x00,16);
                        memset(kimg,0x00,16);

#ifdef CONFIG_CMD_KEY_BURNING
                        OTP_Get_Byte(OTP_K_S, ks, 16);
                        OTP_Get_Byte(OTP_K_H, kh, 16);
                        sync();
						flush_cache((unsigned int) ks, 16);
						flush_cache((unsigned int) kh, 16);
#endif
                        AES_ECB_encrypt(ks, 16, kimg, kh);
						flush_cache((unsigned int) kimg, 16);
                        sync();

                        Kh_key_ptr = kimg;
                        //rtk_hexdump("kimg key : ", (unsigned char *)kimg, 16);
                        Kh_key_ptr[0] = swap_endian(Kh_key_ptr[0]);
                        Kh_key_ptr[1] = swap_endian(Kh_key_ptr[1]);
                        Kh_key_ptr[2] = swap_endian(Kh_key_ptr[2]);
                        Kh_key_ptr[3] = swap_endian(Kh_key_ptr[3]);
                        //rtk_hexdump("Kh_key_ptr : ", (unsigned char *)Kh_key_ptr, 16);
						flush_cache((unsigned int) kimg, 16);
                                                
						// decrypt image
						printf("to decrypt...\n");						
						flush_cache((unsigned int) mem_layout.encrpyted_addr, this_entry->length);
						if (decrypt_image((char *)mem_layout.encrpyted_addr,
							(char *)mem_layout.decrypted_addr,
							this_entry->length,
							Kh_key_ptr))
						{
							printf("decrypt image(%d) error!\n", this_entry->type);
							return RTK_PLAT_ERR_READ_FW_IMG;
						}

						sync();
                        memset(ks,0x00,16);
                        memset(kh,0x00,16);
                        memset(kimg,0x00,16);

						//rtk_hexdump("the first 32-byte decrypted data", (unsigned char *)mem_layout.decrypted_addr, 32);

						//reverse_signature( (unsigned char *)(mem_layout.decrypted_addr + imageSize - img_truncated_size) );

						flush_cache((unsigned int) mem_layout.decrypted_addr, this_entry->length);
						ret = Verify_SHA256_hash( (unsigned char *)mem_layout.decrypted_addr,
												  this_entry->length - img_truncated_size,
												  (unsigned char *)(mem_layout.decrypted_addr + this_entry->length - img_truncated_size),
												  1 );						  
						if( ret < 0 ) {
							printf("[ERR] %s: verify hash fail(%d)\n", __FUNCTION__, ret );
							return RTK_PLAT_ERR_READ_FW_IMG;
						}

						imageSize = imageSize - img_truncated_size - SHA256_SIZE;
					}
				}

				/* if lzma type, decompress image */
				if (mem_layout.bIsCompressed)
				{
					if (lzmaBuffToBuffDecompress((uchar*)mem_layout.decompressed_addr, &decompressedSize, (uchar*)mem_layout.compressed_addr, imageSize) != 0)
					{
						printf("[ERR] %s:Decompress using LZMA error!!\n", __FUNCTION__);

						return RTK_PLAT_ERR_READ_FW_IMG;
					}
				}
			}
		}

		if (version == FW_DESC_TABLE_V1_T_VERSION_11)
		{
			v11_entry = (fw_desc_entry_v11_t*) (fw_entry + unit_len * i);

			if (v11_entry->act_size != 0)
			{
				// string format: "part_num:act_size:hash,"
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%d:%d:", v11_entry->part_num, v11_entry->act_size);
//				strncat(str_phash, buf, strlen(buf));
				memset(buf, 0, sizeof(buf));
				memcpy(buf, v11_entry->hash, sizeof(v11_entry->hash));
				buf[sizeof(v11_entry->hash)] = ',';
//				strncat(str_phash, buf, strlen(buf));
			}
		}
		else if (version == FW_DESC_TABLE_V1_T_VERSION_21)
		{
			v21_entry = (fw_desc_entry_v21_t *)this_entry;

#if defined(Config_Secure_RSA_TRUE)
			// exclude partition 0 (contain bootcode/kernel/audio/video image)
			if ( (v21_entry->part_num != PART_TYPE_RESERVED) &&
				(v21_entry->act_size != 0) ){
				// recover hash value from signature
				memset(checksum, 0, sizeof(v21_entry->RSA_sign));
				memset(signature, 0, sizeof(v21_entry->RSA_sign)+1);
				memcpy(signature, v21_entry->RSA_sign, sizeof(v21_entry->RSA_sign));

				rsa_verify(signature, Config_Secure_RSA_MODULUS, checksum);

				// string format: "part_num:act_size:hash,"
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%d:%d:%s,", v21_entry->part_num, v21_entry->act_size, checksum);
//				strncat(str_phash, buf, strlen(buf));
				//printf("[DEBUG_MSG] part_num:%x, act_size:%x, recovered hash:%s\n", v21_entry->part_num, v21_entry->act_size, checksum);
			}
#endif	/* defined(Config_Secure_RSA_TRUE) */
		}
	//}


	/* set boot_av_info_ptr */
	if (boot_av->dwMagicNumber == SWAPEND32(BOOT_AV_INFO_MAGICNO))
	{
		boot_av->bHDMImode = sys_logo_is_HDMI;

		// enable audio sound
		if (boot_av->dwAudioStreamLength != 0)
		{
			;
		}

		ipc_shm.pov_boot_av_info = SWAPEND32((uint) boot_av);		

		#ifdef DUBUG_BOOT_AV_INFO
		dump_boot_av_info(boot_av);
		#endif
	}
	if (boot_mode != BOOT_MANUAL_MODE)
	{
		if (ipc_shm.audio_fw_entry_pt != 0){				
			printf("Start A/V Firmware ...\n");				
			memcpy((unsigned char *)(MIPS_SHARED_MEMORY_ENTRY_ADDR+0xC4), &ipc_shm, sizeof(ipc_shm));
			memcpy((unsigned char *)(MIPS_SHARED_MEMORY_ENTRY_ADDR), &magic, sizeof(magic));
			memcpy((unsigned char *)(MIPS_SHARED_MEMORY_ENTRY_ADDR +4), &offset, sizeof(offset));								
			flush_dcache_all();
			*(volatile u32 *)(0x1801b110)=0x120;
			rtd_setbits(CLOCK_ENABLE2_reg,_BIT4);
		}
	}
#endif /* CONFIG_SYS_RTK_EMMC_FLASH */
	
	return RTK_PLAT_ERR_OK;
}


/*
 * Use firmware description table to read images from eMMC flash.
 */
int rtk_plat_read_kernel_from_eMMC(
		uint fw_desc_table_base, part_desc_entry_v1_t* part_entry, int part_count,
		void* fw_entry, int fw_count,
		uchar version)
{
#ifdef CONFIG_SYS_RTK_EMMC_FLASH
	fw_desc_entry_v1_t *this_entry;
	fw_desc_entry_v11_t *v11_entry;
	fw_desc_entry_v21_t *v21_entry;
	int i;
	uint unit_len;
	char buf[64];
	uint fw_checksum = 0;
	unsigned int secure_mode;
	unsigned char ks[16],kh[16],kimg[16];
#ifdef CONFIG_CMD_KEY_BURNING
	unsigned int * Kh_key_ptr = NULL; 
#else
	unsigned int * Kh_key_ptr = Kh_key_default; 
#endif
	unsigned int img_truncated_size; // install_a will append 256-byte signature data to it
	int ret;
	boot_av_info_t *boot_av;
	uint block_no;
	MEM_LAYOUT_WHEN_READ_IMAGE_T mem_layout;
	uint imageSize = 0;
	uint decompressedSize = 0;

	// extern variable
	extern unsigned int mcp_dscpt_addr;
	mcp_dscpt_addr = 0;

	secure_mode = rtk_get_secure_boot_type();
	img_truncated_size = RSA_SIGNATURE_LENGTH;
	
	unsigned char str[16];// old array size is 5, change to 16. To avoid the risk in memory overlap.

	/* find fw_entry structure according to version */
	switch (version)
	{
		case FW_DESC_TABLE_V1_T_VERSION_1:
			unit_len = sizeof(fw_desc_entry_v1_t);
			break;

		case FW_DESC_TABLE_V1_T_VERSION_11:
			unit_len = sizeof(fw_desc_entry_v11_t);
			break;

		case FW_DESC_TABLE_V1_T_VERSION_21:
			unit_len = sizeof(fw_desc_entry_v21_t);
			break;

		default:
			return RTK_PLAT_ERR_READ_FW_IMG;
	}

	/* clear boot_av_info memory */
	boot_av = (boot_av_info_t *) MIPS_BOOT_AV_INFO_ADDR;
	memset(boot_av, 0, sizeof(boot_av_info_t));
	


	/* parse each fw_entry */
	for (i = 0; i < fw_count; i++)
	{	
		EXECUTE_CUSTOMIZE_FUNC(0); // insert execute customer callback at here

		this_entry = (fw_desc_entry_v1_t *)(fw_entry + unit_len * i);
		
		if (this_entry->target_addr)
		{
			if (boot_mode == BOOT_RESCUE_MODE || boot_mode == BOOT_ANDROID_MODE)
			{
				switch(this_entry->type)
				{
					case FW_TYPE_KERNEL:
						//kernel_entry = this_entry;
						memset(str, 0, sizeof(str));
						sprintf(str, "%x", this_entry->target_addr); /* write entry-point into string */
						setenv("kernel_loadaddr", str);
						printf("Kernel:\n");
						break;

					case FW_TYPE_RESCUE_DT:
						this_entry->target_addr = rtk_plat_get_dtb_target_address(this_entry->target_addr);
						//printf("this_entry->target_addr =%x\n",this_entry->target_addr);
						memset(str, 0, sizeof(str));
						sprintf(str, "%x", this_entry->target_addr); /* write entry-point into string */
						setenv("fdt_loadaddr", str);
						printf("Rescue DT:\n");
						break;

					case FW_TYPE_RESCUE_ROOTFS:
						printf("Rescue ROOTFS:\n");
						break;

					case FW_TYPE_TEE:
#ifdef CONFIG_TEE
						printf("TEE:\n");
						tee_enable=1;
						break;
#else
						continue;
#endif					
					default:
						//printf("Unknown FW (%d):\n", this_entry->type);
						continue;
				}
			}
			else
			{
				switch(this_entry->type)
				{
					case FW_TYPE_BOOTCODE:
						printf("Boot Code:\n");
						break;

					case FW_TYPE_KERNEL:
						//kernel_entry = this_entry;
						memset(str, 0, sizeof(str));
						sprintf(str, "%x", this_entry->target_addr); /* write entry-point into string */
						setenv("kernel_loadaddr", str);
						printf("Kernel:\n");
						break;

					case FW_TYPE_KERNEL_DT:					
						this_entry->target_addr = rtk_plat_get_dtb_target_address(this_entry->target_addr);
						//printf("this_entry->target_addr =%x\n",this_entry->target_addr);
						memset(str, 0, sizeof(str));
						sprintf(str, "%x", this_entry->target_addr); /* write entry-point into string */
						setenv("fdt_loadaddr", str);				
						printf("DT:\n");
						break;

					case FW_TYPE_KERNEL_ROOTFS:
						printf("ROOTFS:\n");
						break;

					case FW_TYPE_TEE:
#ifdef CONFIG_TEE						
						printf("TEE:\n");
						tee_enable=1;
						break;
#else
						continue;
#endif
										
					case FW_TYPE_JFFS2:
						printf("JFFS2 Image:\n");
						break;

					case FW_TYPE_SQUASH:
						printf("Squash Image:\n");
						break;

					case FW_TYPE_EXT3:
						printf("EXT3 Image:\n");
						break;

					case FW_TYPE_ODD:
						printf("ODD Image:\n");
						break;

					case FW_TYPE_YAFFS2:
						printf("YAFFS2 Image:\n");
						break;
					
					default:
						//printf("Unknown FW (%d):\n", this_entry->type);
						continue;
				}
			}
			
			printf("\t FW Image to 0x%08x, size=0x%08x (0x%08x)\n",
				this_entry->target_addr, this_entry->length, this_entry->target_addr + this_entry->length);

			printf("\t FW Image fr 0x%08x %s\n", eMMC_fw_desc_table_start + this_entry->offset, this_entry->lzma ? "(lzma)" : "(non-lzma)");														
																
																
			WATCHDOG_KICK();

				/* secure mode and lzma will only apply to fw image */
				if (this_entry->type == FW_TYPE_KERNEL ||
					this_entry->type == FW_TYPE_KERNEL_ROOTFS ||
					this_entry->type == FW_TYPE_RESCUE_ROOTFS )
				{
					/* get memory layout before copy fw image */
					mem_layout.bIsEncrypted = (secure_mode != NONE_SECURE_BOOT);
					mem_layout.bIsCompressed = this_entry->lzma;
					mem_layout.image_target_addr = this_entry->target_addr & (~MIPS_KSEG_MSK);
				}
				else
				{
					/* get memory layout before copy other image */
					mem_layout.bIsEncrypted = 0;
					mem_layout.bIsCompressed = 0;
					mem_layout.image_target_addr = this_entry->target_addr & (~MIPS_KSEG_MSK);
				}

				get_mem_layout_when_read_image(&mem_layout);

				imageSize = this_entry->length;

				// 512B aligned
				if (imageSize&(EMMC_BLOCK_SIZE-1)) {
					imageSize &= ~(EMMC_BLOCK_SIZE-1);
					imageSize += EMMC_BLOCK_SIZE;
				}
								
				block_no = (eMMC_fw_desc_table_start + this_entry->offset) / EMMC_BLOCK_SIZE ;
                                
				if (!rtk_eMMC_read(block_no, imageSize, (uint *)mem_layout.flash_to_ram_addr))
				{
					printf("[ERR] Read fw error (type:%d)!\n", this_entry->type);

					return RTK_PLAT_ERR_READ_FW_IMG;
				}
#if 1
				/* Check checksum */
				fw_checksum = get_checksum((uchar *)mem_layout.flash_to_ram_addr, this_entry->length);

				if (this_entry->checksum != fw_checksum && secure_mode!= RTK_SECURE_BOOT)
				{
					printf("\t FW Image checksum FAILED\n");
					printf("\t FW Image entry  checksum=0x%08x\n", this_entry->checksum);
					printf("\t FW Image result checksum=0x%08x\n", fw_checksum);
					return RTK_PLAT_ERR_READ_FW_IMG;
				}
#endif
				/* if secure mode, do AES CBC decrypt */
				if (mem_layout.bIsEncrypted)
				{
					if (secure_mode == RTK_SECURE_BOOT)
					{       
						//rtk_hexdump("the first 32-byte encrypted data", (unsigned char *)mem_layout.encrpyted_addr, 32);
						//rtk_hexdump("the last 512-byte encrypted data", (unsigned char *)(ENCRYPTED_LINUX_KERNEL_ADDR+this_entry->length-512), 512);

                        memset(ks,0x00,16);
                        memset(kh,0x00,16);
                        memset(kimg,0x00,16);

#ifdef CONFIG_CMD_KEY_BURNING
                        OTP_Get_Byte(OTP_K_S, ks, 16);
                        OTP_Get_Byte(OTP_K_H, kh, 16);
                        sync();
						flush_cache((unsigned int) ks, 16);
						flush_cache((unsigned int) kh, 16);
#endif
                        AES_ECB_encrypt(ks, 16, kimg, kh);
						flush_cache((unsigned int) kimg, 16);
                        sync();

                        Kh_key_ptr = kimg;
                        //rtk_hexdump("kimg key : ", (unsigned char *)kimg, 16);
                        Kh_key_ptr[0] = swap_endian(Kh_key_ptr[0]);
                        Kh_key_ptr[1] = swap_endian(Kh_key_ptr[1]);
                        Kh_key_ptr[2] = swap_endian(Kh_key_ptr[2]);
                        Kh_key_ptr[3] = swap_endian(Kh_key_ptr[3]);
                        //rtk_hexdump("Kh_key_ptr : ", (unsigned char *)Kh_key_ptr, 16);
						flush_cache((unsigned int) kimg, 16);
                                                
						// decrypt image
						printf("to decrypt...\n");						
						flush_cache((unsigned int) mem_layout.encrpyted_addr, this_entry->length);
						if (decrypt_image((char *)mem_layout.encrpyted_addr,
							(char *)mem_layout.decrypted_addr,
							this_entry->length,
							Kh_key_ptr))
						{
							printf("decrypt image(%d) error!\n", this_entry->type);
							return RTK_PLAT_ERR_READ_FW_IMG;
						}

						sync();
                        memset(ks,0x00,16);
                        memset(kh,0x00,16);
                        memset(kimg,0x00,16);

						//rtk_hexdump("the first 32-byte decrypted data", (unsigned char *)mem_layout.decrypted_addr, 32);

						//reverse_signature( (unsigned char *)(mem_layout.decrypted_addr + imageSize - img_truncated_size) );

						flush_cache((unsigned int) mem_layout.decrypted_addr, this_entry->length);
						ret = Verify_SHA256_hash( (unsigned char *)mem_layout.decrypted_addr,
												  this_entry->length - img_truncated_size,
												  (unsigned char *)(mem_layout.decrypted_addr + this_entry->length - img_truncated_size),
												  1 );						  
						if( ret < 0 ) {
							printf("[ERR] %s: verify hash fail(%d)\n", __FUNCTION__, ret );
							return RTK_PLAT_ERR_READ_FW_IMG;
						}

						imageSize = imageSize - img_truncated_size - SHA256_SIZE;
					}
				}

				/* if lzma type, decompress image */
				if (mem_layout.bIsCompressed)
				{
					if (lzmaBuffToBuffDecompress((uchar*)mem_layout.decompressed_addr, &decompressedSize, (uchar*)mem_layout.compressed_addr, imageSize) != 0)
					{
						printf("[ERR] %s:Decompress using LZMA error!!\n", __FUNCTION__);

						return RTK_PLAT_ERR_READ_FW_IMG;
					}
				}
			}
		}

		if (version == FW_DESC_TABLE_V1_T_VERSION_11)
		{
			v11_entry = (fw_desc_entry_v11_t*) (fw_entry + unit_len * i);

			if (v11_entry->act_size != 0)
			{
				// string format: "part_num:act_size:hash,"
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%d:%d:", v11_entry->part_num, v11_entry->act_size);
//				strncat(str_phash, buf, strlen(buf));
				memset(buf, 0, sizeof(buf));
				memcpy(buf, v11_entry->hash, sizeof(v11_entry->hash));
				buf[sizeof(v11_entry->hash)] = ',';
//				strncat(str_phash, buf, strlen(buf));
			}
		}
		else if (version == FW_DESC_TABLE_V1_T_VERSION_21)
		{
			v21_entry = (fw_desc_entry_v21_t *)this_entry;

#if defined(Config_Secure_RSA_TRUE)
			// exclude partition 0 (contain bootcode/kernel/audio/video image)
			if ( (v21_entry->part_num != PART_TYPE_RESERVED) &&
				(v21_entry->act_size != 0) ){
				// recover hash value from signature
				memset(checksum, 0, sizeof(v21_entry->RSA_sign));
				memset(signature, 0, sizeof(v21_entry->RSA_sign)+1);
				memcpy(signature, v21_entry->RSA_sign, sizeof(v21_entry->RSA_sign));

				rsa_verify(signature, Config_Secure_RSA_MODULUS, checksum);

				// string format: "part_num:act_size:hash,"
				memset(buf, 0, sizeof(buf));
				sprintf(buf, "%d:%d:%s,", v21_entry->part_num, v21_entry->act_size, checksum);
//				strncat(str_phash, buf, strlen(buf));
				//printf("[DEBUG_MSG] part_num:%x, act_size:%x, recovered hash:%s\n", v21_entry->part_num, v21_entry->act_size, checksum);
			}
#endif	/* defined(Config_Secure_RSA_TRUE) */
		}

	/* Flush caches */
	flush_dcache_all();

#endif /* CONFIG_SYS_RTK_EMMC_FLASH */
	return RTK_PLAT_ERR_OK;
}


int rtk_plat_read_all_image_from_eMMC(
		uint fw_desc_table_base, part_desc_entry_v1_t* part_entry, int part_count,
		void* fw_entry, int fw_count,
		uchar version)
{
	
	if (rtk_plat_pre_init_fw_from_eMMC(fw_desc_table_base, part_entry, part_count, fw_entry,fw_count,version) == RTK_PLAT_ERR_OK)
		return rtk_plat_read_kernel_from_eMMC(fw_desc_table_base, part_entry, part_count, fw_entry,fw_count,version);
	
	return  RTK_PLAT_ERR_READ_FW_IMG;
}
//*** EXECUTE_CUSTOMIZED_FUNCTION_4 ***//
