/*
 * ROSINSON RS-WR818 board support
 *
 * Copyright (c) 2018 ROSINSON
 * Copyright (c) 2018 Rosy Song <rosysong@rosinson.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"

#define WR818_GPIO_LED_WIFI		19
#define WR818_GPIO_LED_SYS		1

#define WR818_GPIO_BTN_RESET            2

#define WR818_KEYS_POLL_INTERVAL        20     /* msecs */
#define WR818_KEYS_DEBOUNCE_INTERVAL    (3 * WR818_KEYS_POLL_INTERVAL)

#define WR818_MAC0_OFFSET               0
#define WR818_WMAC_CALDATA_OFFSET       0x1000

static struct gpio_led WR818_leds_gpio[] __initdata = {
	{
		.name		= "wr818:red:wlan",
		.gpio		= WR818_GPIO_LED_WIFI,
		.active_low	= 1,
	},
	{
		.name		= "wr818:red:status",
		.gpio		= WR818_GPIO_LED_SYS,
		.active_low	= 1,
	},
};

static struct gpio_keys_button WR818_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WR818_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WR818_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg WR818_ar8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data WR818_ar8337_data = {
	.pad0_cfg = &WR818_ar8337_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info WR818_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &WR818_ar8337_data,
	},
};

static void __init wr818_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(WR818_leds_gpio),
				 WR818_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WR818_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(WR818_gpio_keys),
					WR818_gpio_keys);

	ath79_register_usb();

	platform_device_register(&ath79_mdio0_device);

	mdiobus_register_board_info(WR818_mdio0_info,
				    ARRAY_SIZE(WR818_mdio0_info));

	ath79_register_wmac(art + WR818_WMAC_CALDATA_OFFSET, NULL);
	ath79_register_pci();

	ath79_init_mac(ath79_eth0_data.mac_addr, art + WR818_MAC0_OFFSET, 0);

	/* GMAC0 is connected to an AR8337 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_RS_WR818, "RS-WR818", "ROSINSON RS-WR818",
	     wr818_setup);
