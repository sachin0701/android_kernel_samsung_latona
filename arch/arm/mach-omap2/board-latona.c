/*
 * Copyright (C) 2009 Texas Instruments Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Modified by Mark "Hill Beast" Kennard
 *
 * For Samsung Galaxy SL (Latona)
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio.h>
#include <linux/mtd/nand.h>

#include <asm/mach-types.h>
#include <asm/mach/arch.h>

#include "common.h"
#include <plat/board.h>
#include <plat/gpmc-latona.h>
#include <plat/usb.h>

#include <mach/board-latona.h>
#include <mach/sec_log_buf.h>
#include <mach/sec_param.h>
#include <mach/sec_common.h>
#include <mach/sec_mux.h>

#include "board-flash.h"
#include "mux.h"
#include "sdram-qimonda-hyb18m512160af-6.h"

#if defined(CONFIG_SMC91X) || defined(CONFIG_SMC91X_MODULE)

static struct omap_smc91x_platform_data board_smc91x_data = {
	.cs             = 3,
	.flags          = GPMC_MUX_ADD_DATA | IORESOURCE_IRQ_LOWLEVEL,
};

static void __init board_smc91x_init(void)
{
	board_smc91x_data.gpio_irq = 158;
	gpmc_smc91x_init(&board_smc91x_data);
}

#else

static inline void board_smc91x_init(void)
{
}

#endif /* defined(CONFIG_SMC91X) || defined(CONFIG_SMC91X_MODULE) */

static void enable_board_wakeup_source(void)
{
	/* T2 interrupt line (keypad) */
	omap_mux_init_signal("sys_nirq",
		OMAP_WAKEUP_EN | OMAP_PIN_INPUT_PULLUP);
}

static const struct usbhs_omap_board_data usbhs_bdata __initconst = {

	.port_mode[0] = OMAP_USBHS_PORT_MODE_UNUSED,
	.port_mode[1] = OMAP_EHCI_PORT_MODE_PHY,
	.port_mode[2] = OMAP_USBHS_PORT_MODE_UNUSED,

	.phy_reset  = true,
 	.reset_gpio_port[0] = -EINVAL,
 	.reset_gpio_port[1] = 64,
 	.reset_gpio_port[2] = -EINVAL,
};

static struct omap_board_config_kernel sdp_config[] __initdata = {
};

#ifdef CONFIG_OMAP_MUX
static struct omap_board_mux board_mux[] __initdata = {
	{ .reg_offset = OMAP_MUX_TERMINATOR },
};
#endif

/*
 * SDP3630 CS organization
 * See also the Switch S8 settings in the comments.
 */
static char chip_sel_sdp[][GPMC_CS_NUM] = {
	{PDC_NOR, PDC_NAND, PDC_ONENAND, DBG_MPDB, 0, 0, 0, 0}, /* S8:1111 */
	{PDC_ONENAND, PDC_NAND, PDC_NOR, DBG_MPDB, 0, 0, 0, 0}, /* S8:1110 */
	{PDC_NAND, PDC_ONENAND, PDC_NOR, DBG_MPDB, 0, 0, 0, 0}, /* S8:1101 */
};

static struct mtd_partition sdp_onenand_partitions[] = {
	{
		.name		= "X-Loader-OneNAND",
		.offset		= 0,
		.size		= 4 * (64 * 2048),
		.mask_flags	= MTD_WRITEABLE  /* force read-only */
	},
	{
		.name		= "U-Boot-OneNAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 2 * (64 * 2048),
		.mask_flags	= MTD_WRITEABLE  /* force read-only */
	},
	{
		.name		= "U-Boot Environment-OneNAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 1 * (64 * 2048),
	},
	{
		.name		= "Kernel-OneNAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= 16 * (64 * 2048),
	},
	{
		.name		= "File System-OneNAND",
		.offset		= MTDPART_OFS_APPEND,
		.size		= MTDPART_SIZ_FULL,
	},
};

static struct flash_partitions sdp_flash_partitions[] = {
	{
		.parts = sdp_onenand_partitions,
		.nr_parts = ARRAY_SIZE(sdp_onenand_partitions),
	},
};

static void __init omap_sdp_init(void)
{
	omap3_mux_init(board_mux, OMAP_PACKAGE_CBP);
	omap_board_config = sdp_config;
	omap_board_config_size = ARRAY_SIZE(sdp_config);
	latona_peripherals_init();
	omap_sdrc_init(hyb18m512160af6_sdrc_params, NULL);
	latona_display_init();
	board_smc91x_init();
	board_flash_init(sdp_flash_partitions, chip_sel_sdp, NAND_BUSWIDTH_16);
	enable_board_wakeup_source();
	usbhs_init(&usbhs_bdata);
}

MACHINE_START(LATONA, "Latona board")
	.atag_offset	= 0x100,
	.reserve	= omap_reserve,
	.map_io		= omap3_map_io,
	.init_early	= omap3630_init_early,
	.init_irq	= omap3_init_irq,
	.handle_irq	= omap3_intc_handle_irq,
	.init_machine	= omap_sdp_init,
	.timer		= &omap3_timer,
	.restart	= omap_prcm_restart,
MACHINE_END
