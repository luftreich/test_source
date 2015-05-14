#include <linux/device.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/stat.h>
 
MODULE_AUTHOR("Vincent");
MODULE_LICENSE("Dual BSD/GPL");

void obj_test_release(struct kobject *kobject);
ssize_t vincent_test_show(struct kobject *kobject, struct attribute *attr,char *buf);
ssize_t vincent_test_store(struct kobject *kobject,struct attribute *attr,const char *buf, size_t count);

struct attribute test_attr = {
    .name = "vincent_han",
    .mode = S_IRWXUGO,
};
 
static struct attribute *def_attrs[] = {
    &test_attr,
    NULL,
};
 
 
struct sysfs_ops obj_test_sysops =
{
    .show = vincent_test_show,
    .store = vincent_test_store,
};
 
struct kobj_type ktype =
{
    .release = obj_test_release,
    .sysfs_ops=&obj_test_sysops,
    .default_attrs=def_attrs,
};
 
void obj_test_release(struct kobject *kobject)
{
    printk("vincent_test: release .\n");
}
 
ssize_t vincent_test_show(struct kobject *kobject, struct attribute *attr,char *buf)
{
    printk("have show.\n");
    printk("attrname:%s.\n", attr->name);
    sprintf(buf,"%s\n",attr->name);
    return strlen(attr->name)+2;
}
 
ssize_t vincent_test_store(struct kobject *kobject,struct attribute *attr,const char *buf, size_t count)
{
    printk("havestore\n");
    printk("write: %s\n",buf);
    return count;
}
 
struct kobject kobj;
static int kobject_test_init()
{
    printk("kboject test init.\n");
    kobject_init_and_add(&kobj,&ktype,NULL,"vincent_test");
    return 0;
}
 
static int kobject_test_exit()
{
    printk("kobject test exit.\n");
    kobject_del(&kobj);
    return 0;
}

module_init(kobject_test_init);
module_exit(kobject_test_exit);

