// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2024, Beijing ESWIN Computing Technology Co., Ltd.. All rights reserved.
 * SPDX-License-Identifier: GPL-2.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include <common.h>
#include <blk.h>
#include <mmc.h>
#include <usb.h>
#include <fs.h>
#include <u-boot/crc.h>
#include <linux/err.h>
#include <errno.h>
#include <update_init.h>
#include <boot_ab.h>
#include <system_update.h>

static int get_misc_command(struct blk_desc *dev_desc, struct disk_partition *part_info)
{
    int ret;
    struct bootloader_message *abc = NULL;

    ret = get_bootmessage_from_disk(dev_desc, part_info, &abc);

    if(ret < 0){
        printf("UPDATE: get bootloader_message failed!\n");
        return -ENOENT;
    }
    printf("bootloader_message info\n \
            magic : 0x%08x\t command : %s\t crc32_bc : 0x%08x \n\
            kernel : boot%s \t rootfs : rootfs%s \n\
            ", abc->magic, abc->command, abc->crc32_bc,\
            ( abc->curr_bank == 0 )?"a":  ( abc->curr_bank == 1)?"b":" err", \
            ( abc->rtfs_bank == 0 )?"a":  ( abc->rtfs_bank == 1)?"b":" err");
    if (!strcmp(abc->command, "boot_normal")){
        return UPDATE_MODE_NORMAL;
    }

    if (!strcmp(abc->command, "boot_ota")){
        return UPDATE_MODE_OTA;
    }

    if (!strcmp(abc->command, "boot_sd")){
        return UPDATE_MODE_OTA_SD;
    }

    if (!strcmp(abc->command, "boot_usb")){
        return UPDATE_MODE_OTA_USB;
    }

    if (!strcmp(abc->command, "boot_recovery")){
        return UPDATE_MODE_RECOVERY;
    }

    return UPDATE_MODE_NORMAL;
}

int ota_update_init(void)
{
    int ret;
    struct blk_desc *dev_desc;
    struct disk_partition part_info;

    dev_desc = blk_get_dev(MMC_DEV_IFACE, MMC_DEV);

    if (dev_desc == NULL) {
        printf("UPDATE: Block device %s %d not supported.\n",
                MMC_DEV_IFACE, MMC_DEV);
        return -ENODEV;
    }

    if(part_get_info_by_name(dev_desc, PART_MISC, &part_info) < 0){
        printf("part_get_info_by_name failed\n");
        return -ENODEV;
    }

    ret = get_misc_command(dev_desc, &part_info);

    return ret;
}
