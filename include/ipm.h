/* ipm.h - Generic low-level inter-processor mailbox communication API */

/*
 * Copyright (c) 2015 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __INCipmh
#define __INCipmh

/**
 * @brief IPM Interface
 * @defgroup ipm_interface IPM Interface
 * @ingroup io_interfaces
 * @{
 */

#include <nanokernel.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Callback API for incoming IPM messages
 *
 * These callbacks are executed in interrupt context, and only interrupt-
 * safe APIS may be used. Registration of callbacks is done via
 * ipm_register_callback()
 *
 * @param context Arbitrary context pointer provided at registration time
 * @param id Message type identifier
 * @param data Message data pointer. The correct amount of data to read out
 * needs to be inferred by the message id/upper level protocol
 */
typedef void (*ipm_callback_t)(void *context, uint32_t id, volatile void *data);

typedef int (*ipm_send_t)(struct device *ipmdev, int wait, uint32_t id,
			  const void *data, int size);
typedef int (*ipm_max_data_size_get_t)(struct device *ipmdev);
typedef uint32_t (*ipm_max_id_val_get_t)(struct device *ipmdev);
typedef void (*ipm_register_callback_t)(struct device *port, ipm_callback_t cb,
					void *cb_context);
typedef int (*ipm_set_enabled_t)(struct device *ipmdev, int enable);

struct ipm_driver_api {
	ipm_send_t send;
	ipm_register_callback_t register_callback;
	ipm_max_data_size_get_t max_data_size_get;
	ipm_max_id_val_get_t max_id_val_get;
	ipm_set_enabled_t set_enabled;
};

/**
 * Try to send a message over the IPM device
 *
 * @param ipmdev Driver instance
 * @param wait If nonzero, busy-wait for remote to consume the message. The
 *	       message is considered consumed once the remote interrupt handler
 *	       finishes. If there is deferred processing on the remote side,
 *	       or you would like to queue outgoing messages and wait on an
 *	       event/semaphore, you can implement that in a high-level driver
 * @param id Message identifier. Values are constrained by
 *	     ipm_max_data_size_get() since many platforms only allow for a
 *	     subset of bits in a 32-bit register to store the ID.
 * @param data Pointer to data to send in the message
 * @param size size of the data pointed to
 *
 * There are constraints on how much data can be sent or the maximum value
 * of id, use the ipm_max_data_size_get() and ipm_max_id_val_get() functions
 * to determine these.
 *
 * The size parameter is only used on the sending side to know how much data
 * to put in the message registers, it is not passed along to the receiving
 * side. How much data to read back should be dictated by the upper-level
 * protocol.
 *
 * @return -EBUSY if the remote hasn't yet read the last data sent
 *	   -EMSGSIZE If the supplied data size is unsupported by the driver
 *	   -EINVAL   Bad parameter, such as a too-large id value, or
 *		     the device isn't an outbound IPM channel
 *	   0 Success
 */
static inline int ipm_send(struct device *ipmdev, int wait, uint32_t id,
			   void *data, int size)
{
	struct ipm_driver_api *api;

	api = (struct ipm_driver_api *) ipmdev->driver_api;
	return api->send(ipmdev, wait, id, data, size);
}

/**
 * Register a callback function for incoming messages
 *
 * @param ipmdev Driver instance
 * @param cb Callback function to execute on incoming message interrupts
 * @param context Application-specific context pointer which will be passed
 *        to the callback function when executed.
 */
static inline void ipm_register_callback(struct device *ipmdev,
					 ipm_callback_t cb, void *context)
{
	struct ipm_driver_api *api;

	api = (struct ipm_driver_api *) ipmdev->driver_api;
	api->register_callback(ipmdev, cb, context);
}

/**
 * Return the maxmimum number of bytes that can be sent in an outbound message
 *
 * IPM implementations vary in how much data can be sent in a single message
 * since the data payload is typically stored in registers.
 *
 * @param ipmdev Driver instance pointer
 *
 * @return Max size of a message in bytes
 */
static inline int ipm_max_data_size_get(struct device *ipmdev)
{
	struct ipm_driver_api *api;

	api = (struct ipm_driver_api *) ipmdev->driver_api;
	return api->max_data_size_get(ipmdev);
}


/**
 * Return the maximum id value that can be sent in an outbound message
 *
 * Many IPM implementations store the id in a register which may have some bits
 * reserved for other use
 *
 * @param ipmdev Driver instance pointer
 *
 * @return Maximum value of a message id
 */
static inline uint32_t ipm_max_id_val_get(struct device *ipmdev)
{
	struct ipm_driver_api *api;

	api = (struct ipm_driver_api *) ipmdev->driver_api;
	return api->max_id_val_get(ipmdev);
}

/**
 * For inbound channels, enable interrupts/callbacks
 *
 * @param ipmdev Driver instance pointer
 * @param enable 0=disable nonzero=enable
 *
 * @return 0 on success, -EINVAL if this isn't an inbound channel
 */
static inline int ipm_set_enabled(struct device *ipmdev, int enable)
{
	struct ipm_driver_api *api;

	api = (struct ipm_driver_api *) ipmdev->driver_api;
	return api->set_enabled(ipmdev, enable);
}

#ifdef __cplusplus
}
#endif

/**
 * @}
 */
#endif /* __INCipmh */
