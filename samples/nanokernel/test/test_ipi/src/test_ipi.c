/* ipi_dummy.c - Fake IPI driver */

/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1) Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2) Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3) Neither the name of Intel Corporation nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <ipi.h>
#include <drivers/console/ipi_console.h>
#include <nanokernel.h>
#include <device.h>
#include <init.h>
#include <stdio.h>

#include <tc_util.h>
#include "ipi_dummy.h"

#define PRINTK_OUT	1

#if PRINTK_OUT
#define SOURCE	IPI_CONSOLE_STDOUT
#define DEST	IPI_CONSOLE_PRINTK
#else
#define SOURCE	IPI_CONSOLE_PRINTK
#define DEST	IPI_CONSOLE_STDOUT
#endif

/* Set up the dummy IPI driver */
struct ipi_dummy_config_info ipi_dummy0_config_info = {
	.sw_irq = 0
};
struct ipi_dummy_driver_data ipi_dummy0_driver_data;
DECLARE_DEVICE_INIT_CONFIG(ipi_dummy0, "ipi_dummy0", ipi_dummy_init,
			   &ipi_dummy0_config_info);
pure_init(ipi_dummy0, &ipi_dummy0_driver_data);

/* Sending side of the console IPI driver, will forward anything sent
 * to printf() since we selected IPI_CONSOLE_STDOUT */
struct ipi_console_sender_config_info sender_config = {
	.bind_to = "ipi_dummy0",
	.flags = SOURCE
};
DECLARE_DEVICE_INIT_CONFIG(ipi_console_send0, "ipi_send0",
			   ipi_console_sender_init, &sender_config);
nano_late_init(ipi_console_send0, NULL);

/* Receiving side of the console IPI driver. These numbers are
 * more or less arbitrary */
#define LINE_BUF_SIZE		80
#define RING_BUF_SIZE32		8

static uint32_t ring_buf_data[RING_BUF_SIZE32];
static char __stack fiber_stack[IPI_CONSOLE_STACK_SIZE];
static char line_buf[LINE_BUF_SIZE];

/* Dump incoming messages to printk() */
struct ipi_console_receiver_config_info receiver_config = {
	.bind_to = "ipi_dummy0",
	.fiber_stack = fiber_stack,
	.ring_buf_data = ring_buf_data,
	.rb_size32 = RING_BUF_SIZE32,
	.line_buf = line_buf,
	.lb_size = LINE_BUF_SIZE,
	.flags = DEST
};
struct ipi_console_receiver_runtime_data receiver_data;
DECLARE_DEVICE_INIT_CONFIG(ipi_console_recv0, "ipi_recv0",
			   ipi_console_receiver_init, &receiver_config);
nano_early_init(ipi_console_recv0, &receiver_data);

static const char thestr[] = "everything is awesome\n";

void main(void)
{
	int rv, i;
	struct device *ipi;

	TC_START("Test IPI");
	ipi = device_get_binding("ipi_dummy0");

	/* Try sending a raw string to the IPI device to show that the
	 * receiver works */
	for (i = 0; i < strlen(thestr); i++) {
		ipi_send(ipi, 1, thestr[i], NULL, 0);
	}

	/* Now do this through printf() to exercise the sender */
	printf("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\n");

	/* XXX how to tell if something was actually printed out for
	 * automation purposes? */

	rv = TC_PASS;
	TC_END_RESULT(rv);
	TC_END_REPORT(rv);
}

