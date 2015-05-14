#include <stdio.h>
int hotplug_block(void)
{
	char *action = NULL, *minor = NULL;
	char *major = NULL, *driver = NULL;
	int minor_no, major_no, device, part;
	int err;
	int retry = 3, lock_fd = -1;
	char cmdbuf[64] = {0};
	char mntdev[32] = {0};
	char mntpath[32] = {0};
	char devname[10] = {0};

	char *physdevpath = NULL;
	char usb_devname[10] = {0};
	int tmp_num = -1;
	int usb_port_num = -1;
	
	struct flock lk_info = {0};

	if (!(action = getenv("ACTION")) ||
	    !(minor = getenv("MINOR")) ||
	    !(driver = getenv("PHYSDEVDRIVER")) ||
	    !(major = getenv("MAJOR")) ||
		!(physdevpath  = getenv("PHYSDEVPATH"))
	)
	{
		return EINVAL;
	}

	hotplug_dbg("env %s %s!\n", action, driver);
	if (strncmp(driver, "sd", 2))
	{
		return EINVAL;
	}

	if ((lock_fd = open(LOCK_FILE, O_RDWR|O_CREAT, 0666)) < 0) {
		hotplug_dbg("Failed opening lock file LOCK_FILE: %s\n", strerror(errno));
		return -1;
	}

	while (--retry) {
		lk_info.l_type = F_WRLCK;
		lk_info.l_whence = SEEK_SET;
		lk_info.l_start = 0;
		lk_info.l_len = 0;
		if (!fcntl(lock_fd, F_SETLKW, &lk_info)) break;
	}

	if (!retry) {
		hotplug_dbg("Failed locking LOCK_FILE: %s\n", strerror(errno));
		return -1;
	}


	/*TODO: Add by sdh 2013-4-18, for distinguishing usb port mount*/
	/*
	The usb mount infomation:
	usbPort /devices/pci0000:00/0000:00:0a.1/usb1/1-2/1-2:1.0/host5/target5:0:0/5:0:0:0
	usbPort /devices/pci0000:00/0000:00:0a.1/usb1/1-1/1-1:1.0/host4/target4:0:0/4:0:0:0
	hotplug detected usb 3-1:1.0 :  125f/312b/a00  port:1*/
	
	physdevpath = strstr(physdevpath, "usb");
	sscanf(physdevpath, "usb%d/%d-%d", &tmp_num, &tmp_num, &usb_port_num);

	/*get usb port(1 or 2)*/
	if(physdevpath && tmp_num != -1 &&  usb_port_num != -1){
		sprintf(usb_devname, "usb%d", usb_port_num);
		cprintf("get usb port number = %s \n", usb_devname);
	}
	else{
		hotplug_dbg("get bad usb port number!\n");
		return -1;
	}
	/*TODO:End by sdh*/
	
	major_no = atoi(major);
	minor_no = atoi(minor);
	device = minor_no/16;
	part = minor_no%16;

	sprintf(devname, "%s%c%d", driver, 'a' + device, part);
	sprintf(mntdev, "/dev/%s", devname);
	sprintf(mntpath, "/media/%s/%s", usb_devname, devname);
	if (!strcmp(action, "add")) {
		if ((devname[2] > 'd') || (devname[2] < 'a')) {
			hotplug_dbg("bad dev!\n");
			goto exit;
		}

		hotplug_dbg("adding disk...\n");

		err = mknod(mntdev, S_IRWXU|S_IFBLK, makedev(major_no, minor_no));
		hotplug_dbg("err = %d\n", err);

		//err = mkdir(mntpath, 0777);
		sprintf(cmdbuf,"mkdir -p %s",mntpath);//downloader
		system(cmdbuf);
		chmod(mntpath, 0777);
		hotplug_dbg("err %s= %s\n", mntpath, strerror(errno));
		
		/*try to mount vfat format*/
		sprintf(cmdbuf, "mount -t vfat %s %s", mntdev, mntpath);
		err = system(cmdbuf);
		hotplug_dbg("err = %d\n", err);

		/*try to mount ntfs format*/
		if (err) {
			sprintf(cmdbuf, "ntfs-3g %s %s", mntdev, mntpath);
			err = system(cmdbuf);
		}

	
		if (err) {
			hotplug_dbg("unsuccess %d!\n", err);
			unlink(mntdev);
			rmdir(mntpath);
           
		}
		else {
            //modify by linming
            save_usbdirs(usb_devname,devname,"add");
            tw_nvram_set_force("usb_existent","1");
            tw_Downloader_Init_File_list(NULL, 0);

            /* Start usb services */
			usb_start_services();
			
			/* Optimize performance */
			optimize_block_device(devname);
		}
#ifdef __CONFIG_SAMBA__		
		//μ±óDu?ìéè±?2?è?ê±￡?óéóú???????t?é?üóD?üD?￡?Dèòasamba????￡???D??áè????????t/tmp/samba/lib/smb.conf
		stop_samba();
		start_samba();
#endif
	} 
    else if (!strcmp(action, "remove")) {
		/* Stop usb services */
		//usb_stop_services();

		hotplug_dbg("removing disk %s/%s...\n", usb_devname, devname);
		save_usbdirs(usb_devname,devname,"remove");
		sprintf(cmdbuf, UMOUNT_CMD, mntpath);
		err = system(cmdbuf);
		memset(cmdbuf, 0, sizeof(cmdbuf));
		unlink(mntdev);
		rmdir(mntpath);
		hotplug_usb_power_recover();
	} else {
		hotplug_dbg("not support action!\n");
	}

exit:
	close(lock_fd);
	unlink(LOCK_FILE);
	return 0;
}