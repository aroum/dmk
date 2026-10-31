/**
 * *****************************************************************************
 *  @file       hcd_bmcu.c
 *  @author     Baikal electronics SDK team
 *  @brief      Baikal MCU USB Host driver source file
 *  @version    2.4.0
 *  @date       2026-10-06
 * *****************************************************************************
 *  @copyright Copyright (c) 2026 Baikal Electronics JSC
 *
 *  This software is licensed under terms that can be found in the LICENSE file
 *  in the root directory of this software component.
 *  If no LICENSE file comes with this software, it is provided AS-IS.
 * *****************************************************************************
 */

#include "hcd_bmcu.h"
#include "tusb_option.h"

#include <stdio.h>

#if CFG_TUH_ENABLED && CFG_TUSB_MCU == OPT_MCU_BMCU

#include "../../host/hcd.h"
#include "../../host/usbh.h"

#include "../../../hw/bsp/bmcu/board.h"

#define PIPE_MAX                (USBH_EP_MAX)   /*!< The maximum number of pipes (with Default Control Pipe) */
#define PIPE_INVALID_INDEX      (0xFFU)         /*!< Invalid block index */

#define FIFO_RAM_SIZE           (4096U)                 /*!< Total amount of RAM for the endpoint FIFOs */
#define FREE_BLK_LIST_SIZE      (PIPE_MAX * 2U - 1U)    /*!< The maximal number of nodes in the free block list */
#define BLK_INVALID_INDEX       (0xFFU)                 /*!< Invalid block index */
#define BLK_INVALID_ADDRESS     (0x0U)                  /*!< Invalid block address (0x0 is reserved for the EP0) */
#define BLK_START_ADDRESS       (64U)                   /*!< Start address for the free block list */

#if defined (BE_U1000) && (USBH_USE_DMA)
#define DMA_CH_MAX              (5U)        /*!< The maximum number of DMA channels */
#define DMA_CH_INVALID_INDEX    (0x1FU)     /*!< Invalid USB DMA channel index */
#define DMA_CH_NUM_MASK         (0x1FU)     /*!< DMA channel number mask */
#define DMA_CH_IN_USE_FLAG      (0x80U)     /*!< DMA channel is used for the selected channel flag */
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

/** @brief Pipe 0 state definitions */
typedef enum {
    PIPE0_STATE_IDLE = 0,       /*!< Idle state */
    PIPE0_STATE_SETUP,          /*!< Setup state */
    PIPE0_STATE_DATA_IN,        /*!< Data IN state */
    PIPE0_STATE_DATA_OUT,       /*!< Data OUT state */
    PIPE0_STATE_STATUS_IN,      /*!< Status IN state */
    PIPE0_STATE_STATUS_OUT      /*!< Status OUT state */
} pipe0_state_t;

/** @brief USB transfer control structure */
typedef struct {
    uint8_t* xfer_buf;          /*!< Pointer to the data buffer allocated for xfer */
    uint16_t xfer_len;          /*!< The size of the data buffer allocated for xfer.
                                     For IN direction it defines the length of the data to be sent to host,
                                     for OUT direction it is the size of the buffer allocated for the data from host. */
    uint16_t xferred;           /*!< The number of bytes transferred in the current xfer */
    uint16_t last_packet_size;  /*!< The size of the last packet sent or received in bytes */
} xfer_ctrl_t;

/** @brief Pipe control structure */
typedef struct {
    uint8_t dev_addr;           /*!< Target device address associated with the pipe */
    uint8_t ep_addr;            /*!< The address of the target device endpoint associated with the pipe */
    uint16_t max_packet_size;   /*!< The maximum packet size endpoint is capable of sending or receiving (wMaxPacketSize) */
    uint16_t fifo_addr;         /*!< The address of the host endpoint (pipe) FIFO */
    uint16_t fifo_size;         /*!< The size of the host endpoint (pipe) FIFO */
    xfer_ctrl_t xfer;           /*!< The transfer control structure */
#if defined (BE_U1000) && (USBH_USE_DMA)
    uint8_t dma_ch;             /*!< DMA channel assigned to the pipe */
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
} pipe_ctrl_t;

/** @brief USB Host Controller Driver control structure */
typedef struct
{
    pipe_ctrl_t pipe[PIPE_MAX][2];              /*!< The pipe control structure */
    pipe0_state_t pipe0_state;                  /*!< Pipe 0 state */
    tusb_control_request_t control_request;     /*!< Control request */
} hcd_ctrl_t;

static hcd_ctrl_t hcd;

/** @brief The macro to get the Pipe n control structure pointer */
#define PIPE_CTRL_BASE(num, dir)        &hcd.pipe[num][dir]

/** @brief Free block linked list node definition. */
typedef struct {
    uint16_t address;   /*!< Start address of the free block (within RAM buffer) */
    uint16_t size;      /*!< Size of the free block in bytes */
    uint8_t  prev;      /*!< Index of the previous free block linked list node */
    uint8_t  next;      /*!< Index of the next free block linked list node */
} free_blk_list_node_t;

/** @brief The array of free block list nodes. */
static free_blk_list_node_t free_blk_list_node[FREE_BLK_LIST_SIZE];

/** @brief The head of the free block list. */
static uint8_t free_blk_list_head;

#if defined (BE_U1000) && (USBH_USE_DMA)
/** @brief USB DMA control structure */
typedef struct
{
    uint8_t ep_addr[DMA_CH_MAX];    /*!< Local endpoint address the USB DMA channel assigned to.
                                         Bit 3..0: The endpoint number
                                         Bit 7:    Direction
                                                       0 = OUT endpoint
                                                       1 = IN endpoint */
} dma_ctrl_t;

dma_ctrl_t dma_ctrl;
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

#if defined (USBH_SPEED_TEST)

/** @brief USB Host speed test control structure definition. */
typedef struct {
    uint64_t start_time;    /*!< Test start time (us) */
    uint64_t stop_time;     /*!< Test stop time (us) */
    uint64_t bytes_sent;    /*!< The number of bytes sent */
    uint64_t test_data_lim; /*!< The number of bytes to send */
    bool     is_running;    /*!< True, if the test is running; otherwise false */
} hcd_speed_test_stats_t;

/** @brief USB Host speed test control structure. */
volatile hcd_speed_test_stats_t hcd_speed_test_stats;

/**
 * @brief Initializes the USB Host speed test.
 */
void hcd_speed_test_init(void)
{
    hcd_speed_test_stats.start_time = 0ULL;
    hcd_speed_test_stats.stop_time = 0ULL;
    hcd_speed_test_stats.bytes_sent = 0ULL;
    hcd_speed_test_stats.test_data_lim = 0ULL;
    hcd_speed_test_stats.is_running = false;
}

/**
 * @brief Sets the number of bytes to send during the USB Host speed test.
 */
void hcd_speed_test_set_data_lim(uint64_t size)
{
    hcd_speed_test_stats.test_data_lim = size;
}

/**
 * @brief Starts the USB Host speed test.
 */
void hcd_speed_test_start(void)
{
    printf("USB Host speed test is started.\r\n");
    hcd_speed_test_stats.is_running = true;
    hcd_speed_test_stats.bytes_sent = 0ULL;
    hcd_speed_test_stats.start_time = __get_time();
}

/**
 * @brief Stops the USB Host speed test.
 */
void hcd_speed_test_stop(void)
{
    printf("USB Host speed test is finished.\r\n");
    hcd_speed_test_stats.stop_time = __get_time();
    hcd_speed_test_stats.is_running = false;
}

/**
 * @brief Checks, if the USB Host speed test is running.
 * @returns True, if the test is running; otherwise false.
 */
bool hcd_speed_test_is_running(void)
{
    return hcd_speed_test_stats.is_running;
}

/**
 * @brief Returns the USB Host speed test start time.
 * @returns The start time in microseconds.
 */
uint64_t hcd_speed_test_get_start_time(void)
{
    return hcd_speed_test_stats.start_time;
}

/**
 * @brief Returns the USB Host speed test stop time.
 * @returns The stop time in microseconds.
 */
uint64_t hcd_speed_test_get_stop_time(void)
{
    return hcd_speed_test_stats.stop_time;
}

/**
 * @brief Returns the number of bytes sent during the USB Host speed test.
 * @returns The number of bytes sent.
 */
uint64_t hcd_speed_test_get_bytes_sent(void)
{
    return hcd_speed_test_stats.bytes_sent;
}

/**
 * @brief Prints an unsigned 64-bit number.
 * @param The number to print.
 */
void hcd_speed_test_print64(uint64_t value)
{
    uint64_t divisor = 1ULL;
    while (value / divisor >= 10ULL)
    {
        if (divisor > UINT64_MAX / 10U)
            break;

        divisor *= 10U;
    }

    while (divisor > 0ULL)
    {
        putchar((int)((value / divisor) % 10U) + '0');
        divisor /= 10U;
    }
}

/**
 * @brief Prints the USB Host speed test results.
 */
void hcd_speed_test_print_stats(void)
{
    printf("USB Host speed test results:\r\n");

    printf("Start time: ");
    hcd_speed_test_print64(hcd_speed_test_stats.start_time / 1000000UL);
    printf(".%03u", (uint16_t)((hcd_speed_test_stats.start_time + 500U) / 1000U) % 1000U);
    printf("s\r\n");

    printf("Stop time:  ");
    hcd_speed_test_print64(hcd_speed_test_stats.stop_time / 1000000UL);
    printf(".%03u", (uint16_t)((hcd_speed_test_stats.stop_time + 500U) / 1000U) % 1000U);
    printf("s\r\n");

    printf("Bytes sent: ");
    hcd_speed_test_print64(hcd_speed_test_stats.bytes_sent);
    printf("B\r\n");

    double speed = (double)(hcd_speed_test_stats.bytes_sent * 1000000UL) / (hcd_speed_test_stats.stop_time - hcd_speed_test_stats.start_time) / 1024U / 1024U;
    speed += 0.05;  /* Rounding */
    printf("Speed:      ");
    hcd_speed_test_print64((uint64_t)speed);
    printf(".%u", (uint16_t)((uint64_t)(speed * 10U) % 10U));
    printf("MB/s\r\n");
}

#endif /* defined (USBH_SPEED_TEST) */

/** @brief Endpoint 0 interrupt handler */
static void ep0_int_handler(void);
/** @brief Endpoint 1 to 15 RX interrupt handler */
static void epn_rx_int_handler(uint8_t epnum);
/** @brief Endpoint 1 to 15 TX interrupt handler */
static void epn_tx_int_handler(uint8_t epnum);

/** @brief Writes the endpoint FIFO. */
static void tx_fifo_write(uint8_t epnum, uint8_t const * buffer, size_t count);
/** @brief Reads the endpoint FIFO. */
static uint16_t rx_fifo_read(uint8_t epnum, uint8_t * buffer);

/** @brief Initializes a pipe control structure. */
static void pipe_ctrl_init(uint8_t num, tusb_dir_t dir);
/** @brief Finds pipe by the target device endpoint address. */
static uint8_t find_pipe(uint8_t dev_addr, uint8_t ep_addr);
/** @brief Finds a pipe that is not in use. */
static uint8_t find_free_pipe(tusb_dir_t dir);

/** @brief Initializes the free block list. */
static void free_blk_list_init(void);
/** @brief Removes a node from the free block list. */
static void free_blk_list_remove_node(uint8_t index);
/** @brief Allocates a memory block from the free block list. */
static uint16_t free_blk_list_alloc(uint16_t size);
/** @brief Deallocates a memory block and returns it to the free block list. */
static void free_blk_list_dealloc(uint16_t address, uint16_t size);

#if defined (BE_U1000) && (USBH_USE_DMA)
/** @brief Initializes USB DMA channel control structure. */
static void usb_dma_init();
/** @brief Finds the first unused USB DMA channel with the lowest index. */
static uint8_t usb_dma_channel_find(void);
/** @brief Allocates a USB DMA channel. */
static uint8_t usb_dma_channel_alloc(uint8_t ch_num, uint8_t ep_addr);
/** @brief Deallocates a USB DMA channel. */
static void usb_dma_channel_dealloc(uint8_t ch_num);
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

/** @brief Closes control or non-control pipe. */
static void close_pipe(uint8_t num, tusb_dir_t dir);

/**
 * @brief Initializes a pipe control structure.
 * @param num The pipe number.
 * @param dir The direction (TUSB_DIR_OUT or TUSB_DIR_IN).
 */
void pipe_ctrl_init(uint8_t num, tusb_dir_t dir)
{
    pipe_ctrl_t * pipe = PIPE_CTRL_BASE(num, dir);

    pipe->dev_addr = 0U;
    pipe->ep_addr = 0U;
    pipe->max_packet_size = 0U;
    pipe->fifo_addr = 0U;
    pipe->fifo_size = 0U;
#if defined (BE_U1000) && (USBH_USE_DMA)
    pipe->dma_ch = 0U;
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
}

/**
 * @brief Finds pipe by the target device endpoint address.
 * @param dev_addr The target device address.
 * @param ep_addr The target device endpoint address.
 * @returns The pipe number, if the pipe have been found, otherwise 0xFF.
 */
uint8_t find_pipe(uint8_t dev_addr, uint8_t ep_addr)
{
    uint8_t pipe_num = PIPE_INVALID_INDEX;

    uint8_t ep_num = tu_edpt_number(ep_addr);
    uint8_t ep_dir = tu_edpt_dir(ep_addr);

    if (ep_num == 0U)
    {
        pipe_num = 0U;
    }
    else
    {
        uint8_t i = 1U;
        do
        {
            pipe_ctrl_t * pipe = PIPE_CTRL_BASE(i, ep_dir);
            if ((pipe->dev_addr == dev_addr) &&
                (pipe->ep_addr == ep_addr))
            {
                pipe_num = i;
                break;
            }
            i++;
        }
        while (i < PIPE_MAX);
    }

    return pipe_num;
}

/**
 * @brief Finds a pipe that is not in use.
 * @note Pipe 0 is reserved for the Default Control Pipe.
 * @param dir The pipe direction (TUSB_DIR_OUT or TUSB_DIR_IN).
 * @returns The pipe number, if the pipe have been found, otherwise 0xFF.
 */
uint8_t find_free_pipe(tusb_dir_t dir)
{
    uint8_t pipe_num = PIPE_INVALID_INDEX;

    uint8_t i = 1U;
    do
    {
        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(i, dir);
        if (pipe->ep_addr == 0U)
        {
            pipe_num = i;
            break;
        }
        i++;
    }
    while (i < PIPE_MAX);

    return pipe_num;
}

/** @brief Initializes the free block list. */
void free_blk_list_init(void)
{
    /* Allocate a single memory block */
    /* The first 64 bytes are reserved for Endpoint 0 */
    free_blk_list_node[0].address = BLK_START_ADDRESS;
    free_blk_list_node[0].size = FIFO_RAM_SIZE - BLK_START_ADDRESS;
    free_blk_list_node[0].prev = BLK_INVALID_INDEX;
    free_blk_list_node[0].next = BLK_INVALID_INDEX;

    /* Clear the rest of nodes */
    for (uint8_t i = 1U; i < FREE_BLK_LIST_SIZE; i++)
    {
        free_blk_list_node[i].address = BLK_INVALID_ADDRESS;
        free_blk_list_node[i].size = 0U;
        free_blk_list_node[i].prev = BLK_INVALID_INDEX;
        free_blk_list_node[i].next = BLK_INVALID_INDEX;
    }

    free_blk_list_head = 0U;
}

/**
 * @brief Removes a node from the free block list.
 * @param index The node index.
 */
void free_blk_list_remove_node(uint8_t index)
{
    uint8_t prev = free_blk_list_node[index].prev;
    uint8_t next = free_blk_list_node[index].next;

    if (prev != BLK_INVALID_INDEX)
    {
        /* The node not the head */
        free_blk_list_node[prev].next = next;
    }
    else
    {
        /* The node is the head */
        free_blk_list_head = next;
    }

    if (next != BLK_INVALID_INDEX)
    {
        free_blk_list_node[next].prev = prev;
    }

    /* Clear the node. It marks that this node may be reused
       on memory deallocation. */
    free_blk_list_node[index].address = BLK_INVALID_ADDRESS;
    free_blk_list_node[index].size = 0U;
    free_blk_list_node[index].prev = BLK_INVALID_INDEX;
    free_blk_list_node[index].next = BLK_INVALID_INDEX;
}

/**
 * @brief Allocates a memory block from the free block list.
 * @note It finds the best-fit memory block conforming the next constraints:
 *       - The smallest free block that fits is taken
 *       - If multiple blocks match then use the one with the lowest address
 * @param size The block size to allocate.
 * @returns The allocated block address or BLK_INVALID_ADDRESS on error.
 */
uint16_t free_blk_list_alloc(uint16_t size)
{
    uint16_t alloc_address = BLK_INVALID_ADDRESS;
    uint8_t best_index = BLK_INVALID_INDEX;

    /* Find best-fit block */
    if (size != 0U)
    {
        uint8_t current_index = free_blk_list_head;
        
        while (current_index != BLK_INVALID_INDEX)
        {
            if (free_blk_list_node[current_index].size >= size)
            {
                /* A suitable block found */
                if (best_index == BLK_INVALID_INDEX)
                {
                    best_index = current_index;
                }
                else
                {
                    if ((free_blk_list_node[current_index].size < free_blk_list_node[best_index].size) ||
                        ((free_blk_list_node[current_index].size == free_blk_list_node[best_index].size) &&
                         (free_blk_list_node[current_index].address < free_blk_list_node[best_index].address)))
                    {
                        best_index = current_index;
                    }
                }
            }

            current_index = free_blk_list_node[current_index].next;
        }
    }

    if (best_index != BLK_INVALID_INDEX)
    {
        alloc_address = free_blk_list_node[best_index].address;

        if (free_blk_list_node[best_index].size == size)
        {
            /* The block will be allocated entirely: remove it from the list */
            free_blk_list_remove_node(best_index);
        }
        else
        {
            /* The block will be allocated partially: shrink it */
            free_blk_list_node[best_index].address += size;
            free_blk_list_node[best_index].size -= size;
        }
    }

    return alloc_address;
}

/**
 * @brief Deallocates a memory block and returns it to the free block list.
 * @param address The memory start address.
 * @param size The memory size in bytes.
 */
void free_blk_list_dealloc(uint16_t address, uint16_t size)
{
    if ((size != 0U) &&
        (address >= BLK_START_ADDRESS) &&
        (((uint32_t)address + size) <= FIFO_RAM_SIZE))
    {
        /* The new block to be added to the list */
        uint16_t new_blk_start = address;
        uint16_t new_blk_end   = address + (size - 1U);

        /* First pass: coalesce with all overlapping/adjacent nodes */
        uint8_t current_index = free_blk_list_head;

        while (current_index != BLK_INVALID_INDEX)
        {
            uint8_t next_index = free_blk_list_node[current_index].next;

            uint16_t blk_start = free_blk_list_node[current_index].address;
            uint16_t blk_end   = blk_start + (free_blk_list_node[current_index].size - 1U);

            if (!((new_blk_end < (blk_start - 1U)) ||
                 ((new_blk_start - 1U) > blk_end)))   /* Block start address is always higher then 0 by design,
                                                         no overload here */
            {
                /* Merge blocks: expand new region */
                if (blk_start < new_blk_start)
                {
                    new_blk_start = blk_start;
                }

                if (blk_end > new_blk_end)
                {
                    new_blk_end = blk_end;
                }

                /* Remove the node from the list */
                free_blk_list_remove_node(current_index);
            }

            current_index = next_index;
        }

        /* Second pass: find a free node index */
        current_index = BLK_INVALID_INDEX;

        for (uint8_t i = 0U; i < FREE_BLK_LIST_SIZE; i++)
        {
            if (free_blk_list_node[i].size == 0U)
            {
                current_index = i;
                break;
            }
        }

        /* A free node index always exists by design:
           the result of index check will be always true */
        if (current_index != BLK_INVALID_INDEX)
        {
            /* Add the new block to the list at head position */
            free_blk_list_node[current_index].address = new_blk_start;
            free_blk_list_node[current_index].size = new_blk_end - new_blk_start + 1U;

            free_blk_list_node[current_index].prev = BLK_INVALID_INDEX;
            free_blk_list_node[current_index].next = free_blk_list_head;

            if (free_blk_list_head != BLK_INVALID_INDEX)
            {
                free_blk_list_node[free_blk_list_head].prev = current_index;
            }

            free_blk_list_head = current_index;
        }
    }
}

#if defined (BE_U1000) && (USBH_USE_DMA)

/**
 * @brief Initializes USB DMA channel control structure.
 */
void usb_dma_init()
{
    memset(dma_ctrl.ep_addr, 0x0U, DMA_CH_MAX);
}

/**
 * @brief Finds the first unused USB DMA channel with the lowest index.
 * @note Channel 0 is reserved for the Default Control Pipe.
 * @returns The channel number, if it have been found, otherwise 0xFF.
 */
uint8_t usb_dma_channel_find(void)
{
    uint8_t ch_num = DMA_CH_INVALID_INDEX;

    if (DMA_CH_MAX > 1U)
    {
        uint8_t i = 1U;
        do
        {
            if (dma_ctrl.ep_addr[i] == 0x0U)
            {
                ch_num = i;
                break;
            }
            i++;
        }
        while (i < DMA_CH_MAX);
    }

    return ch_num;
}

/**
 * @brief Allocates a USB DMA channel.
 * @note Channel 0 is reserved for the Default Control Pipe.
 * @param ch_num The USB DMA channel number.
 * @param ep_addr Local endpoint address the USB DMA channel assigned to.
 * @returns 0, if the channel has been allocated successfully, otherwise 1.
 */
uint8_t usb_dma_channel_alloc(uint8_t ch_num, uint8_t ep_addr)
{
    uint8_t retval = 1U;

    if (ch_num == 0U)
    {
        if ((ep_addr == 0x0U) || (ep_addr == TUSB_DIR_IN_MASK))
        {
            dma_ctrl.ep_addr[ch_num] = ep_addr;
            retval = 0U;
        }
    }
    else
    {
        if (ch_num < DMA_CH_MAX)
        {
            if (dma_ctrl.ep_addr[ch_num] == 0x0U)
            {
                dma_ctrl.ep_addr[ch_num] = ep_addr;
                retval = 0U;
            }
        }
    }

    return retval;
}

/**
 * @brief Deallocates a USB DMA channel.
 * @param ch_num The USB DMA channel number.
 */
void usb_dma_channel_dealloc(uint8_t ch_num)
{
    if (ch_num < DMA_CH_MAX)
    {
        dma_ctrl.ep_addr[ch_num] = 0x0U;
    }
}

#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

//--------------------------------------------------------------------+
// Controller API
//--------------------------------------------------------------------+

/**
 * @brief Configures host stack behavior with dynamic or port-specific parameters.
 * @note This function is called by tuh_configure().
 * @note tuh_configure() is called before tuh_init().
 * @param rhport The roothub port number.
 * @param cfg_id The configuration ID.
 * @param cfg_param The configuration data.
 */
bool hcd_configure(uint8_t rhport, uint32_t cfg_id, const void* cfg_param)
{
    (void)rhport;
    (void)cfg_id;
    (void)cfg_param;

    /* Not implemented */

    return false;
}

/**
 * @brief Initializes USB controller in host mode.
 * @param rhport The roothub port number.
 * @param rh_init The pointer to the roothub port initialization structure.
 */
bool hcd_init(uint8_t rhport, const tusb_rhport_init_t* rh_init)
{
    (void)rhport;
    (void)rh_init;

    /* Reset the USB controller. Software reset is not implemented in BMCU-U and
       BE-U1000 microcontrollers. It's included for future compatibility. */
    USB->CTRL.SOFT_RST |= (USB_SOFT_RST_NRSTX | USB_SOFT_RST_NRST);
    while ((USB->CTRL.SOFT_RST & (USB_SOFT_RST_NRSTX | USB_SOFT_RST_NRST)) != 0U);

    /* Disable endpoint interrupts */
    USB->COMMON.INTRRXE = 0x0U;
    USB->COMMON.INTRTXE = 0x0U;

    /* Initialize pipe control structures */
    for (uint8_t i = 0U; i < PIPE_MAX; i++)
    {
        pipe_ctrl_init(i, TUSB_DIR_OUT);
        pipe_ctrl_init(i, TUSB_DIR_IN);
    }

    /* Initialize FIFO RAM free block list */
    free_blk_list_init();

#if defined (BE_U1000) && (USBH_USE_DMA)
    /* Initialize USB DMA control structure */
    usb_dma_init();
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

    /* Configure DRVVBUS control */
    board_vbus_init();

    /* Enable host functions: high speed host disconnection detection */
#if defined (BE_U1000)
    CRU->USBCR |= CRU_USBCR_OTG_SUSPENDM;
#endif

    /* Enable interrupts */
    USB->COMMON.INTRUSBE = USB_INTRUSBE_VBUSERRIE |
                           USB_INTRUSBE_DISCONIE |
                           USB_INTRUSBE_CONNIE |
                           USB_INTRUSBE_RESETIE;

    /* Configure USB interrupts in CLIC */
#ifndef CORE_BM310S14
    CLIC_ConfigIRQ(CLIC_USB_IRQn,                   /* Interrupt */
                   CLIC_INTATTR_MODE_MACHINE,       /* Privilege mode */
                   USB_INT_LEVEL,                   /* Level */
                   USB_INT_PRIORITY,                /* Priority */
                   CLIC_INTATTR_SHV_VECTORED,       /* Vector mode */
                   CLIC_INTATTR_TRIG_TYPE_LEVEL,    /* Type */
                   CLIC_INTATTR_TRIG_POL_P);        /* Polarity */

#if defined (BE_U1000) && (USBH_USE_DMA)
    CLIC_ConfigIRQ(CLIC_USB_DMA_IRQn,               /* Interrupt */
                   CLIC_INTATTR_MODE_MACHINE,       /* Privilege mode */
                   USB_DMA_INT_LEVEL,               /* Level */
                   USB_DMA_INT_PRIORITY,            /* Priority */
                   CLIC_INTATTR_SHV_VECTORED,       /* Vector mode */
                   CLIC_INTATTR_TRIG_TYPE_LEVEL,    /* Type */
                   CLIC_INTATTR_TRIG_POL_P);        /* Polarity */
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

#endif

    /* Start session */
    USB->CTRL.DEVCTL |= USB_DEVCTL_SESSION;

    /* No need to wait for Host Mode bit here, it will be tested
       upon device connection with hcd_port_connect_status() */

    return true;
}

/**
 * @brief Processes all the hardware generated events.
 * @param rhport The roothub port number.
 * @param in_isr Executed from ISR flag.
 */
void hcd_int_handler(uint8_t rhport, bool in_isr)
{
    (void)in_isr;

    /* Read and clear interrupt registers */
    uint8_t usbInt = USB->COMMON.INTRUSB & USB->COMMON.INTRUSBE;
    uint16_t rxInt = USB->COMMON.INTRRX & USB->COMMON.INTRRXE;
    uint16_t txInt = USB->COMMON.INTRTX & USB->COMMON.INTRTXE;

    if (usbInt & USB_INTRUSB_VBUSERRIF)
    {
        /* Vbus error:
               - The sequence to recover from VBUS error:
                     1. Turn off VBUS.  Wait until VBUS level indicator
                        reads 00b or 01b.
                     2. Turn on VBUS. Wait until VBUS level indicator
                        reads 11b.
                     3. Set SESSION bit.
               - When VBUS is controlled by the Session bit, we only need
                 to restart session.
               - In the case of VBus > VBus Valid becomes valid again,
                 the Connect interrupt will be generated, the connected
                 device will be reset and re-enumerated.
               - The insufficient power on VBUS may lead to the situation
                 the connected device is not responding and the enumeration
                 process gets stuck. At the moment, TinyUSB stack does not
                 implement timeout for device enumeration. In this case
                 the device must be unplugged and plugged in again manually.
        */

        /* Generate disconnect event to release resources */
        hcd_event_device_remove(rhport, true);

        /* Process event immediately to make sure it is handled
           before the next connect event occurs. */
        tuh_task_ext(UINT32_MAX, true);

        /* Restart session */
        USB->CTRL.DEVCTL |= USB_DEVCTL_SESSION;
    }

    if (usbInt & USB_INTRUSB_CONNIF)
    {
        /* Connect interrupt */
        hcd_event_device_attach(rhport, true);
    }

    if (usbInt & USB_INTRUSB_DISCONIF)
    {
        /* Disconnect interrupt */
        hcd_event_device_remove(rhport, true);
    }

    if (usbInt & USB_INTRUSB_RESETIF)
    {
        /* Babble: Port has assumed that the function it is connected to
                   has malfunctioned and has suspended all transactions. */

        /* Restart session. The device might need to be unplugged.
           The user might need to notify application about the error. */
        USB->CTRL.DEVCTL |= USB_DEVCTL_SESSION;
    }

    /* Endpoint 0 interrupt */
    if (txInt & USB_INTRTX_EP0IF)
    {
        txInt &= ~USB_INTRTX_EP0IF;
        ep0_int_handler();
    }

    /* Tx Endpoint's interrupts */
    uint8_t i = 0U;
    while (txInt > 0U)
    {
        txInt >>= 1;
        i++;

        if (txInt & 0x1U)
        {
            epn_tx_int_handler(i);
        }
    }

    /* Rx Endpoint's interrupts */
    i = 0U;
    while (rxInt > 0U)
    {
        rxInt >>= 1;
        i++;

        if (rxInt & 0x1U)
        {
            epn_rx_int_handler(i);
        }
    }
}

/**
 * @brief Endpoint 0 interrupt handler.
 */
void ep0_int_handler(void)
{
    uint8_t status = USB->EPCSR[0].HOST.CSR0L;

    if (status & (USB_CSR0L_HOST_NAKTMOUT | USB_CSR0L_HOST_ERROR | USB_CSR0L_HOST_RXSTALL))
    {
        xfer_result_t xfer_result;

        /* Flush FIFO */
        if (status & (USB_CSR0L_HOST_TXPKTRDY | USB_CSR0L_HOST_RXPKTRDY))
        {
            USB->EPCSR[0].HOST.CSR0H = USB_CSR0H_HOST_FLSHFIFO;
        }

        if (status & USB_CSR0L_HOST_RXSTALL)
        {
            xfer_result = XFER_RESULT_STALLED;
        }
        else
        {
            xfer_result = XFER_RESULT_FAILED;
        }

        /* Clear error bits */
        USB->EPCSR[0].HOST.CSR0L = 0x0U;

        switch (hcd.pipe0_state)
        {
            case PIPE0_STATE_SETUP:
            {
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_OUT);
                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, xfer_result, true);
            }
            break;

            case PIPE0_STATE_DATA_IN:
            {
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_IN);
                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, xfer_result, true);
            }
            break;

            case PIPE0_STATE_DATA_OUT:
            {
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_OUT);
                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, xfer_result, true);
            }
            break;

            case PIPE0_STATE_STATUS_IN:
            {
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_IN);
                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, 0U, xfer_result, true);
            }
            break;

            case PIPE0_STATE_STATUS_OUT:
            {
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_OUT);
                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, 0U, xfer_result, true);
            }
            break;

            default:
            {
                /* No action */
            }
            break;
        }

        hcd.pipe0_state = PIPE0_STATE_IDLE;
    }
    else
    {
        switch (hcd.pipe0_state)
        {
            case PIPE0_STATE_SETUP:
            {
                /* Setup state */

                /* The SETUP Phase has been correctly ACKed */
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_OUT);

                pipe->xfer.xferred += pipe->xfer.last_packet_size;

                if (hcd.control_request.wLength != 0U)
                {
                    /* Transfer has Data stage */
                    if (hcd.control_request.bmRequestType_bit.direction == TUSB_DIR_IN)
                    {
                        /* Device-to-host */
                        hcd.pipe0_state = PIPE0_STATE_DATA_IN;
                    }
                    else
                    {
                        /* Host-to-device */
                        hcd.pipe0_state = PIPE0_STATE_DATA_OUT;
                    }
                }
                else
                {
                    /* Transfer has no Data stage */
                    hcd.pipe0_state = PIPE0_STATE_STATUS_IN;
                }

                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
            }
            break;

            case PIPE0_STATE_DATA_IN:
            {
                /* Data IN state */
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_IN);

                pipe->xfer.last_packet_size = 0U;

#if defined (BE_U1000) && (USBH_USE_DMA)
                if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
                {
                    if (pipe->xfer.xfer_buf != NULL)
                    {
                        pipe->xfer.last_packet_size = tu_min16(USB->EPCSR[0].PERIPH.RXCOUNT, pipe->xfer.xfer_len - pipe->xfer.xferred);

                        uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

                        USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                        USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
                        USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | (0UL << USB_DMA_CNTL_DMAEP_Pos) |
                                                       USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                       USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
                    }
                }
                else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
                {
                    if (pipe->xfer.xfer_buf != NULL)
                    {
                        pipe->xfer.last_packet_size = rx_fifo_read(0U, (uint8_t*)((uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred));
                        pipe->xfer.xferred += pipe->xfer.last_packet_size;
                    }

                    if ((pipe->xfer.xferred >= hcd.control_request.wLength) ||
                        (pipe->xfer.last_packet_size < pipe->max_packet_size))
                    {
                        /* The last data packet */
                        hcd.pipe0_state = PIPE0_STATE_STATUS_OUT;
                        USB->EPCSR[0].HOST.CSR0L &= ~USB_CSR0L_HOST_RXPKTRDY;

                        /* Notify stack about the end of the data transaction */
                        hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
                    }
                    else
                    {
                        /* More data packets to request */
                        USB->EPCSR[0].HOST.CSR0L |= USB_CSR0L_HOST_REQPKT;
                    }
                }
            }
            break;

            case PIPE0_STATE_DATA_OUT:
            {
                /* Data OUT state */
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_OUT);

                /* The last packet has been correctly ACKed */
                pipe->xfer.xferred += pipe->xfer.last_packet_size;

                if ((pipe->xfer.xferred < pipe->xfer.xfer_len) &&
                    (pipe->xfer.last_packet_size == pipe->max_packet_size))
                {
                    /* Send the next data packet */
                    pipe->xfer.last_packet_size = tu_min16(pipe->max_packet_size, pipe->xfer.xfer_len - pipe->xfer.xferred);

#if defined (BE_U1000) && (USBH_USE_DMA)
                    if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
                    {
                        uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

                        USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                        USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
                        USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | (0UL << USB_DMA_CNTL_DMAEP_Pos) |
                                                       USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                       USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
                    }
                    else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
                    {
                        tx_fifo_write(0U, (uint8_t*)((uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred), pipe->xfer.last_packet_size);

                        USB->EPCSR[0].HOST.CSR0L |= USB_CSR0L_HOST_TXPKTRDY;
                    }
                }
                else
                {
                    hcd.pipe0_state = PIPE0_STATE_STATUS_IN;

                    /* Notify USB stack about the data sent */
                    hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
                }
            }
            break;

            case PIPE0_STATE_STATUS_IN:
            {
                /* Status IN state */
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_IN);

                USB->EPCSR[0].HOST.CSR0L &= ~(USB_CSR0L_HOST_STATPKT | USB_CSR0L_HOST_RXPKTRDY);

                /* Notify stack about the status stage of control transfer */
                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, 0U, XFER_RESULT_SUCCESS, true);

                /* Set Pipe 0 state to IDLE */
                hcd.pipe0_state = PIPE0_STATE_IDLE;
            }
            break;

            case PIPE0_STATE_STATUS_OUT:
            {
                /* Status OUT state */
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_OUT);

                /* Notify stack about the status stage of control transfer */
                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, 0U, XFER_RESULT_SUCCESS, true);

                /* Set Pipe 0 state to IDLE */
                hcd.pipe0_state = PIPE0_STATE_IDLE;
            }
            break;

            default:
            {
                /* Set Pipe 0 state to IDLE */
                hcd.pipe0_state = PIPE0_STATE_IDLE;
            }
            break;
        }
    }
}

/**
 * @brief RX Endpoint 1 to 15 interrupt handler.
 * @param epnum The endpoint number.
 */
void epn_rx_int_handler(uint8_t epnum)
{
    uint8_t status = USB->EPCSR[epnum].HOST.RXCSRL;

    if (status & (USB_RXCSRL_HOST_RXSTALL | USB_RXCSRL_HOST_DERRNAKT | USB_RXCSRL_HOST_ERROR))
    {
        xfer_result_t xfer_result;

        /* Flush FIFO */
        if (status & USB_RXCSRL_HOST_RXPKTRDY)
        {
            USB->EPCSR[epnum].HOST.RXCSRL = USB_RXCSRL_HOST_FLSHFIFO;
        }

        if (status & USB_RXCSRL_HOST_RXSTALL)
        {
            xfer_result = XFER_RESULT_STALLED;
        }
        else
        {
            xfer_result = XFER_RESULT_FAILED;
        }

        /* Clear error bits */
        USB->EPCSR[epnum].HOST.RXCSRL = 0x0U;

        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(epnum, TUSB_DIR_IN);
        hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, xfer_result, true);
    }
    else
    {
        if (status & USB_RXCSRL_HOST_RXPKTRDY)
        {
            pipe_ctrl_t * pipe = PIPE_CTRL_BASE(epnum, TUSB_DIR_IN);

#if defined (USBH_SPEED_TEST) && defined (USBH_SPEED_TEST_RX)
            if (!hcd_speed_test_is_running())
            {
                if (hcd_speed_test_get_bytes_sent() == 0ULL)
                {
                    hcd_speed_test_start();
                }
            }

            if (hcd_speed_test_is_running())
            {
#if defined (BE_U1000) && (USBH_USE_DMA)
                if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
                {
                    pipe->xfer.last_packet_size = tu_min16(USB->EPCSR[epnum].PERIPH.RXCOUNT, pipe->xfer.xfer_len - pipe->xfer.xferred);

                    uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

                    USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                    USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
                    USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                                   USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                   USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
                }
                else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
                {
                    /* Read data from FIFO */
                    pipe->xfer.last_packet_size = rx_fifo_read(epnum, (uint8_t*)((uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred));

                    /* Clear RxPktRdy flag */
                    USB->EPCSR[epnum].HOST.RXCSRL &= ~USB_RXCSRL_HOST_RXPKTRDY;

                    hcd_speed_test_stats.bytes_sent += pipe->xfer.last_packet_size;

                    if (hcd_speed_test_stats.bytes_sent < hcd_speed_test_stats.test_data_lim)
                    {
                        /* Send IN token */
                        USB->EPCSR[epnum].HOST.RXCSRL |= USB_RXCSRL_HOST_REQPKT;
                    }
                    else
                    {
                        /* All test packets were received */
                        /* End xfer correctly even though it was interfered by the test */

                        hcd_speed_test_stop();
                        hcd_speed_test_print_stats();

                        /* The last packet has been correctly ACKed */
                        pipe->xfer.xferred += pipe->xfer.last_packet_size;

                        hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
                    }
                }
            }
#else
#if defined (BE_U1000) && (USBH_USE_DMA)
            if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
            {
                pipe->xfer.last_packet_size = tu_min16(USB->EPCSR[epnum].PERIPH.RXCOUNT, pipe->xfer.xfer_len - pipe->xfer.xferred);

                uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

                USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
                USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                               USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                               USB_DMA_CNTL_DMADIR_WRITE | USB_DMA_CNTL_DMAEN_ENABLE;
            }
            else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
            {
                /* Read data from FIFO */
                pipe->xfer.last_packet_size = rx_fifo_read(epnum, (uint8_t*)((uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred));

                /* Clear RxPktRdy flag */
                USB->EPCSR[epnum].HOST.RXCSRL &= ~USB_RXCSRL_HOST_RXPKTRDY;

                pipe->xfer.xferred += pipe->xfer.last_packet_size;

                if ((pipe->xfer.xferred < pipe->xfer.xfer_len) &&
                    (pipe->xfer.last_packet_size == pipe->max_packet_size))
                {
                    /* More data packets expected */

                    /* Send IN token */
                    USB->EPCSR[epnum].HOST.RXCSRL |= USB_RXCSRL_HOST_REQPKT;
                }
                else
                {
                    /* Data transfer complete */

                    /* Notify USB stack about the data received */
                    hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
                }
            }
#endif /* defined (USBH_SPEED_TEST) && defined (USBH_SPEED_TEST_RX) */
        }
    }
}

/**
 * @brief TX Endpoint 1 to 15 interrupt handler.
 * @param epnum The endpoint number.
 */
void epn_tx_int_handler(uint8_t epnum)
{
    uint8_t status = USB->EPCSR[epnum].HOST.TXCSRL;

    if (status & (USB_TXCSRL_HOST_INCOMPTX | USB_TXCSRL_HOST_RXSTALL | USB_TXCSRL_HOST_ERROR))
    {
        xfer_result_t xfer_result;

        /* Flush FIFO */
        if (status & USB_TXCSRL_HOST_TXPKTRDY)
        {
            USB->EPCSR[epnum].HOST.TXCSRL = USB_TXCSRL_HOST_FLSHFIFO;
        }

        if (status & USB_TXCSRL_HOST_RXSTALL)
        {
            xfer_result = XFER_RESULT_STALLED;
        }
        else
        {
            xfer_result = XFER_RESULT_FAILED;
        }

        /* Clear error bits */
        USB->EPCSR[epnum].HOST.TXCSRL = 0x0U;

        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(epnum, TUSB_DIR_OUT);
        hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, xfer_result, true);
    }
    else
    {
        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(epnum, TUSB_DIR_OUT);

#if defined (USBH_SPEED_TEST) && defined (USBH_SPEED_TEST_TX)
        if (hcd_speed_test_is_running())
        {
            hcd_speed_test_stats.bytes_sent += pipe->xfer.last_packet_size;

            if (hcd_speed_test_stats.bytes_sent < hcd_speed_test_stats.test_data_lim)
            {
                /* Send more test packets without involving USB stack */
#if defined (BE_U1000) && (USBH_USE_DMA)
                if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
                {
                    uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

                    USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                    USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
                    USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                                   USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                   USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
                }
                else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
                {
                    tx_fifo_write(epnum, (uint8_t*)((uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred), pipe->xfer.last_packet_size);

                    USB->EPCSR[epnum].HOST.TXCSRL |= USB_TXCSRL_HOST_TXPKTRDY;
                }
            }
            else
            {
                /* All test packets were sent */
                /* End xfer correctly even though it was interfered by the test */

                hcd_speed_test_stop();
                hcd_speed_test_print_stats();

                /* The last packet has been correctly ACKed */
                pipe->xfer.xferred += pipe->xfer.last_packet_size;

                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
            }
        }
        else
#endif /* defined (USBH_SPEED_TEST) && defined (USBH_SPEED_TEST_TX) */
        {
            /* The last packet has been correctly ACKed */
            pipe->xfer.xferred += pipe->xfer.last_packet_size;

            if ((pipe->xfer.xferred < pipe->xfer.xfer_len) &&
                (pipe->xfer.last_packet_size == pipe->max_packet_size))
            {
                /* Send the next data packet */
                pipe->xfer.last_packet_size = tu_min16(pipe->max_packet_size, pipe->xfer.xfer_len - pipe->xfer.xferred);

#if defined (BE_U1000) && (USBH_USE_DMA)
                if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
                {
                    uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

                    USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred + (uint32_t)RAM_ACCESS_OFFSET;
                    USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
                    USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)epnum << USB_DMA_CNTL_DMAEP_Pos) |
                                                   USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                   USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
                }
                else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
                {
                    tx_fifo_write(epnum, (uint8_t*)((uint32_t)pipe->xfer.xfer_buf + pipe->xfer.xferred), pipe->xfer.last_packet_size);

                    USB->EPCSR[epnum].HOST.TXCSRL |= USB_TXCSRL_HOST_TXPKTRDY;
                }
            }
            else
            {
                /* Notify USB stack about the data sent */
                hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
            }
        }
    }
}

/**
 * @brief Enables the USB host interrupts.
 * @param rhport The roothub port number.
 */
void hcd_int_enable(uint8_t rhport)
{
    (void)rhport;

#ifndef CORE_BM310S14
    CLIC_EnableIRQ(CLIC_USB_IRQn);
#if defined (BE_U1000) && (USBH_USE_DMA)
    CLIC_EnableIRQ(CLIC_USB_DMA_IRQn);
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
#endif
}

/**
 * @brief Disables the USB host interrupts.
 * @param rhport The roothub port number.
 */
void hcd_int_disable(uint8_t rhport)
{
    (void)rhport;

#ifndef CORE_BM310S14
    CLIC_DisableIRQ(CLIC_USB_IRQn);
#if defined (BE_U1000) && (USBH_USE_DMA)
    CLIC_DisableIRQ(CLIC_USB_DMA_IRQn);
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
#endif
}

/**
 * @brief Returns the last frame number.
 */
uint32_t hcd_frame_number(uint8_t rhport)
{
    (void)rhport;

    return USB->COMMON.FRAME;
}

//--------------------------------------------------------------------+
// Port API
//--------------------------------------------------------------------+

/**
 * @brief Returns the current connect status of a roothub port.
 * @param rhport The roothub port number.
 * @returns True, if acting as a host and there is a device connected, otherwise false.
 */
bool hcd_port_connect_status(uint8_t rhport)
{
    (void)rhport;

    bool retval = true;

    uint8_t devctl = USB->CTRL.DEVCTL;

    if ((devctl & USB_DEVCTL_HOSTMODE) == 0U)
    {
        /* Not host */
        retval = false;
    }

    if ((devctl & (USB_DEVCTL_FSDEV | USB_DEVCTL_LSDEV)) == 0U)
    {
        /* No device connected */
        retval = false;
    }

    return retval;
}

/**
 * @brief Resets USB bus on a port.
 * @note Returns immediately, hcd_port_reset_end() will be invoked after 10ms 
 *       to complete the reset sequence.
 * @param rhport The roothub port number.
 */
void hcd_port_reset(uint8_t rhport)
{
    (void)rhport;

    if (TUH_OPT_HIGH_SPEED)
    {
        /* High-speed mode */
        USB->COMMON.POWER |= USB_POWER_HSEN;
    }
    else
    {
        /* Full-speed mode */
        USB->COMMON.POWER &= ~USB_POWER_HSEN;
    }

    USB->COMMON.POWER |= USB_POWER_RESET;
}

/**
 * @brief Completes USB bus reset sequence.
 * @note Invoked 10ms after hcd_port_reset().
 * @param rhport The roothub port number.
 */
void hcd_port_reset_end(uint8_t rhport)
{
    (void)rhport;

    USB->COMMON.POWER &= ~USB_POWER_RESET;
}

/**
 * @brief Returns USB port link speed.
 * @param rhport The roothub port number.
 * @returns The USB port link speed.
 */
tusb_speed_t hcd_port_speed_get(uint8_t rhport)
{
    (void)rhport;

    tusb_speed_t speed = TUSB_SPEED_INVALID;

    uint8_t devctl = USB->CTRL.DEVCTL;

    if ((devctl & USB_DEVCTL_LSDEV) != 0U)
    {
        /* Low-speed */
        speed = TUSB_SPEED_LOW;
    }
    else
    {
        if ((devctl & USB_DEVCTL_FSDEV) != 0U)
        {
            /* Full-speed */
            speed = TUSB_SPEED_FULL;
        }

        if ((USB->COMMON.POWER & USB_POWER_HSMODE) != 0U)
        {
            /* High-speed */
            speed = TUSB_SPEED_HIGH;
        }
    }

    return speed;
}

/**
 * @brief Closes control or non-control pipe.
 * @note The function does not perform parameters validation.
 * @param num The pipe number.
 * @param dir The pipe direction (TUSB_DIR_OUT or TUSB_DIR_IN).
 */
void close_pipe(uint8_t num, tusb_dir_t dir)
{
    if (num == 0U)
    {
        /* Disable Endpoint 0 interrupt */
        USB->COMMON.INTRTXE &= ~0x1U;

        /* Flush FIFO */
        if (USB->EPCSR[0].HOST.CSR0L & (USB_CSR0L_HOST_TXPKTRDY | USB_CSR0L_HOST_RXPKTRDY))
        {
            USB->EPCSR[0].HOST.CSR0H = USB_CSR0H_HOST_FLSHFIFO;
        }

        /* Clear control and status register */
        USB->EPCSR[0].HOST.CSR0L = 0x0U;

#if defined (BE_U1000) && (USBH_USE_DMA)
        /* Deallocate USB DMA channel */
        usb_dma_channel_dealloc(0U);
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

        hcd.pipe0_state = PIPE0_STATE_IDLE;
    }
    else
    {
        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(num, dir);

        if (dir == TUSB_DIR_OUT)
        {
            /* OUT */

            /* Disable endpoint interrupt */
            USB->COMMON.INTRTXE &= (1U << num);

            /* Flush FIFO */
            if (USB->EPCSR[num].HOST.TXCSRL & USB_TXCSRL_HOST_TXPKTRDY)
            {
                USB->EPCSR[num].HOST.TXCSRL = USB_TXCSRL_HOST_FLSHFIFO;
            }

            /* Clear control and status register */
            USB->EPCSR[num].HOST.TXCSRL = 0x0U;
        }
        else
        {
            /* IN */

            /* Disable endpoint interrupt */
            USB->COMMON.INTRRXE &= (1U << num);

            /* Flush FIFO */
            if (USB->EPCSR[num].HOST.RXCSRL & USB_RXCSRL_HOST_RXPKTRDY)
            {
                USB->EPCSR[num].HOST.RXCSRL = USB_RXCSRL_HOST_FLSHFIFO;
            }

            /* Clear control and status register */
            USB->EPCSR[num].HOST.RXCSRL = 0x0U;
        }

#if defined (BE_U1000) && (USBH_USE_DMA)
        /* Deallocate USB DMA channel */
        usb_dma_channel_dealloc(pipe->dma_ch);
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

        /* Deallocate FIFO memory block */
        free_blk_list_dealloc(pipe->fifo_addr, pipe->fifo_size);

        /* Clear pipe control structure */
        pipe_ctrl_init(num, dir);
    }
}

/**
 * @brief Closes all endpoints that belong to the target device.
 * @param rhport The roothub port number.
 * @param dev_addr The target device address.
 */
void hcd_device_close(uint8_t rhport, uint8_t dev_addr)
{
    (void)rhport;

    if (dev_addr == 0x0U)
    {
        close_pipe(0U, TUSB_DIR_OUT);
    }
    else
    {
        for (uint8_t i = 1U; i < PIPE_MAX; i++)
        {
            /* OUT */
            pipe_ctrl_t * pipe = PIPE_CTRL_BASE(i, TUSB_DIR_OUT);
            if (pipe->dev_addr == dev_addr)
            {
                close_pipe(i, TUSB_DIR_OUT);
            }

            /* IN */
            pipe = PIPE_CTRL_BASE(i, TUSB_DIR_IN);
            if (pipe->dev_addr == dev_addr)
            {
                close_pipe(i, TUSB_DIR_IN);
            }
        }
    }
}

//--------------------------------------------------------------------+
// Endpoints API
//--------------------------------------------------------------------+

/**
 * @brief Opens an endpoint (control or non-control).
 * @param rhport The roothub port number.
 * @param dev_addr The target device address.
 * @param ep_desc The target device endpoint descriptor.
 *                - Control: The default Endpoint 0 descriptor.
 *                - Non-control: The descriptor received during enumeration.
 * @return True, if the endpoint has been opened successfully, otherwise false.
 */
bool hcd_edpt_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_endpoint_t const * ep_desc)
{
    (void)rhport;

    bool retval = true;

    /* The target device endpoint address */
    uint8_t ep_addr = ep_desc->bEndpointAddress;
    uint8_t ep_num = tu_edpt_number(ep_addr);
    uint8_t ep_dir = tu_edpt_dir(ep_addr);

    if (ep_num == 0U)
    {
        /* Endpoint 0 */
        USB->EPCSR[0].HOST.NAKLIMIT0 = ep_desc->bInterval;

        /* The control pipe consumes two endpoint addresses (IN and OUT).
           Initialize two control structures for it. */
        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_IN);
        pipe->dev_addr = dev_addr;
        pipe->ep_addr = TUSB_DIR_IN_MASK | 0x0U;
        pipe->max_packet_size = tu_edpt_packet_size(ep_desc);
        pipe->fifo_addr = 0U;
        pipe->fifo_size = pipe->max_packet_size;
#if defined (BE_U1000) && (USBH_USE_DMA)
        pipe->dma_ch = DMA_CH_IN_USE_FLAG;
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

        pipe = PIPE_CTRL_BASE(0, TUSB_DIR_OUT);
        pipe->dev_addr = dev_addr;
        pipe->ep_addr = 0x0U;
        pipe->max_packet_size = tu_edpt_packet_size(ep_desc);
        pipe->fifo_addr = 0U;
        pipe->fifo_size = pipe->max_packet_size;
#if defined (BE_U1000) && (USBH_USE_DMA)
        pipe->dma_ch = DMA_CH_IN_USE_FLAG;
        usb_dma_channel_alloc(0U, 0x0U);
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

        /* Set EP0 state to IDLE */
        hcd.pipe0_state = PIPE0_STATE_IDLE;

        /* Flush FIFO */
        if (USB->EPCSR[0].HOST.CSR0L & (USB_CSR0L_HOST_TXPKTRDY | USB_CSR0L_HOST_RXPKTRDY))
        {
            USB->EPCSR[0].HOST.CSR0H = USB_CSR0H_HOST_FLSHFIFO;
        }

        /* Clear control and status register */
        USB->EPCSR[0].HOST.CSR0L = 0x0U;

        /* Enable the control endpoint interrupt */
        USB->COMMON.INTRTXE |= 0x1U;
    }
    else
    {
        /* Non-control endpoint */

        /* Find a free pipe */
        uint8_t pipe_num = find_free_pipe(ep_dir);

        if (pipe_num != 0xFFU)
        {
            /* A free pipe has been found */
            tuh_bus_info_t bus_info;
            tuh_bus_info_get(dev_addr, &bus_info);

            /* TXTYPE and RXTYPE registers have the same bitfield structure.
               Use the same value for both. */
            uint8_t type = 0U;

            /* Speed */
            switch (bus_info.speed)
            {
                case TUSB_SPEED_LOW:
                {
                    type = USB_TXTYPE_SPEED_LOW;
                }
                break;

                case TUSB_SPEED_FULL:
                {
                    type = USB_TXTYPE_SPEED_FULL;
                }
                break;

                case TUSB_SPEED_HIGH:
                {
                    type = USB_TXTYPE_SPEED_HIGH;
                }
                break;

                default:
                {
                    retval = false;
                }
                break;
            }

            /* Protocol */
            switch (ep_desc->bmAttributes.xfer)
            {
                case TUSB_XFER_ISOCHRONOUS:
                {
                    type |= USB_TXTYPE_PROTOCOL_ISOCHRONOUS;
                }
                break;

                case TUSB_XFER_BULK:
                {
                    type |= USB_TXTYPE_PROTOCOL_BULK;
                }
                break;

                case TUSB_XFER_INTERRUPT:
                {
                    type |= USB_TXTYPE_PROTOCOL_INTERRUPT;
                }
                break;

                default:
                {
                    retval = false;
                }
                break;
            }

            /* Target Endpoint Number */
            type |= ep_num;

            /* Calculate FIFO size and find a free buffer */
            uint16_t max_packet_size = tu_edpt_packet_size(ep_desc);
            uint16_t fifo_size = 28U - TU_MIN(28U, __CLZ((uint32_t)max_packet_size));
            if ((8U << fifo_size) < max_packet_size)
            {
                fifo_size++;
            }
            fifo_size = 1U << (fifo_size + 3U);
            uint16_t fifo_address = free_blk_list_alloc(fifo_size);

            if (fifo_address == BLK_INVALID_ADDRESS)
            {
                retval = false;
            }

            if (retval != false)
            {
                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(pipe_num, ep_dir);

                pipe->dev_addr = dev_addr;
                pipe->ep_addr = ep_addr;
                pipe->max_packet_size = max_packet_size;

                pipe->fifo_addr = fifo_address;
                pipe->fifo_size = fifo_size;

                /* Store the current endpoint index */
                uint8_t old_index = USB->COMMON.INDEX & USB_INDEX_ENDPOINT;

                /* Select the endpoint registers */
                USB->COMMON.INDEX = pipe_num & USB_INDEX_ENDPOINT;

#if defined (BE_U1000) && (USBH_USE_DMA)
                /* Find free USB DMA channel */
                uint8_t dma_ch = usb_dma_channel_find();
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

                if (ep_dir == TUSB_DIR_OUT)
                {
                    /* OUT */

#if defined (BE_U1000) && (USBH_USE_DMA)
                    if (dma_ch != DMA_CH_INVALID_INDEX)
                    {
                        pipe->dma_ch = DMA_CH_IN_USE_FLAG | dma_ch;
                        usb_dma_channel_alloc(dma_ch, pipe_num);
                    }
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

                    /* Configure the target endpoint */
                    USB->TADDR[pipe_num].TXFUNCADDR = dev_addr;
                    USB->TADDR[pipe_num].TXHUBADDR = bus_info.hub_addr;
                    USB->TADDR[pipe_num].TXHUBPORT = bus_info.hub_port;

                    USB->EPCSR[pipe_num].HOST.TXTYPE = type;
                    USB->EPCSR[pipe_num].HOST.TXINTERVAL = ep_desc->bInterval;
                    USB->EPCSR[pipe_num].HOST.TXMAXP = pipe->max_packet_size;

                    /* Set FIFO address */
                    USB->CTRL.TXFIFOADD = (pipe->fifo_addr >> 3) & USB_TXFIFOADD_AD;

                    /* Set FIFO size */
                    USB->CTRL.TXFIFOSZ = (__CTZ(pipe->fifo_size) - 3U) & USB_TXFIFOSZ_SZ;

#if defined(USBH_DPKTBUF)
                    /* Enable double packet buffering */
                    USB->CTRL.TXFIFOSZ |= USB_TXFIFOSZ_DPB;
#endif /* defined(USBH_DPKTBUF) */

                    /* Flush FIFO */
                    if (USB->EPCSR[pipe_num].HOST.TXCSRL & USB_TXCSRL_HOST_TXPKTRDY)
                    {
                        /* NOTE: It may be necessary to set this bit twice in succession
                                 if double buffering is enabled. */
                        USB->EPCSR[pipe_num].HOST.TXCSRL |= USB_TXCSRL_HOST_FLSHFIFO;
                    }

                    /* Reset the endpoint data toggle */
                    USB->EPCSR[pipe_num].HOST.TXCSRL |= USB_TXCSRL_HOST_CLRDT;

                    /* Enable the endpoint interrupt */
                    USB->COMMON.INTRTXE |= (1U << pipe_num);
                }
                else
                {
                    /* IN */

#if defined (BE_U1000) && (USBH_USE_DMA)
                    if (dma_ch != DMA_CH_INVALID_INDEX)
                    {
                        pipe->dma_ch = dma_ch;
                        usb_dma_channel_alloc(dma_ch, TUSB_DIR_IN_MASK | pipe_num);
                    }
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

                    /* Configure the target endpoint */
                    USB->TADDR[pipe_num].RXFUNCADDR = dev_addr;
                    USB->TADDR[pipe_num].RXHUBADDR = bus_info.hub_addr;
                    USB->TADDR[pipe_num].RXHUBPORT = bus_info.hub_port;

                    USB->EPCSR[pipe_num].HOST.RXTYPE = type;
                    USB->EPCSR[pipe_num].HOST.RXINTERVAL = ep_desc->bInterval;
                    USB->EPCSR[pipe_num].HOST.RXMAXP = pipe->max_packet_size;

                    /* Set FIFO address */
                    USB->CTRL.RXFIFOADD = (pipe->fifo_addr >> 3) & USB_RXFIFOADD_AD;

                    /* Set FIFO size */
                    USB->CTRL.RXFIFOSZ = (__CTZ(pipe->fifo_size) - 3U) & USB_RXFIFOSZ_SZ;

#if defined(USBH_DPKTBUF)
                    /* Enable double packet buffering */
                    USB->CTRL.RXFIFOSZ |= USB_RXFIFOSZ_DPB;
#endif /* defined(USBH_DPKTBUF) */

                    /* Flush FIFO */
                    if (USB->EPCSR[pipe_num].HOST.RXCSRL & USB_RXCSRL_HOST_RXPKTRDY)
                    {
                        /* NOTE: It may be necessary to set this bit twice in succession
                                 if double buffering is enabled. */
                        USB->EPCSR[pipe_num].HOST.RXCSRL |= USB_RXCSRL_HOST_FLSHFIFO;
                    }

                    /* Reset the endpoint data toggle */
                    USB->EPCSR[pipe_num].HOST.RXCSRL |= USB_RXCSRL_HOST_CLRDT;

                    /* Enable the endpoint interrupt */
                    USB->COMMON.INTRRXE |= (1U << pipe_num);
                }

                /* Restore endpoint index */
                USB->COMMON.INDEX = old_index;
            }
        }
        else
        {
            /* A free pipe has not been found */
            retval = false;
        }
    }

    return retval;
}

/**
 * @brief Closes a non-control endpoint.
 * @param rhport The roothub port number.
 * @param daddr The target device address.
 * @param ep_addr The target device endpoint address.
 * @return True, if the endpoint has been closed successfully, otherwise false.
 */
bool hcd_edpt_close(uint8_t rhport, uint8_t daddr, uint8_t ep_addr)
{
    (void)rhport;

    bool retval = true;

    uint8_t pipe_num = find_pipe(daddr, ep_addr);

    if ((pipe_num != 0U) &&
        (pipe_num != PIPE_INVALID_INDEX))
    {
        uint8_t ep_dir = tu_edpt_dir(ep_addr);
        close_pipe(pipe_num, ep_dir);
    }
    else
    {
        retval = false;
    }

    return retval;
}

/**
 * @brief Data transfer callback function.
 * @note This function is called when hcd_event_xfer_complete() is invoked.
 * @param rhport The roothub port number.
 * @param dev_addr The target device address.
 * @param ep_addr The endpoint address.
 * @param buffer The data buffer. It might be split in several packets.
 * @param total_bytes The data length (full transfer).
 * @return True, if completed successfully, otherwise false.
 */
bool hcd_edpt_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr, uint8_t * buffer, uint16_t buflen)
{
    (void)rhport;

    bool retval = true;

    uint8_t pipe_num = find_pipe(dev_addr, ep_addr);

    if (pipe_num != PIPE_INVALID_INDEX)
    {
        uint8_t ep_dir = tu_edpt_dir(ep_addr);
        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(pipe_num, ep_dir);

        pipe->xfer.xfer_buf = buffer;
        pipe->xfer.xfer_len = buflen;

        if (pipe_num == 0U)
        {
            /* Control pipe */
            switch (hcd.pipe0_state)
            {
                case PIPE0_STATE_DATA_IN:
                {
                    /* Data IN state */
                    pipe->xfer.xferred = 0U;
                    pipe->xfer.last_packet_size = 0U;

                    USB->EPCSR[0].HOST.CSR0L |= USB_CSR0L_HOST_REQPKT;
                }
                break;

                case PIPE0_STATE_DATA_OUT:
                {
                    /* Data OUT state */
                    pipe->xfer.xferred = 0U;
                    pipe->xfer.last_packet_size = tu_min16(pipe->max_packet_size, pipe->xfer.xfer_len - pipe->xfer.xferred);

#if defined (BE_U1000) && (USBH_USE_DMA)
                    if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
                    {
                        uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

                        USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + (uint32_t)RAM_ACCESS_OFFSET;
                        USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
                        USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | (0UL << USB_DMA_CNTL_DMAEP_Pos) |
                                                       USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                       USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
                    }
                    else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
                    {
                        tx_fifo_write(pipe_num, pipe->xfer.xfer_buf, pipe->xfer.last_packet_size);

                        /* Send OUT token followed by the first data packet */
                        USB->EPCSR[0].HOST.CSR0L |= USB_CSR0L_HOST_TXPKTRDY;
                    }
                }
                break;

                case PIPE0_STATE_STATUS_IN:
                {
                    /* Status IN state */

                    /* Send IN token */
                    USB->EPCSR[0].HOST.CSR0L |= (USB_CSR0L_HOST_STATPKT | USB_CSR0L_HOST_REQPKT);
                }
                break;

                case PIPE0_STATE_STATUS_OUT:
                {
                    /* Status OUT state */

                    /* Send OUT token */
                    USB->EPCSR[0].HOST.CSR0L |= (USB_CSR0L_HOST_STATPKT | USB_CSR0L_HOST_TXPKTRDY);
                }
                break;

                default:
                {
                    /* No action */
                }
                break;
            }
        }
        else
        {
            /* Non-control pipe */
            if (ep_dir == TUSB_DIR_OUT)
            {
                /* OUT */
                pipe->xfer.last_packet_size = tu_min16(pipe->max_packet_size, pipe->xfer.xfer_len);
                pipe->xfer.xferred = 0U;

#if defined (USBH_SPEED_TEST) && defined (USBH_SPEED_TEST_TX)
                if ((!hcd_speed_test_is_running()) &&
                    (pipe->xfer.last_packet_size != 0U))
                {
                    hcd_speed_test_start();
                }
#endif /* defined (USBH_SPEED_TEST) && defined (USBH_SPEED_TEST_TX) */

#if defined (BE_U1000) && (USBH_USE_DMA)
                if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
                {
                    uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

                    USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + (uint32_t)RAM_ACCESS_OFFSET;
                    USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
                    USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | ((uint32_t)pipe_num << USB_DMA_CNTL_DMAEP_Pos) |
                                                   USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                                   USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
                }
                else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
                {
                    tx_fifo_write(pipe_num, pipe->xfer.xfer_buf, pipe->xfer.last_packet_size);

                    /* Send OUT token followed by the first data packet */
                    USB->EPCSR[pipe_num].HOST.TXCSRL |= USB_TXCSRL_HOST_TXPKTRDY;
                }
            }
            else
            {
                /* IN */
                pipe->xfer.xferred = 0U;
                pipe->xfer.last_packet_size = 0U;

                /* Send IN token */
                USB->EPCSR[pipe_num].HOST.RXCSRL |= USB_RXCSRL_HOST_REQPKT;
            }
        }
    }
    else
    {
        retval = false;
    }

    return retval;
}

/**
 * @brief Aborts a queued transfer.
 * @note It can only abort transfer that has not been started.
 * @param rhport The roothub port number.
 * @param dev_addr The target device address.
 * @param ep_addr The endpoint address.
 * @returns True, if a queued transfer is aborted; false, if there is no transfer to abort.
 */
bool hcd_edpt_abort_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr)
{
    (void)rhport;

    bool retval = true;

    uint8_t pipe_num = find_pipe(dev_addr, ep_addr);

    if (pipe_num != PIPE_INVALID_INDEX)
    {
        if (pipe_num == 0U)
        {
            /* Control endpoint */

            /* Flush FIFO */
            if (USB->EPCSR[0].HOST.CSR0L & (USB_CSR0L_HOST_TXPKTRDY | USB_CSR0L_HOST_RXPKTRDY))
            {
                USB->EPCSR[0].HOST.CSR0H = USB_CSR0H_HOST_FLSHFIFO;
            }

            /* Clear control and status register */
            USB->EPCSR[0].HOST.CSR0L = 0x0U;

            hcd.pipe0_state = PIPE0_STATE_IDLE;
        }
        else
        {
            /* Non-control endpoint */
            uint8_t ep_dir = tu_edpt_dir(ep_addr);

            if (ep_dir == TUSB_DIR_OUT)
            {
                /* OUT */

                /* Flush FIFO */
                if (USB->EPCSR[pipe_num].HOST.TXCSRL & USB_TXCSRL_HOST_TXPKTRDY)
                {
                    USB->EPCSR[pipe_num].HOST.TXCSRL = USB_TXCSRL_HOST_FLSHFIFO;
                }

                /* Clear control and status register */
                USB->EPCSR[pipe_num].HOST.TXCSRL = 0x0U;
            }
            else
            {
                /* IN */

                /* Flush FIFO */
                if (USB->EPCSR[pipe_num].HOST.RXCSRL & USB_RXCSRL_HOST_RXPKTRDY)
                {
                    USB->EPCSR[pipe_num].HOST.RXCSRL = USB_RXCSRL_HOST_FLSHFIFO;
                }

                 /* Clear control and status register */
                USB->EPCSR[pipe_num].HOST.RXCSRL = 0x0U;
            }
        }
    }
    else
    {
        retval = false;
    }

    return retval;
}

/**
 * @brief Sends an 8-byte Setup Packet on the SETUP Phase of a Control Transaction.
 * @param rhport The roothub port number.
 * @param dev_addr The target USB device address.
 * @param setup_packet The pointer to setup packet data.
 * @returns True, if sent successfully, otherwise false.
 */
bool hcd_setup_send(uint8_t rhport, uint8_t dev_addr, uint8_t const setup_packet[8])
{
    (void)rhport;

    bool retval = true;

    /* Configure the target endpoint */
    tuh_bus_info_t bus_info;
    tuh_bus_info_get(dev_addr, &bus_info);

    switch (bus_info.speed)
    {
        case TUSB_SPEED_LOW:
        {
            USB->EPCSR[0].HOST.TYPE0 = USB_TYPE0_SPEED_LOW;
        }
        break;

        case TUSB_SPEED_FULL:
        {
            USB->EPCSR[0].HOST.TYPE0 = USB_TYPE0_SPEED_FULL;
        }
        break;

        case TUSB_SPEED_HIGH:
        {
            USB->EPCSR[0].HOST.TYPE0 = USB_TYPE0_SPEED_HIGH;
        }
        break;

        default:
        {
            retval = false;
        }
        break;
    }

    if (retval != false)
    {
        USB->TADDR[0].TXFUNCADDR = dev_addr;
        USB->TADDR[0].TXHUBADDR = bus_info.hub_addr;
        USB->TADDR[0].TXHUBPORT = bus_info.hub_port;

        /* Save request data */
        memcpy((void*)&hcd.control_request, (void*)setup_packet, 8U);

        /* Fill in the pipe control structure */

        /* Multiple connected devices share the same control pipe.
           Set the target device address in both OUT and IN control
           structures before starting a control transfer. */
        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_IN);
        pipe->dev_addr = dev_addr;

        /* Use OUT control structure for SETUP transaction */
        pipe = PIPE_CTRL_BASE(0, TUSB_DIR_OUT);
        pipe->dev_addr = dev_addr;

        pipe->xfer.xfer_buf = (uint8_t*)setup_packet;
        pipe->xfer.xfer_len = 8U;
        pipe->xfer.last_packet_size = 8U;
        pipe->xfer.xferred = 0U;

#if defined (BE_U1000) && (USBH_USE_DMA)
        if (pipe->dma_ch & DMA_CH_IN_USE_FLAG)
        {
            uint8_t dma_ch = pipe->dma_ch & DMA_CH_NUM_MASK;

            USB->DMA.CH[dma_ch].DMA_ADDR = (uint32_t)pipe->xfer.xfer_buf + (uint32_t)RAM_ACCESS_OFFSET;
            USB->DMA.CH[dma_ch].DMA_COUNT = pipe->xfer.last_packet_size;
            USB->DMA.CH[dma_ch].DMA_CNTL = USB_DMA_CNTL_DMABRSTM_MODE3 | (0UL << USB_DMA_CNTL_DMAEP_Pos) |
                                           USB_DMA_CNTL_DMAIE_ENABLE | USB_DMA_CNTL_DMAMODE_MODE0 |
                                           USB_DMA_CNTL_DMADIR_READ | USB_DMA_CNTL_DMAEN_ENABLE;
        }
        else
#endif /* defined (BE_U1000) && (USBH_USE_DMA) */
        {
            /* Fill in the FIFO */
            tx_fifo_write(0U, pipe->xfer.xfer_buf, pipe->xfer.last_packet_size);

            hcd.pipe0_state = PIPE0_STATE_SETUP;

            /* Send a SETUP token followed by the 8-byte command */
            USB->EPCSR[0].HOST.CSR0L |= (USB_CSR0L_HOST_SETUPPKT | USB_CSR0L_HOST_TXPKTRDY);
        }
    }

    return retval;
}

/**
 * @brief Clears the endpoint stall and resets data toggle to 0.
 * @param rhport The roothub port number.
 * @param dev_addr The target device address.
 * @param ep_addr The endpoint address.
 */
bool hcd_edpt_clear_stall(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr)
{
    (void)rhport;

    bool retval = true;

    uint8_t pipe_num = find_pipe(dev_addr, ep_addr);

    if (pipe_num != PIPE_INVALID_INDEX)
    {
        if (pipe_num == 0U)
        {
            /* Control endpoint */
            USB->EPCSR[0].HOST.CSR0L &= ~USB_CSR0L_HOST_RXSTALL;
        }
        else
        {
            /* Non-control endpoint */
            uint8_t ep_dir = tu_edpt_dir(ep_addr);

            if (ep_dir == TUSB_DIR_OUT)
            {
                /* OUT */
                USB->EPCSR[pipe_num].HOST.TXCSRL &= ~USB_TXCSRL_HOST_RXSTALL;
                USB->EPCSR[pipe_num].HOST.TXCSRL |= USB_TXCSRL_HOST_CLRDT;
            }
            else
            {
                /* IN */
                USB->EPCSR[pipe_num].HOST.RXCSRL &= ~USB_RXCSRL_HOST_RXSTALL;
                USB->EPCSR[pipe_num].HOST.RXCSRL |= USB_RXCSRL_HOST_CLRDT;
            }
        }
    }
    else
    {
        retval = false;
    }

    return retval;
}

/**
 * @brief Writes the endpoint FIFO.
 * @param epnum The endpoint number.
 * @param buffer The pointer to data buffer.
 * @param count The number of bytes to write.
 */
void tx_fifo_write(uint8_t epnum, uint8_t const * buffer, size_t count)
{
    if (buffer != NULL)
    {
        /* The endpoint buffer alignment is defined by CFG_TUSB_MEM_ALIGN macro 
        in tusb_option.h. Make sure the alignment size is 4 bytes for optimal
        performance. */
        uint32_t *buffer_word = (uint32_t*)buffer;

        volatile uint8_t * fifo_reg = (volatile uint8_t *)(&USB->FIFO[epnum]);
        volatile uint32_t * fifo_reg_word = (volatile uint32_t *)(&USB->FIFO[epnum]);

        size_t i = 0U;

        /* Write FIFO by word whenever possible */
        size_t count_word = count / sizeof(uint32_t);
        while (i < count_word)
        {
            *fifo_reg_word = buffer_word[i];
            i++;
        }

        i *= sizeof(uint32_t);

        /* Write the rest of data by byte */
        while (i < count)
        {
            *fifo_reg = buffer[i];
            i++;
        }
    }
}

/**
 * @brief Reads the endpoint FIFO.
 * @param epnum The endpoint number.
 * @param buffer The pointer to data buffer.
 * @return The number of bytes have been read.
 */
uint16_t rx_fifo_read(uint8_t epnum, uint8_t * buffer)
{
    uint16_t count = USB->EPCSR[epnum].PERIPH.RXCOUNT;

    if (buffer != NULL)
    {
        /* The endpoint buffer alignment is defined by CFG_TUSB_MEM_ALIGN macro 
        in tusb_option.h. Make sure the alignment size is 4 bytes for optimal
        performance. */
        uint32_t *buffer_word = (uint32_t*)buffer;

        volatile uint8_t * fifo_reg = (volatile uint8_t *)(&USB->FIFO[epnum]);
        volatile uint32_t * fifo_reg_word = (volatile uint32_t *)(&USB->FIFO[epnum]);

        uint16_t i = 0U;

        /* Read FIFO by word whenever possible */
        uint16_t count_word = count / sizeof(uint32_t);

        while (i < count_word)
        {
            buffer_word[i] = *fifo_reg_word;
            i++;
        }

        i *= sizeof(uint32_t);

        /* Read the rest of data by byte */
        while (i < count)
        {
            buffer[i] = *fifo_reg;
            i++;
        }
    }
    else
    {
        count = 0U;
    }

    return count;
}

void __attribute__ ((interrupt)) USB_IRQHandler(void)
{
    hcd_int_handler(BOARD_TUH_RHPORT, true);
}

#if defined (BE_U1000) && (USBH_USE_DMA)

void __attribute__ ((interrupt)) USB_DMA_IRQHandler(void)
{
    uint8_t dmaInt = USB->DMA.DMA_INTR;

    uint8_t i = 0U;
    while (dmaInt > 0U)
    {
        if (dmaInt & 0x1U)
        {
            if (i == 0U)
            {
                /* Control endpoint */
                switch (hcd.pipe0_state)
                {
                    case PIPE0_STATE_IDLE:
                    {
                        /* IDLE state */
                        /* Setup packet has been filled in the FIFO */

                        hcd.pipe0_state = PIPE0_STATE_SETUP;

                        /* Send a SETUP token followed by the 8-byte command */
                        USB->EPCSR[0].HOST.CSR0L |= (USB_CSR0L_HOST_SETUPPKT | USB_CSR0L_HOST_TXPKTRDY);
                    }
                    break;

                    case PIPE0_STATE_DATA_IN:
                    {
                        /* Data IN state */
                        pipe_ctrl_t * pipe = PIPE_CTRL_BASE(0, TUSB_DIR_IN);

                        pipe->xfer.xferred += pipe->xfer.last_packet_size;

                        if ((pipe->xfer.xferred >= hcd.control_request.wLength) ||
                            (pipe->xfer.last_packet_size < pipe->max_packet_size))
                        {
                            /* The last data packet */
                            hcd.pipe0_state = PIPE0_STATE_STATUS_OUT;
                            USB->EPCSR[0].HOST.CSR0L &= ~USB_CSR0L_HOST_RXPKTRDY;

                            /* Notify stack about the end of the data transaction */
                            hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
                        }
                        else
                        {
                            /* More data packets to request */
                            USB->EPCSR[0].HOST.CSR0L |= USB_CSR0L_HOST_REQPKT;
                        }
                    }
                    break;

                    case PIPE0_STATE_DATA_OUT:
                    {
                        /* Data OUT state */

                        /* Send OUT token followed by the first data packet */
                        USB->EPCSR[0].HOST.CSR0L |= USB_CSR0L_HOST_TXPKTRDY;
                    }
                    break;

                    default:
                    {
                        /* No action */
                    }
                    break;
                }
            }
            else if ((dma_ctrl.ep_addr[i] & TUSB_DIR_IN_MASK) != 0U)
            {
                /* RX endpoint */
                uint8_t pipe_num = dma_ctrl.ep_addr[i] & TUSB_EPNUM_MASK;

                pipe_ctrl_t * pipe = PIPE_CTRL_BASE(pipe_num, TUSB_DIR_IN);

                /* Clear RxPktRdy flag */
                USB->EPCSR[pipe_num].HOST.RXCSRL &= ~USB_RXCSRL_HOST_RXPKTRDY;

#if defined (USBH_SPEED_TEST) && defined (USBH_SPEED_TEST_RX)
                hcd_speed_test_stats.bytes_sent += pipe->xfer.last_packet_size;

                if (hcd_speed_test_stats.bytes_sent < hcd_speed_test_stats.test_data_lim)
                {
                    /* Send IN token */
                    USB->EPCSR[pipe_num].HOST.RXCSRL |= USB_RXCSRL_HOST_REQPKT;
                }
                else
                {
                    /* All test packets were received */
                    /* End xfer correctly even though it was interfered by the test */

                    hcd_speed_test_stop();
                    hcd_speed_test_print_stats();

                    /* The last packet has been correctly ACKed */
                    pipe->xfer.xferred += pipe->xfer.last_packet_size;

                    hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
                }
#else
                pipe->xfer.xferred += pipe->xfer.last_packet_size;

                if ((pipe->xfer.xferred < pipe->xfer.xfer_len) &&
                    (pipe->xfer.last_packet_size == pipe->max_packet_size))
                {
                    /* More data packets expected */

                    /* Send IN token */
                    USB->EPCSR[pipe_num].HOST.RXCSRL |= USB_RXCSRL_HOST_REQPKT;
                }
                else
                {
                    /* Data transfer complete */

                    /* Notify USB stack about the data received */
                    hcd_event_xfer_complete(pipe->dev_addr, pipe->ep_addr, pipe->xfer.xferred, XFER_RESULT_SUCCESS, true);
                }
#endif /* defined (USBH_SPEED_TEST) && defined (USBH_SPEED_TEST_RX) */
            }
            else
            {
                /* TX endpoint */
                uint8_t pipe_num = dma_ctrl.ep_addr[i] & TUSB_EPNUM_MASK;

                /* Send OUT token followed by the first data packet */
                USB->EPCSR[pipe_num].HOST.TXCSRL |= USB_TXCSRL_HOST_TXPKTRDY;
            }
        }

        dmaInt >>= 1;
        i++;
    } 
}

#endif /* defined (BE_U1000) && (USBH_USE_DMA) */

#endif
