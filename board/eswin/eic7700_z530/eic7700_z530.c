// SPDX-License-Identifier: GPL-2.0
/*
 * EIC7700 Z530 board init
 *
 * Copyright 2024, Beijing ESWIN Computing Technology Co., Ltd.. All rights reserved.
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
 * Authors: Xiang Xu <xuxiang@eswincomputing.com>
 */

#include <common.h>
#include <dm.h>
#include <linux/delay.h>
#include <linux/io.h>
#include "env.h"
#include <errno.h>
#include <blk.h>
#include <mmc.h>
#include <net.h>
#include <dm/uclass.h>
#include <u-boot/crc.h>
#include <i2c.h>
#include <spi.h>
#include <spi_flash.h>
#include <init.h>
#include <asm/gpio.h>
#include <dm/device-internal.h>
#include <dm/pinctrl.h>
#include <eswin/cpu.h>
#ifdef CONFIG_ESWIN_UMBOX
#include <eswin/eswin-umbox-srvc.h>
#endif

int set_voltage_default(void)
{
	ofnode node;
	struct udevice *pinctrl;
	struct gpio_desc desc;

	node = ofnode_path("/config");
	if (!ofnode_valid(node)) {
		pr_err("Can't find /config node!\n");
		return -EINVAL;
	}
	if(uclass_get_device(UCLASS_PINCTRL, 0, &pinctrl)) {
		debug("%s: Cannot find pinctrl device\n", __func__);
		return -EINVAL;
	}
	if(pinctrl_select_state(pinctrl, "default")) {
		printf("Failed to set pinctrl state: %d\n", pinctrl_select_state(pinctrl, "default"));
		return -EINVAL;
	}
	if(gpio_request_by_name_nodev(node, "power-gpios", 0, &desc,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE)) {
		pr_err("Can't request  \"power-gpios\" !\n");
		return -EINVAL;
	}
	dm_gpio_set_value(&desc, 0);
	return 0;
}

int misc_init_r(void)
{
	struct udevice *dev;

	set_voltage_default();

#ifdef CONFIG_ESWIN_PMP
	eswin_pmp_init();
#endif

#if defined(CONFIG_ESWIN_SPI)
	es_bootspi_write_protection_init();
#endif

	uclass_get_device_by_name(UCLASS_VIDEO, "display-subsystem", &dev);

	env_set_ulong("ram_size", (gd->ram_size / 1024 / 1024 / 1024));
	eswin_update_bootargs();
	return 0;
}

void irq_mux_route(void)
{
	unsigned int val;

	/* Route all interrupts from default LPCPU/SCPU to MCPU
	 * I2C0 bit16
	 * I2C1 bit15
	 * RTC  bit14~13
	 * GPIO bit12
	 * SPI  bit11~10
	 * DMA  bit9
	 * MPMP bit8
	 * TIMER0 bit7~6
	 * TIMER1 bit5~4
	 * TIMER2 bit3~2
	 * TIMER3 bit1~0
	*/

	val = 0;
	writel(val,(void *)(0x51810000+0x3c0));
}

int board_init(void)
{
	/* For now nothing to do here. */
	irq_mux_route();
	return 0;
}

int board_late_init(void)
{
#ifdef CONFIG_ESWIN_UMBOX
	lpcpu_misc_func();
#endif
	return 0;
}
