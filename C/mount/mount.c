#include <stdio.h>
#include <string.h>
#include "xl_common.h"
#include <mntent.h>
#define MOUNT_TAB_FILE "/proc/mounts"
int main(int argc, char *argv[])
{
    FILE * mount_table;
    struct mntent *mount_entry;
    mount_table = setmntent(MOUNT_TAB_FILE,"r");
    if (!mount_table)
    {
        printf("unable get proc mount\n");
        return 1;
    }
    
    if (argc <2 )
    {
        printf("less params ,return\n");
        return 1;
    }
    char * name = argv[1];
    printf ("get device name [%s]\n",name);

    while(1)
    {
        const char *device;
        const char *mount_point;
        const char *fs_type;
        if (mount_table)
        {
            mount_entry = getmntent(mount_table);
            if (!mount_entry)
            {
                endmntent(mount_table);
                break;
            }
        }
        
        device = mount_entry->mnt_fsname;
        mount_point = mount_entry->mnt_dir;
        fs_type = mount_entry->mnt_type;
        
        if (strcmp(name,device) == 0)
        {
            printf ("device [%s] mount_point [%s] fs_type [%s]\n",device,mount_point,fs_type);
            break;
        }
    }    
    
    return 0;
}


