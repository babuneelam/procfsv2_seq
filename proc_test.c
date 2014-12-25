
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <asm/uaccess.h>

#define PROC_TEST_DIR "test"
#define PROC_TEST_CONFIG_FILE "config"

/**
 * This structure hold information about the /proc file
 *
 */
struct proc_dir_entry *proc_test_dir, *proc_test_config;

#define CONFIG_BUFF_MAX_SIZE 100
#define MAX_CONFIG_ENTRIES 4
static char proc_test_config_buff[CONFIG_BUFF_MAX_SIZE] = "0 0 0 0";
static char proc_test_config_buff_format[] = "Format: \n"
    "   whether data sould be stored in a hash table\n"
    "   Max Hash table size\n"
    "   whether to log statistics \n"
    "   Max number of statistics to be stored \n";

static int use_hash_tbl=0, max_hash_tbl_size=0, log_stats=0, max_stats=0;

/* Tmp variable to sanitize user input config data */
static char proc_test_config_buff_tmp[CONFIG_BUFF_MAX_SIZE] = "0 0 0 0";
static int use_hash_tbl_tmp=0, max_hash_tbl_size_tmp=0, log_stats_tmp=0,
           max_stats_tmp=0;

#define MAX_CONFIG_ENTRIES2 6
static int proc_test_config_buff2[MAX_CONFIG_ENTRIES2] = {20,21,22,23,24,25};


static int proc_test_config_open(struct inode *, struct file *);
static ssize_t proc_test_config_write(struct file *,
        const char __user *, size_t , loff_t *);

static struct file_operations proc_test_config_file_ops = {
        .owner = THIS_MODULE,
        .open = proc_test_config_open,
        .read = seq_read,
        .write = proc_test_config_write,
        .llseek = seq_lseek,
        .release = seq_release,
};

static void *proc_test_config_start(struct seq_file *, loff_t *);
static void *proc_test_config_next(struct seq_file *, void *, loff_t *);
static void proc_test_config_stop(struct seq_file *, void *);
static int proc_test_config_read(struct seq_file *, void *);

static struct seq_operations proc_test_config_seq_op = {
        .start =        proc_test_config_start,
        .next =         proc_test_config_next,
        .stop =         proc_test_config_stop,
        .show =         proc_test_config_read
};


static int proc_test_config_open(struct inode *inode, struct file *file)
{
        return seq_open(file, &proc_test_config_seq_op);
}


/**
 * This function is called at the beginning of a sequence.
 * ie, when:
 *      - the /proc file is read (first time)
 *      - after the function stop (end of sequence)
 *
 */

static void *proc_test_config_start(struct seq_file *m, loff_t * pos)
{
        static int cur_array_num=1, entry_num = 0;

        /* beginning a new sequence ? */
        if (*pos == 0) {
//printk("seq_start invoked at beg of first sequence \r\n");
                m->private = (void *)&cur_array_num;
                seq_printf(m, "\n\nConfig 1 Info:\n");
                seq_printf(m, "----------------------------\n");
                /* yes => return a non null value to begin the sequence */
                return &entry_num;
        } else if (*pos == 1) {
//printk("\n\nseq_start invoked at beg of second sequence \r\n");
                cur_array_num++;
                entry_num=0;
                m->private = (void *)&cur_array_num;
                seq_printf(m, "\n\nConfig 2 Info:\n");
                seq_printf(m, "----------------------------\n");
                /* yes => return a non null value to begin the sequence */
                return &entry_num;
        } else {
                /* no => it's the end of the sequence, return end to stop reading */
//printk("seq_start invoked at end of sequence \r\n");
                *pos = 0;
                return NULL;
        }
}

/**
 * This function is called after the beginning of a sequence.
 * It's called untill the return is NULL (this ends the sequence).
 *
 */
static void *proc_test_config_next(struct seq_file *m, void *entry_num,
        loff_t * pos)
{
    int cur_array_num=*((int *)m->private);

    if (cur_array_num == 1)
    {
//printk("seq_next invoked in first sequence \r\n");
        *pos = 1;
        return NULL;            /* all data is displayed in one chunk */
    } else if (cur_array_num == 2) {
//printk("seq_next invoked in second sequence, entry num = %d \r\n", *((int *)entry_num));
        if (*((int *)entry_num) < MAX_CONFIG_ENTRIES2)
        {
            (*(int *)entry_num)++;
            return entry_num;
        } else
            return NULL;
    }

    return NULL;
}

static void proc_test_config_stop(struct seq_file *m, void *entry_num)
{
    if (*((int *)m->private) == 1)
    {
//printk("seq_stop invoked in first sequence \r\n");
        return;
    } else if (*((int *)m->private) == 2) {
//printk("seq_stop invoked in second sequence");
        seq_printf(m, "\n\n ");
        return;
    }
}

static int proc_test_config_read(struct seq_file *m, void *entry_num)
{
    int cur_array_num=*((int *)m->private);

//printk("seq_show invoked in sequence %d \r\n", cur_array_num);

    if (cur_array_num == 1)
    {
//printk("seq_show invoked in first sequence \r\n");
        seq_printf(m, "\n%s \n\n%s \n\n", proc_test_config_buff,
                proc_test_config_buff_format);
    } else if (cur_array_num == 2) {
//printk("seq_show invoked in second sequence \r\n");
        if (*((int*)entry_num) < MAX_CONFIG_ENTRIES2)
            seq_printf(m, "%d ", proc_test_config_buff2[*((int *)entry_num)]);
    }

    return 0;
}

static int isdigit(char c) {
        return ((c >= '0') && (c <= '9'));
}

static ssize_t proc_test_config_write(struct file *file,
        const char __user *buffer, size_t size, loff_t *ppos)
{
    char *buff, *buff1, **end_ptr = NULL;
    unsigned int i;

    if (size > CONFIG_BUFF_MAX_SIZE)
        return 0;

    copy_from_user(proc_test_config_buff_tmp, buffer, size);
    proc_test_config_buff_tmp[size] = '\0';

    buff = proc_test_config_buff_tmp;
    for (i=0; i< MAX_CONFIG_ENTRIES; i++)
    {
        buff1 = buff;
        while (isdigit(*buff))
                buff++;
        if (*buff != '\n') {
                if (*buff == ' ')
                        buff++;
                else
                        return 0;
        }

        switch (i) {
            case 1:
                use_hash_tbl_tmp = simple_strtoul(buff1, end_ptr, 10);
                break;
            case 2:
                max_hash_tbl_size_tmp = simple_strtoul(buff1, end_ptr, 10);
                break;
            case 3:
                log_stats_tmp = simple_strtoul(buff1, end_ptr, 10);
                break;
            case 4:
                max_stats_tmp = simple_strtoul(buff1, end_ptr, 10);
                break;
        }
    }
    *buff = '\0'; /* Ignoring any additional input user may have provided */


    /* Copy to the real config buffer & variables */
    strcpy(proc_test_config_buff, proc_test_config_buff_tmp);
    use_hash_tbl = use_hash_tbl_tmp;
    max_hash_tbl_size = max_hash_tbl_size_tmp;
    log_stats = log_stats_tmp;
    max_stats = max_stats_tmp;

    return size;
}

int proc_test_init(void)
{

        /*Create a new directory named PROC_TEST_DIR in /proc */
        proc_test_dir = proc_mkdir(PROC_TEST_DIR, NULL);
        if (!proc_test_dir)
        {
            printk("PROC_TEST_DIR creation failed.. Exiting. \r\n");
            return -1;
        }

        /* Create a new proc file in PROC_TEST_DIR in "rw- r-- r--" mode*/
        proc_test_config = proc_create(PROC_TEST_CONFIG_FILE, 0644,
                                proc_test_dir, &proc_test_config_file_ops);

        if (proc_test_dir == NULL) {
                printk("Could not create /proc/%s/%s\n", PROC_TEST_DIR,
                       PROC_TEST_CONFIG_FILE);
                remove_proc_entry(PROC_TEST_DIR, NULL);
                return -1;
        }

        printk("/proc/%s/%s created\n",
               PROC_TEST_DIR, PROC_TEST_CONFIG_FILE);

        return 0;
}

static void proc_test_exit(void)
{
        printk("proc test module de-init: start");
        //proc_remove(proc_test_config);
        //proc_remove(proc_test_dir);
        remove_proc_entry(PROC_TEST_CONFIG_FILE, proc_test_dir);
        remove_proc_entry(PROC_TEST_DIR, NULL);
        printk("proc test module de-init: iend");
}

module_init(proc_test_init);
module_exit(proc_test_exit);

MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Babu");

