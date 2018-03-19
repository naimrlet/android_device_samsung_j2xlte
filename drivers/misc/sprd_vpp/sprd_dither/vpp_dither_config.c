/*
 * Copyright (C) 2014 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/math64.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/irq.h>
#include <linux/kthread.h>
#include <linux/io.h>
#include <linux/pid.h>

#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_fdt.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/device.h>
#endif

#include <mach/hardware.h>
#include <mach/arch_misc.h>
#include <mach/sci.h>
#include <video/ion_sprd.h>

#ifdef CONFIG_HAS_EARLYSUSPEND_GSP
#include <linux/earlysuspend.h>
#endif

#include <mach/hardware.h>
#include "vpp_dither_types.h"
#include "vpp_dither_config.h"
#include "vpp_dither_reg.h"


extern struct vpp_dither_device *vpp_dither_ctx;

void VPP_Dither_Early_Init()
{
	const unsigned char dither_path_table[DITHER_PATH_SIZE] =
	{
		0x00,0x01,0x11,0x10,0x20,0x30,0x31,0x21,0x22,0x32,0x33,0x23,0x13,0x12,0x02,0x03,0x04,0x14,0x15,0x05,0x06,0x07,0x17,0x16,0x26,0x27,0x37,0x36,0x35,0x25,0x24,0x34,
		0x44,0x54,0x55,0x45,0x46,0x47,0x57,0x56,0x66,0x67,0x77,0x76,0x75,0x65,0x64,0x74,0x73,0x72,0x62,0x63,0x53,0x43,0x42,0x52,0x51,0x41,0x40,0x50,0x60,0x61,0x71,0x70,
		0x80,0x90,0x91,0x81,0x82,0x83,0x93,0x92,0xa2,0xa3,0xb3,0xb2,0xb1,0xa1,0xa0,0xb0,0xc0,0xc1,0xd1,0xd0,0xe0,0xf0,0xf1,0xe1,0xe2,0xf2,0xf3,0xe3,0xd3,0xd2,0xc2,0xc3,
		0xc4,0xc5,0xd5,0xd4,0xe4,0xf4,0xf5,0xe5,0xe6,0xf6,0xf7,0xe7,0xd7,0xd6,0xc6,0xc7,0xb7,0xa7,0xa6,0xb6,0xb5,0xb4,0xa4,0xa5,0x95,0x94,0x84,0x85,0x86,0x96,0x97,0x87,
		0x88,0x98,0x99,0x89,0x8a,0x8b,0x9b,0x9a,0xaa,0xab,0xbb,0xba,0xb9,0xa9,0xa8,0xb8,0xc8,0xc9,0xd9,0xd8,0xe8,0xf8,0xf9,0xe9,0xea,0xfa,0xfb,0xeb,0xdb,0xda,0xca,0xcb,
		0xcc,0xcd,0xdd,0xdc,0xec,0xfc,0xfd,0xed,0xee,0xfe,0xff,0xef,0xdf,0xde,0xce,0xcf,0xbf,0xaf,0xae,0xbe,0xbd,0xbc,0xac,0xad,0x9d,0x9c,0x8c,0x8d,0x8e,0x9e,0x9f,0x8f,
		0x7f,0x7e,0x6e,0x6f,0x5f,0x4f,0x4e,0x5e,0x5d,0x4d,0x4c,0x5c,0x6c,0x6d,0x7d,0x7c,0x7b,0x6d,0x6a,0x7a,0x79,0x78,0x68,0x69,0x59,0x58,0x48,0x49,0x4a,0x5a,0x5b,0x4b,
		0x3b,0x2b,0x2a,0x3a,0x39,0x38,0x28,0x29,0x19,0x18,0x08,0x09,0x0a,0x1a,0x1b,0x0b,0x0c,0x0d,0x1d,0x1c,0x2c,0x3c,0x3d,0x2d,0x2e,0x3e,0x3f,0x2f,0x1f,0x1e,0x0e,0x0f,
		0xff,0xef,0xee,0xfe,0xfd,0xfc,0xec,0xed,0xdd,0xdc,0xcc,0xcd,0xce,0xde,0xdf,0xcf,0xbf,0xbe,0xae,0xaf,0x9f,0x8f,0x8e,0x9e,0x9d,0x8d,0x8c,0x9c,0xac,0xad,0xbd,0xbc,
		0xbb,0xba,0xaa,0xab,0x9b,0x8b,0x8a,0x9a,0x99,0x89,0x88,0x98,0xa8,0xa9,0xb9,0xb8,0xc8,0xd8,0xd9,0xc9,0xca,0xcb,0xdb,0xda,0xea,0xeb,0xfb,0xfa,0xf9,0xe9,0xe8,0xf8,
		0xf7,0xf6,0xe6,0xe7,0xd7,0xc7,0xc6,0xd6,0xd5,0xc5,0xc4,0xd4,0xe4,0xe5,0xf5,0xf4,0xf3,0xe3,0xe2,0xf2,0xf1,0xf0,0xe0,0xe1,0xd1,0xd0,0xc0,0xc1,0xc2,0xd2,0xd3,0xc3,
		0xb3,0xa3,0xa2,0xb2,0xb1,0xb0,0xa0,0xa1,0x91,0x90,0x80,0x81,0x82,0x92,0x93,0x83,0x84,0x85,0x95,0x94,0xa4,0xb4,0xb5,0xa5,0xa6,0xb6,0xb7,0xa7,0x97,0x96,0x86,0x87,
		0x77,0x76,0x66,0x67,0x57,0x47,0x46,0x56,0x55,0x45,0x44,0x54,0x64,0x65,0x75,0x74,0x73,0x63,0x62,0x72,0x71,0x70,0x60,0x61,0x51,0x50,0x40,0x41,0x42,0x52,0x53,0x43,
		0x33,0x23,0x22,0x32,0x31,0x30,0x20,0x21,0x11,0x10,0x00,0x01,0x02,0x12,0x13,0x03,0x04,0x05,0x15,0x14,0x24,0x34,0x35,0x25,0x26,0x36,0x37,0x27,0x17,0x16,0x06,0x07,
		0x08,0x18,0x19,0x09,0x0a,0x0b,0x1b,0x1a,0x2a,0x2b,0x3b,0x3a,0x39,0x29,0x28,0x38,0x48,0x49,0x59,0x58,0x68,0x78,0x79,0x69,0x6a,0x7a,0x7b,0x6b,0x5b,0x5a,0x4a,0x4b,
		0x4c,0x4d,0x5d,0x5c,0x6c,0x7c,0x7d,0x6d,0x6e,0x7e,0x7f,0x6f,0x5f,0x5e,0x4e,0x4f,0x3f,0x2f,0x2e,0x3e,0x3d,0x3c,0x2c,0x2d,0x1d,0x1c,0x0c,0x0d,0x0e,0x1e,0x1f,0x0f,
		0xff,0xfe,0xee,0xef,0xdf,0xcf,0xce,0xde,0xdd,0xcd,0xcc,0xdc,0xec,0xed,0xfd,0xfc,0xfb,0xeb,0xea,0xfa,0xf9,0xf8,0xe8,0xe9,0xd9,0xd8,0xc8,0xc9,0xca,0xda,0xdb,0xcb,
		0xbb,0xab,0xaa,0xba,0xb9,0xb8,0xa8,0xa9,0x99,0x98,0x88,0x89,0x8a,0x9a,0x9b,0x8b,0x8c,0x8d,0x9d,0x9c,0xac,0xbc,0xbd,0xad,0xae,0xbe,0xbf,0xaf,0x9f,0x9e,0x8e,0x8f,
		0x7f,0x6f,0x6e,0x7e,0x7d,0x7c,0x6c,0x6d,0x5d,0x5c,0x4c,0x4d,0x4e,0x5e,0x5f,0x4f,0x3f,0x3e,0x2e,0x2f,0x1f,0x0f,0x0e,0x1e,0x1d,0x0d,0x0c,0x1c,0x2c,0x2d,0x3d,0x3c,
		0x3b,0x3a,0x2a,0x2b,0x1b,0x0b,0x0a,0x1a,0x19,0x09,0x08,0x18,0x28,0x29,0x39,0x38,0x48,0x58,0x59,0x49,0x4a,0x4b,0x5b,0x5a,0x6a,0x6b,0x7b,0x7a,0x79,0x69,0x68,0x78,
		0x77,0x67,0x66,0x76,0x75,0x74,0x64,0x65,0x55,0x54,0x44,0x45,0x46,0x56,0x57,0x47,0x37,0x36,0x26,0x27,0x17,0x07,0x06,0x16,0x15,0x05,0x04,0x14,0x24,0x25,0x35,0x34,
		0x33,0x32,0x22,0x23,0x13,0x03,0x02,0x12,0x11,0x01,0x00,0x10,0x20,0x21,0x31,0x30,0x40,0x50,0x51,0x41,0x42,0x43,0x53,0x52,0x62,0x63,0x73,0x72,0x71,0x61,0x60,0x70,
		0x80,0x81,0x91,0x90,0xa0,0xb0,0xb1,0xa1,0xa2,0xb2,0xb3,0xa3,0x93,0x92,0x82,0x83,0x84,0x94,0x95,0x85,0x86,0x87,0x97,0x96,0xa6,0xa7,0xb7,0xb6,0xb5,0xa5,0xa4,0xb4,
		0xc4,0xd4,0xd5,0xc5,0xc6,0xc7,0xd7,0xd6,0xe6,0xe7,0xf7,0xf6,0xf5,0xe5,0xe4,0xf4,0xf3,0xf2,0xe2,0xe3,0xd3,0xc3,0xc2,0xd2,0xd1,0xc1,0xc0,0xd0,0xe0,0xe1,0xf1,0xf0,
		0x00,0x10,0x11,0x01,0x02,0x03,0x13,0x12,0x22,0x23,0x33,0x32,0x31,0x21,0x20,0x30,0x40,0x41,0x51,0x50,0x60,0x70,0x71,0x61,0x62,0x72,0x73,0x63,0x53,0x52,0x42,0x43,
		0x44,0x45,0x55,0x54,0x64,0x74,0x75,0x65,0x66,0x76,0x77,0x67,0x57,0x56,0x46,0x47,0x37,0x27,0x26,0x36,0x35,0x34,0x24,0x25,0x15,0x14,0x04,0x05,0x06,0x16,0x17,0x07,
		0x08,0x09,0x19,0x18,0x28,0x38,0x39,0x29,0x2a,0x3a,0x3b,0x2b,0x1b,0x1a,0x0a,0x0b,0x0c,0x1c,0x1d,0x0d,0x0e,0x0f,0x1f,0x1e,0x2e,0x2f,0x3f,0x3e,0x3d,0x2d,0x2c,0x3c,
		0x4c,0x5c,0x5d,0x4d,0x4e,0x4f,0x5f,0x5e,0x6e,0x6f,0x7f,0x7e,0x7d,0x6d,0x6c,0x7c,0x7b,0x7a,0x6a,0x6b,0x5b,0x4b,0x4a,0x5a,0x59,0x49,0x48,0x58,0x68,0x69,0x79,0x78,
		0x88,0x89,0x99,0x98,0xa8,0xb8,0xb9,0xa9,0xaa,0xba,0xbb,0xab,0x9b,0x9a,0x8a,0x8b,0x8c,0x9c,0x9d,0x8d,0x8e,0x8f,0x9f,0x9e,0xae,0xaf,0xbf,0xbe,0xbd,0xad,0xac,0xbc,
		0xcc,0xdc,0xdd,0xcd,0xce,0xcf,0xdf,0xde,0xee,0xef,0xff,0xfe,0xfd,0xed,0xec,0xfc,0xfb,0xfa,0xea,0xeb,0xdb,0xcb,0xca,0xda,0xd9,0xc9,0xc8,0xd8,0xe8,0xe9,0xf9,0xf8,
		0xf7,0xe7,0xe6,0xf6,0xf5,0xf4,0xe4,0xe5,0xd5,0xd4,0xc4,0xc5,0xc6,0xd6,0xd7,0xc7,0xb7,0xb6,0xa6,0xa7,0x97,0x87,0x86,0x96,0x95,0x85,0x84,0x94,0xa4,0xa5,0xb5,0xb4,
		0xb3,0xb2,0xa2,0xa3,0x93,0x83,0x82,0x92,0x91,0x81,0x80,0x90,0xa0,0xa1,0xb1,0xb0,0xc0,0xd0,0xd1,0xc1,0xc2,0xc3,0xd3,0xd2,0xe2,0xe3,0xf3,0xf2,0xf1,0xe1,0xe0,0xf0
	};

	sci_glb_set(REG_AON_APB_APB_EB0, BIT(25));	//enable MM module
	sci_glb_set(REG_MM_AHB_AHB_EB, BIT(8));		//enable VPP module
	sci_glb_set(REG_MM_AHB_GEN_CKG_CFG, BIT(9));	//enable VPP AXI

	TB_REG_SET(DITH_MEM_SW, 0x00);
	memcpy(DITH_PATH_TABLE, &dither_path_table, DITHER_PATH_SIZE);

	TB_REG_SET(RB_COEF_CFG0, 0x0);
	TB_REG_SET(RB_COEF_CFG1, 0x0);
	TB_REG_SET(RB_COEF_CFG2, 0xC0C0B0B0);
	TB_REG_SET(RB_COEF_CFG3, 0xE0D0D0C0);
	TB_REG_SET(G_COEF_CFG0, 0x0);
	TB_REG_SET(G_COEF_CFG1, 0x0);
	TB_REG_SET(G_COEF_CFG2, 0xC0C0B0B0);
	TB_REG_SET(G_COEF_CFG3, 0xE0D0D0C0);
	TB_REG_SET(DITH_MEM_SW, 0x01);

	VPP_Dither_IRQENABLE_SET(0);
	VPP_Dither_IRQSTATUS_Clear();
	VPP_Dither_IRQENABLE_SET(1);
}
#if 0
int VPP_Dither_Clock_Init(struct vpp_dither_device *dev)
{
	int ret = 0;
	struct clk *clk_mm_i;
	struct clk *clk_vpp_i;
	struct clk *clk_vpp_parent_i;

#ifdef CONFIG_OF
	clk_mm_i = of_clk_get_by_name(dev->of_dev->of_node, "clk_mm_i");
#else
	clk_mm_i = clk_get(NULL, "clk_mm_i");
#endif
	if(IS_ERR(clk_mm_i))
	{
		printk(KERN_ERR "[vpp_dither] can't get clock [%s]\n", "clk_mm_i");
		ret = -EINVAL;
		goto out;
	}
	else
	{
		dev->clk_mm = clk_mm_i;
		printk(KERN_INFO "[vpp_dither] get clk_mm ok!\n");
	}

	ret = clk_enable(dev->clk_mm);
	if(ret)
	{
		printk(KERN_ERR "[vpp_dither] mm_clk:clk_enable failed!\n");
		return ret;
	}

#ifdef CONFIG_OF
	clk_vpp_i = of_clk_get_by_name(dev->of_dev->of_node, "clk_vpp");
#else
	clk_vpp_i = clk_get(NULL, "clk_vpp");
#endif
	if(IS_ERR(clk_vpp_i))
	{
		printk(KERN_ERR "[vpp_dither] can't get clock [%s]\n", "clk_vpp");
		ret = -EINVAL;
		goto out;
	}
	else
	{
		dev->clk_vpp = clk_vpp_i;
		printk(KERN_INFO "[vpp_dither] get clk_vpp ok!\n");
	}

	clk_vpp_parent_i = clk_get(NULL, "clk_256m");
	if(IS_ERR(clk_vpp_parent_i))
	{
		printk(KERN_ERR "[vpp_dither] can't get clock [%s]\n", "clk_vpp_parent");
		ret = -EINVAL;
		goto out;
	}
	else
	{
		dev->clk_vpp_parent = clk_vpp_parent_i;
		printk(KERN_INFO "[vpp_dither] get clk_vpp_parent ok!\n");
	}

	ret = clk_set_parent(dev->clk_vpp, dev->clk_vpp_parent);
	if(ret)
	{
		printk(KERN_ERR "[vpp_dither] clk_set_parent failed!\n");
		ret = -EINVAL;
		goto out;
	}

	return 0;

out:
	if(dev->clk_mm)
		clk_put(dev->clk_mm);

	if(dev->clk_vpp)
		clk_put(dev->clk_vpp);

	if(dev->clk_vpp_parent)
		clk_put(dev->clk_vpp_parent);

	return ret;
}

void VPP_Dither_Clock_Enable(struct vpp_dither_device *dev)
{
	int ret = 0;

	if(dev->clk_vpp == NULL)
	{
		printk(KERN_ERR "[vpp_dither] clk not init yet!\n");
		return;
	}

	ret = clk_prepare_enable(dev->clk_vpp);
	if(ret)
	{
		printk(KERN_ERR "[vpp_dither] enable clock fail!\n");
		return;
	}
}

void VPP_Dither_Clock_Disable(struct vpp_dither_device *dev)
{
	if(dev->clk_vpp == NULL)
	{
		printk(KERN_ERR "[vpp_dither] clk not init yet!\n");
		return;
	}

	clk_disable_unprepare(dev->clk_vpp);
}
#endif
void VPP_Dither_Module_Enable()
{
	unsigned int reg_value = 0;

	reg_value = TB_REG_GET(VPP_CFG);
	reg_value |= 0x01;
	TB_REG_SET(VPP_CFG, reg_value);
}

void VPP_Dither_Module_Disable()
{
	unsigned int reg_value = 0;

	reg_value = TB_REG_GET(VPP_CFG);
	reg_value &= 0xFFFFFFFE;
	TB_REG_SET(VPP_CFG, reg_value);
}

void VPP_Dither_IRQSTATUS_Clear()
{
	TB_REG_SET(VPP_INT_CLR, 0x01);
}

void VPP_Dither_IRQENABLE_SET(unsigned char irq_flag)
{
	unsigned int reg_value = 0;

	reg_value = TB_REG_GET(VPP_INT_MSK);

	if(irq_flag == 1)
		reg_value |= 0x01;
	else
		reg_value &= 0xFFFFFFFE;

	TB_REG_SET(VPP_INT_MSK, reg_value);
}

void VPP_Dither_Init(struct vpp_dither_device *dev)
{
	//clock enable
//	VPP_Dither_Clock_Enable(dev);

	//module enable
	VPP_Dither_Module_Enable();

	//irq enable
//	VPP_Dither_IRQENABLE_SET(0);
//	VPP_Dither_IRQSTATUS_Clear();
//	VPP_Dither_IRQENABLE_SET(1);
}

void VPP_Dither_Deinit(struct vpp_dither_device *dev)
{
	//module disable
	VPP_Dither_Module_Disable();

	//irq disable
//	VPP_Dither_IRQENABLE_SET(0);
//	VPP_Dither_IRQSTATUS_Clear();

	//clock disable
//	VPP_Dither_Clock_Disable(dev);
}

void VPP_Dither_Module_Reset()
{
	sci_glb_set(REG_MM_AHB_AHB_RST, BIT(15));
	udelay(10);
	sci_glb_clr(REG_MM_AHB_AHB_RST, BIT(15));
}

void VPP_Dither_Info_Config(struct fb_to_vpp_info *fb_info, struct vpp_dither_device *dev)
{
	dev->reg_info.output_format = fb_info->output_format;	//RGB666
	dev->reg_info.pixel_type = fb_info->pixel_type;			//one pixel one word
	dev->reg_info.input_format = fb_info->input_format;		//ARGB888
	dev->reg_info.block_mod = 0;
	dev->reg_info.img_width = fb_info->img_width;
	dev->reg_info.img_height = fb_info->img_height;
	dev->reg_info.src_addr = fb_info->buf_addr;
	dev->reg_info.des_addr = fb_info->buf_addr;
}

void VPP_Dither_CFG_Config(struct vpp_dither_device *dev)
{
	unsigned int reg_value = 0;

	if(NULL == dev)
	{
		printk(KERN_ERR "[vpp_dither] %s, dev is null!\n",__func__);
		return 0;
	}

	if(dev->reg_info.output_format == 1)		//RGB666
		dev->reg_info.pixel_type = 1;			//one pixel one word

	reg_value |= dev->reg_info.output_format << 0;
	reg_value |= dev->reg_info.pixel_type << 1;
	reg_value |= dev->reg_info.input_format << 2;
	reg_value |= dev->reg_info.block_mod << 3;
	TB_REG_SET(DITH_PATH_CFG, reg_value);

	reg_value = 0;
	reg_value |= dev->reg_info.img_width << 0;
	reg_value |= dev->reg_info.img_height << 16;
	TB_REG_SET(DITH_IMG_SIZE, reg_value);

	reg_value = dev->reg_info.src_addr >> 3;
	TB_REG_SET(DITH_SRC_ADDR, reg_value);

	reg_value = dev->reg_info.des_addr >> 3;
	TB_REG_SET(DITH_DES_ADDR, reg_value);
}

int VPP_Dither_Map(struct vpp_dither_device *dev)
{
	return 0;
}

int VPP_Dither_Unmap(struct vpp_dither_device *dev)
{
	return 0;
}

int irq_status_get()
{
	unsigned int irq_value = 0;

	irq_value = TB_REG_GET(VPP_INT_STS);
	if(irq_value & 0x01)
		return 1;
	else
		return 0;
}

irqreturn_t VPP_Dither_IRQ_Handler(int irq, void *dev_id)
{
	struct vpp_dither_device *dev = dev_id;

	if(1 == irq_status_get())
	{
		printk(KERN_INFO "[vpp_dither] %s enter!\n",__func__);

		VPP_Dither_IRQENABLE_SET(0);
		VPP_Dither_IRQSTATUS_Clear();
		up(&dev->wait_interrupt_sem);

		VPP_Dither_IRQENABLE_SET(1);
		return IRQ_HANDLED;
	}
	else
		return IRQ_NONE;
}

void VPP_Dither_Trigger()
{
	TB_REG_SET(DITH_PATH_START, 0x01);
}

unsigned int Dither_WORKSTATUS_Get()
{
	return TB_REG_GET(AXIM_STS);
}

void VPP_Dither_Wait_Finish()
{
	volatile unsigned int i = 0;
	while(1)
	{
		if(Dither_WORKSTATUS_Get() == 0)
			return;
		else
		{
			msleep(1);
			i++;
			if(i >= 30)
			{
				printk(KERN_ERR "[vpp_dither] device is hane up!\n");
				return;
			}
		}
	}
}

/*
	FB Interface
*/
int Do_Dither_Work(struct fb_to_vpp_info *fb_info)
{
	int ret = 0;
	struct vpp_dither_device *dev = vpp_dither_ctx;

	printk(KERN_INFO "[vpp_dither] %s enter!\n",__func__);

	if(0 == dev->is_device_free)	//device busy
	{
		printk(KERN_ERR "[vpp_dither] %s, device is busy!\n",__func__);
		return ret;
	}

	/*VPP_DITHER_SET_PARAM*/

	ret = down_interruptible(&dev->hw_resource_sem);
	dev->is_device_free = 0;
	if(ret)
	{
		printk(KERN_ERR "[vpp_dither] %s,wait hw sema interrupt by signal,return",__func__);
		ret = -ERESTARTSYS;
		goto exit;
	}

	VPP_Dither_Init(dev);
	ret = VPP_Dither_Map(dev);
	if(ret)
	{
		printk(KERN_ERR "[vpp_dither] %s, mmu map fail!\n",__func__);
		ret = -EFAULT;
		goto exit2;
	}
	VPP_Dither_Info_Config(fb_info, dev);
	VPP_Dither_CFG_Config(dev);

	/*VPP_DITHER_TRIGGER_RUN*/
	VPP_Dither_Trigger();

	/*VPP_DITHER_WAIT_FINISH*/
	ret = down_timeout(&dev->wait_interrupt_sem, msecs_to_jiffies(30));
	if(ret == 0)
	{
		printk(KERN_INFO "[vpp_dither] %s,wait done sema success!\n",__func__);
	}
	else if(ret == -ETIME)//timeout
	{
		printk(KERN_ERR "[vpp_dither] %s,wait done sema timeout!\n",__func__);
		VPP_Dither_Module_Reset();
		VPP_Dither_Early_Init();
		ret = -EFAULT;
		goto exit2;
	}
	else if(ret)
	{
		printk(KERN_ERR "[vpp_dither] %s,wait done sema interrupted by a signal!\n",__func__);
		VPP_Dither_Module_Reset();
		VPP_Dither_Early_Init();
		ret = -EFAULT;
		goto exit2;
	}

//	VPP_Dither_Wait_Finish();
	VPP_Dither_Unmap(dev);
	VPP_Dither_Deinit(dev);

	up(&dev->hw_resource_sem);
	dev->is_device_free = 1;

	return ret;

exit2:
	VPP_Dither_Unmap(dev);
	VPP_Dither_Deinit(dev);
	sema_init(&dev->wait_interrupt_sem, 0);
exit1:
	up(&dev->hw_resource_sem);

exit:
	return ret;

}




